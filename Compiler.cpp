#include <cctype>
#include <iostream>

#include "Compiler.hpp"
#include "Error.hpp"

namespace ast {

int Compiler::get_tok_precedence() {
  if (!isascii(cur_token)) {
    return -1;
  } else if (precedence.count(cur_token) == 0) {
    return -1;
  } else {
    return precedence[cur_token];
  }
}

void Compiler::get_tok() { cur_token = lexer->get_tok(); }

std::unique_ptr<ExprAST> Compiler::parse_number_expr() {
  auto result = std::make_unique<NumberExprAST>(lexer->get_num_val());
  get_tok();
  return std::move(result);
}

std::unique_ptr<ExprAST> Compiler::parse_paren_expr() {
  get_tok(); // eat '('
  auto expr = parse_expr();
  if (!expr)
    return nullptr;

  if (cur_token != ')')
    return log_error("expected a ')'");

  get_tok(); // eat ')'
  return expr;
}

std::unique_ptr<ExprAST> Compiler::parse_ident_expr() {
  std::string ident = lexer->get_identifier();
  get_tok(); // get next token, eat identifier

  // variable reference
  if (cur_token != '(')
    return std::make_unique<VariableExprAST>(ident);

  // function call
  get_tok(); // eat '('
  std::vector<std::unique_ptr<ExprAST>> args;
  while (cur_token != ')') {
    if (auto arg = parse_expr())
      args.push_back(std::move(arg));
    else
      return nullptr;

    if (cur_token == ')') // end of the args
      break;

    if (cur_token != ',')
      return log_error("expected expression or ')'");
    get_tok(); // eat ,
  }

  get_tok(); // eat ')'
  return std::make_unique<CallExprAST>(ident, std::move(args));
}

std::unique_ptr<ExprAST> Compiler::parse_primary() {
  switch (cur_token) {
  case tok_identifier:
    return parse_ident_expr();
  case tok_number:
    return parse_number_expr();
  case '(':
    return parse_paren_expr();
  default:
    return log_error("unknown token when expecting an expression");
  }
}

std::unique_ptr<ExprAST> Compiler::parse_expr() {
  // an expression is a primary expression followed by a sequence of [binop,
  // primary] pairs
  auto LHS = parse_primary();
  if (!LHS)
    return nullptr;

  return parse_binop_rhs(0, std::move(LHS));
}

std::unique_ptr<ExprAST>
Compiler::parse_binop_rhs(int expr_prec, std::unique_ptr<ExprAST> LHS) {
  while (true) {
    int tok_prec = get_tok_precedence();

    // if this binop binds as tightly as the current binop, consume it otherwise
    // we're done
    if (tok_prec < expr_prec)
      return LHS;

    char binop = cur_token;
    get_tok();

    // Parse the primary expression after the binary operator.
    auto RHS = parse_primary();
    if (!RHS)
      return nullptr;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int next_tok = get_tok_precedence();
    if (tok_prec < next_tok) {
      RHS = parse_binop_rhs(tok_prec + 1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }

    LHS =
        std::make_unique<BinaryExprAST>(binop, std::move(LHS), std::move(RHS));
  }
}

std::unique_ptr<PrototypeAST> Compiler::parse_prototype() {
  if (cur_token != tok_identifier)
    return log_errorP("expecting an identifier but got invalid token");

  std::string func_name = lexer->get_identifier();
  get_tok();

  if (cur_token != '(')
    return log_errorP("expecting a '(' but got invalid token");

  get_tok(); // eat (
  std::vector<std::string> args;
  while (cur_token == tok_identifier) {
    args.push_back(lexer->get_identifier());
    get_tok();
  }

  if (cur_token != ')')
    return log_errorP("expecting a ')' but got invalid token");

  get_tok(); // eat )
  return std::make_unique<PrototypeAST>(func_name, std::move(args));
}

std::unique_ptr<PrototypeAST> Compiler::parse_extern() {
  get_tok(); // eat extern
  return parse_prototype();
}

std::unique_ptr<FunctionAST> Compiler::parse_definition() {
  get_tok(); // eat 'def'
  auto proto = parse_prototype();
  if (!proto)
    return nullptr;

  if (auto expr = parse_expr())
    return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));

  return nullptr;
}

std::unique_ptr<FunctionAST> Compiler::parse_top_level() {
  if (auto expr = parse_expr()) {
    auto proto = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));
  }
  return nullptr;
}

void Compiler::handle_def(Codegen &codegen) {
  if (auto def_ast = parse_definition()) {
    if (auto def_ir = def_ast->accept(&codegen)) {
      std::cout << "parsed a function definiton\n" << std::flush;
      def_ir->print(llvm::errs());
      //std::cout << std::endl;
      codegen.add_module();
      codegen.init_module_and_pass_mngr();
    }
  } else
    // Skip token for error recovery.
    get_tok();
}

void Compiler::handle_extern(Codegen &codegen) {
  if (auto ex_ast = parse_extern()) {
    if (auto ex_ir = ex_ast->accept(&codegen)) {
      std::cout << "parsed an extern\n" << std::flush;
      ex_ir->print(llvm::errs());
      //std::cout << std::endl;
      codegen.store_proto(ex_ast->get_name(), std::move(ex_ast));
    }
  } else
    // Skip token for error recovery.
    get_tok();
}

void Compiler::handle_top_level(Codegen &codegen) {
  if (auto fn_ast = parse_top_level()) {
    if (auto fn_ir = fn_ast->accept(&codegen)) {
      //std::cout << "parsed a top-level expression\n" << std::flush;
      //fn_ir->print(llvm::errs());
      //std::cout << std::endl;
      
      // only evaluate on top-level expressions
      codegen.eval();
    }
  } else
    // Skip token for error recovery.
    get_tok();
}

void Compiler::compile() {
  std::cout << "ready> " << std::flush;
  Codegen codegen;
  get_tok();

  while (true) {
    switch (cur_token) {
    case tok_eof:
      return;
    case ';': // ignore top-level semicolons.
      get_tok();
      break;
    case tok_def:
      handle_def(codegen);
      break;
    case tok_extern:
      handle_extern(codegen);
      break;
    default:
      handle_top_level(codegen);
      break;
    }
    std::cout << "ready> " << std::flush;
  }

  codegen.dump();
}
} // namespace ast

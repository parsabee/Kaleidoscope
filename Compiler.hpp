#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <iostream>
#include <memory>
#include <unordered_map>
#include <utility>

#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

#include "AST.hpp"
#include "Codegen.hpp"
#include "Lexer.hpp"
#include "KaleidoscopeJIT.h"

namespace ast {
class Compiler {
  std::unordered_map<char, int> precedence;
  std::unique_ptr<Lexer> lexer;
  std::istream &input;
  int cur_token;

public:
  explicit Compiler(std::istream &input)
      : lexer(std::make_unique<Lexer>(input)), cur_token(256),
        input(input), precedence{{'<', 10}, {'>', 10}, {'+', 20},
                                 {'-', 20}, {'*', 40}, {'/', 40}}
  // initializing precedence table
  {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
  }

  void compile();

private:
  // returning the precedence of current token
  int get_tok_precedence();

  // getting the next token
  void get_tok();

  // numberexpr ::= number
  std::unique_ptr<ExprAST> parse_number_expr();

  // parenexpr ::= '(' expr ')'
  std::unique_ptr<ExprAST> parse_paren_expr();

  // identifierexpr ::= identifier
  //                ::= identifier '(' expression* ')'
  std::unique_ptr<ExprAST> parse_ident_expr();

  // primary ::= identifierexpr
  //         ::= numberexpr
  //         ::= parenexpr
  std::unique_ptr<ExprAST> parse_primary();

  // expression ::= primary binoprhs
  std::unique_ptr<ExprAST> parse_expr();

  // binoprhs ::= (op primary)*
  std::unique_ptr<ExprAST> parse_binop_rhs(int expr_prec,
                                           std::unique_ptr<ExprAST> LHS);

  // prototype ::= id '(' id* ')'
  std::unique_ptr<PrototypeAST> parse_prototype();

  // extern ::= 'extern' prototype
  std::unique_ptr<PrototypeAST> parse_extern();

  // definition ::= 'def' prototype expression
  std::unique_ptr<FunctionAST> parse_definition();

  // toplevelexpr ::= expression
  std::unique_ptr<FunctionAST> parse_top_level();

  // handlers
  void handle_def(Codegen &codegen);
  void handle_extern(Codegen &codegen);
  void handle_top_level(Codegen &codegen);

  //module initializer
  void init_module_and_pass_mngr(void);
};

} // namespace ast

#endif // COMPILER_HPP

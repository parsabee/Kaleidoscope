#include <iostream>

#include "Codegen.hpp"
#include "Error.hpp"

void Codegen::init_module_and_pass_mngr(void) {
  //initializing data layout of the jit
  module = std::make_unique<llvm::Module>("Kaleidescope", context);

  module->setDataLayout(JIT->getTargetMachine().createDataLayout());
  
  FPM = std::make_unique<llvm::legacy::FunctionPassManager>(module.get());

  // Do simple "peephole" optimizations and bit-twiddling optzns.
  FPM->add(llvm::createInstructionCombiningPass());

  // Reassociate expressions.
  FPM->add(llvm::createReassociatePass());

  // Eliminate Common SubExpressions.
  FPM->add(llvm::createNewGVNPass());

  // Simplify the control flow graph (deleting unreachable blocks, etc).
  FPM->add(llvm::createCFGSimplificationPass());

  FPM->doInitialization();
}

llvm::Value *Codegen::visit(ast::NumberExprAST *node) {
  return llvm::ConstantFP::get(context, llvm::APFloat(node->get_val()));
}

// VariableExprAST
llvm::Value *Codegen::visit(ast::VariableExprAST *node) {
  llvm::Value *val = named_values[node->get_name()];
  if (!val)
    return log_errorV("Unknown variable name");
  return val;
}

// BinaryExprAST
llvm::Value *Codegen::visit(ast::BinaryExprAST *node) {
  auto L = node->get_lhs()->accept(this);
  auto R = node->get_rhs()->accept(this);
  if (!L || !R)
    return nullptr;

  switch (node->get_op()) {
  case '+':
    return builder.CreateFAdd(L, R, "addtmp");

  case '-':
    return builder.CreateFSub(L, R, "subtmp");

  case '*':
    return builder.CreateFMul(L, R, "multmp");

  case '<':
    L = builder.CreateFCmpULT(L, R, "cmptmp");
    return builder.CreateUIToFP(L, llvm::Type::getDoubleTy(context));

  default:
    return log_errorV("Invalid binary operator");
  }
}

llvm::Function *Codegen::get_func(const std::string& name) {
  
  // check to see if the function is in this module
  if (auto *f = module->getFunction(name))
    return f;

  auto fi = function_protos.find(name);
  if (fi != function_protos.end())
    return fi->second->accept(this);

  return nullptr;
}

// CallExprAST
llvm::Value *Codegen::visit(ast::CallExprAST *node) {
  llvm::Function *calleeF = get_func(node->get_callee());
  if (!calleeF)
    return log_errorV("Unknown function");

  auto &args = node->get_args();
  if (calleeF->arg_size() != args.size())
    return log_errorV("Incorrect # arguments");

  std::vector<llvm::Value *> argsV;
  for (unsigned i = 0, e = args.size(); i != e; i++) {
    argsV.push_back(args[i]->accept(this));
    if (!argsV.back())
      return nullptr;
  }

  return builder.CreateCall(calleeF, argsV, "calltmp");
}

// PrototypeAST
llvm::Function *Codegen::visit(ast::PrototypeAST *node) {
  auto &args = node->get_args();
  std::vector<llvm::Type *> doubles(args.size(),
                                    llvm::Type::getDoubleTy(context));

  llvm::FunctionType *ft =
      llvm::FunctionType::get(llvm::Type::getDoubleTy(context), doubles, false);

  llvm::Function *f = llvm::Function::Create(
      ft, llvm::Function::ExternalLinkage, node->get_name(), module.get());

  unsigned idx = 0;
  for (auto &arg : f->args())
    arg.setName(args[idx++]);

  return f;
}

// FunctionAST
llvm::Function *Codegen::visit(ast::FunctionAST *node) {
  
  auto tmp = std::move(node->get_proto());
  auto &p = *(tmp.get());
  function_protos[p.get_name()] = std::move(tmp);
  
  // checking to see if function already exist
  llvm::Function *f = get_func(p.get_name());
  if (!f)
    return nullptr;
  
  // setting entry point for function
  llvm::BasicBlock *bb = llvm::BasicBlock::Create(context, "entry", f);
  builder.SetInsertPoint(bb);

  // adding args to symbol table
  named_values.clear();
  for (auto &arg : f->args())
    named_values[arg.getName().str()] = &arg;

  if (llvm::Value *ret = node->get_body()->accept(this)) {
    builder.CreateRet(ret);
    llvm::verifyFunction(*f);
    FPM->run(*f);
    return f;
  }

  // delete function incase user mistyped
  f->eraseFromParent();
  return nullptr;
}

void Codegen::add_module() {
  JIT->addModule(std::move(module));  
}

void Codegen::store_proto(const std::string& name, std::unique_ptr<ast::PrototypeAST> ptr) {
  function_protos[name] = std::move(ptr);
}

void Codegen::eval() {
  auto h = JIT->addModule(std::move(module));
  init_module_and_pass_mngr();

  auto expr_sym = JIT->findSymbol("__anon_expr");
  assert(expr_sym && "Function not found");
  
  double (*fp)() = (double (*)())(intptr_t)cantFail(expr_sym.getAddress());
  std::cout << "Evaluated to: " << fp() << "\n";

  JIT->removeModule(h);
}

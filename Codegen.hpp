#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <iostream>
#include <map>
#include <utility>

#include "llvm/IR/LegacyPassManager.h" // llvm::legacy::FunctionPassManager()
#include "llvm/Transforms/InstCombine/InstCombine.h" // llvm::createInstructionCombiningPass()
#include "llvm/Transforms/Scalar.h" // llvm::createReassociatePass()
                                    // llvm::createNewGVNPass()
                                    // llvm::createCFGSimplificationPass()

#include "AST.hpp"
#include "KaleidoscopeJIT.h"
#include "Visitor.hpp"

class Codegen : public NodeVisitor {
  llvm::LLVMContext context;
  llvm::IRBuilder<> builder;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::orc::KaleidoscopeJIT> JIT;
  std::unique_ptr<llvm::legacy::FunctionPassManager> FPM;
  std::map<std::string, llvm::Value *> named_values;
  std::map<std::string, std::unique_ptr<ast::PrototypeAST>> function_protos;

public:
  Codegen()
      : builder(context), JIT(std::make_unique<llvm::orc::KaleidoscopeJIT>()) {
    init_module_and_pass_mngr();
  }

  // NumberExprAST
  llvm::Value *visit(ast::NumberExprAST *node) override;

  // VariableExprAST
  llvm::Value *visit(ast::VariableExprAST *node) override;

  // BinaryExprAST
  llvm::Value *visit(ast::BinaryExprAST *node) override;

  // CallExprAST
  llvm::Value *visit(ast::CallExprAST *node) override;

  // PrototypeAST
  llvm::Function *visit(ast::PrototypeAST *node) override;

  // FunctionAST
  llvm::Function *visit(ast::FunctionAST *node) override;

  // Dumping generated IR
  void dump() { module->print(llvm::errs(), nullptr); }
  
  void add_module();

  void store_proto(const std::string& name, std::unique_ptr<ast::PrototypeAST> ptr);
  
  // JIT evaluate
  void eval();

  // Initializing module
  void init_module_and_pass_mngr(void);

private:
  llvm::Function *get_func(const std::string &name);
};

#endif // CODEGEN_HPP

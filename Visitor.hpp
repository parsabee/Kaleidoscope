#ifndef VISITOR_HPP
#define VISITOR_HPP

#include <memory>

#include "AST.hpp"

// forward reference
namespace ast {
class NumberExprAST;
class VariableExprAST;
class BinaryExprAST;
class CallExprAST;
class PrototypeAST;
class FunctionAST;
} // namespace ast

// abstract visitor class
// for visitor pattern implementation
class NodeVisitor {
public:
  // NumberExprAST
  virtual llvm::Value *visit(ast::NumberExprAST *node) = 0;

  // VariableExprAST
  virtual llvm::Value *visit(ast::VariableExprAST *node) = 0;

  // BinaryExprAST
  virtual llvm::Value *visit(ast::BinaryExprAST *node) = 0;

  // CallExprAST
  virtual llvm::Value *visit(ast::CallExprAST *node) = 0;

  // PrototypeAST
  virtual llvm::Function *visit(ast::PrototypeAST *node) = 0;

  // FunctionAST
  virtual llvm::Function *visit(ast::FunctionAST *node) = 0;
};

#endif // VISITOR_HPP

#include "AST.hpp"

namespace ast {
llvm::Value *NumberExprAST::accept(NodeVisitor *visitor) const {
  return visitor->visit(this);
}

llvm::Value *VariableExprAST::accept(NodeVisitor *visitor) const {
  return visitor->visit(this);
}

llvm::Value *BinaryExprAST::accept(NodeVisitor *visitor) const {
  return visitor->visit(this);
}

llvm::Value *CallExprAST::accept(NodeVisitor *visitor) const {
  return visitor->visit(this);
}

llvm::Function *PrototypeAST::accept(NodeVisitor *visitor) const {
  return visitor->visit(this);
}

llvm::Function *FunctionAST::accept(NodeVisitor *visitor) const {
  return visitor->visit(this);
}
} // namespace ast
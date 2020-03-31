#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "Visitor.hpp"
// #include "Codegen.hpp"

class NodeVisitor; // forward reference

namespace ast {
// Base node of the AST
class ExprAST {
public:
  virtual ~ExprAST() {}
  virtual llvm::Value *accept(NodeVisitor *visitor) const = 0;
};

// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
  double val;

public:
  explicit NumberExprAST(double val) : val(val) {}

  double get_val() const { return val; }

  llvm::Value *accept(NodeVisitor *visitor) const override;
};

// Expression class for variables
class VariableExprAST : public ExprAST {
  std::string name;

public:
  explicit VariableExprAST(const std::string &name) : name(name) {}

  const std::string &get_name() const { return name; }

  llvm::Value *accept(NodeVisitor *visitor) const override;
};

// Expression class for binary operations
class BinaryExprAST : public ExprAST {
  char op;
  std::unique_ptr<ExprAST> lhs, rhs;

public:
  BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs,
                std::unique_ptr<ExprAST> rhs)
      : lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {}

  char get_op() const { return op; }

  const ExprAST *get_lhs() const { return lhs.get(); }

  const ExprAST *get_rhs() const { return rhs.get(); }

  llvm::Value *accept(NodeVisitor *visitor) const override;
};

// Expression class for calls
class CallExprAST : public ExprAST {
  std::string callee;
  std::vector<std::unique_ptr<ExprAST>> args;

public:
  CallExprAST(const std::string &callee,
              std::vector<std::unique_ptr<ExprAST>> args)
      : callee(callee), args(std::move(args)) {}

  llvm::Value *accept(NodeVisitor *visitor) const override;

  const std::string &get_callee() const { return callee; }

  const std::vector<std::unique_ptr<ExprAST>> &get_args() const { return args; }
};

// PrototypeAST - This class represents the "prototype" for a function,
// which captures its name, and its argument names (thus implicitly the number
// of arguments the function takes).
class PrototypeAST {
  std::string name;
  std::vector<std::string> args;

public:
  PrototypeAST(const std::string &name, std::vector<std::string> args)
      : name(name), args(std::move(args)) {}

  const std::string &get_name() const { return name; }

  llvm::Function *accept(NodeVisitor *visitor) const;

  const std::vector<std::string> &get_args() const { return args; }
};

// FunctionAST - This class represents a function definition itself.
class FunctionAST {
  std::unique_ptr<PrototypeAST> proto;
  std::unique_ptr<ExprAST> body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> proto,
              std::unique_ptr<ExprAST> body)
      : proto(std::move(proto)), body(std::move(body)) {}

  llvm::Function *accept(NodeVisitor *visitor) const;

  std::unique_ptr<PrototypeAST> get_proto() { return std::move(proto); }

  const ExprAST *get_body() const { return body.get(); }
};
} // namespace ast

#endif // AST_HPP

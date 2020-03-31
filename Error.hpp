#ifndef ERROR_HPP
#define ERROR_HPP

#include "AST.hpp"

// Error logging facility
std::unique_ptr<ast::ExprAST> log_error(const char *err);

std::unique_ptr<ast::PrototypeAST> log_errorP(const char *err);

llvm::Value *log_errorV(const char *err);

#endif // ERROR_HPP
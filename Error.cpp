#include "Error.hpp"

std::unique_ptr<ast::ExprAST> log_error(const char *err) {
  std::cerr << "Error: " << err << std::endl << std::flush;
  return nullptr;
}

std::unique_ptr<ast::PrototypeAST> log_errorP(const char *err) {
  log_error(err);
  return nullptr;
}

llvm::Value *log_errorV(const char *err) {
  log_error(err);
  return nullptr;
}
#include "Compiler.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
  ast::Compiler compiler(std::cin);
  compiler.compile(); //acutally interpret!
  return 0;
}

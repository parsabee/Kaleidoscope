#ifndef LEXER_HPP
#define LEXER_HPP

#include <ctype.h>
#include <iostream>
#include <string.h>

namespace ast {
enum Token {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5
};

class Lexer {
  std::string identifier_str; // Filled in if tok_identifier
  double num_val;             // Filled in if tok_number
  std::istream &input_stream;
  char last_char;

public:
  explicit Lexer(std::istream &in)
      : num_val(0), last_char(' '), input_stream(in), identifier_str() {}

  // the tokenizer
  // returns the next token found in the input stream
  int get_tok();

  // the last num_val
  double get_num_val() { return num_val; }

  // the last identifier
  std::string get_identifier() { return identifier_str; }
};
} // namespace ast

#endif // LEXER_HPP
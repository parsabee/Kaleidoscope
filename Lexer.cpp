#include "Lexer.hpp"
#include "Error.hpp"

#include <cstdlib>

namespace ast {
int Lexer::get_tok() {

  // getting rid of whitespace
  while (last_char == ' ' || last_char == '\n') {
    last_char = input_stream.get();
  }

  // if we get an alphabetic char
  if (isalpha(last_char)) {
    identifier_str = last_char;

    // get next char while it's alphanumeric
    while (isalnum(last_char = input_stream.get())) {
      identifier_str += last_char;
    }

    if (identifier_str == "def") { // if the token is "def"
      return tok_def;
    } else if (identifier_str == "extern") { // if the token is "extern"
      return tok_extern;
    } else { // otherwise it's an identifier
      return tok_identifier;
    }

    // if we get a digit or '.'
  } else if (isdigit(last_char) || last_char == '.') {
    int dot_count = 0;

    std::string num_str;
    while (isdigit(last_char) || last_char == '.') {
      if (last_char == '.') {
        dot_count++;
      }

      num_str += last_char;
      last_char = input_stream.get();
    }

    // FIXME, if dot_count >= 2

    num_val = std::strtod(num_str.c_str(), 0);
    return tok_number;

    // comment until the end of this line
  } else if (last_char == '#') {
    while (last_char != EOF || last_char != '\n' || last_char != '\r')
      last_char = input_stream.get();
    if (last_char == EOF)
      return tok_eof;
    else
      return Lexer::get_tok();

    // end of file
  } else if (last_char == EOF) {
    return tok_eof;

    // otherwise return the ascii value
  } else {
    int ret_val = last_char;
    last_char = input_stream.get();
    return ret_val;
  }
}
} // namespace ast

// //testing tokenizer
// int main(int argc, char *argv[]) {
//     ast::Lexer lexer(std::cin);
//     int token;
//     while((token = lexer.get_tok()) != ast::tok_eof) {
//         switch(token) {
//             case ast::tok_identifier: std::cout << "identifier " <<
//             lexer.get_identifier() << "\n"; break; case ast::tok_number:
//             std::cout << "number " << lexer.get_num_val() << "\n";
//         }
//     }
// }

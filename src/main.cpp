// git push -u origin main
#include "../src/include/tokenization.hpp"
#include "../src/include/parser.hpp"
#include "../src/include/generation.hpp"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// git push -u origin main

// Helper function to convert TokenType to string
[[nodiscard]] inline std::string to_string(TokenType type) {
  switch (type) {
    case TokenType::exit: return "exit";
    case TokenType::return_: return "return";
    case TokenType::print: return "print";
    case TokenType::semi: return "semi";
    case TokenType::open_paren: return "open_paren";
    case TokenType::close_paren: return "close_paren";
    case TokenType::ident: return "ident";
    case TokenType::eq: return "eq";
    case TokenType::int_lit: return "int_lit";
    case TokenType::int_type: return "int_type";
    case TokenType::bool_true_lit: return "bool_true_lit";
    case TokenType::bool_false_lit: return "bool_false_lit";
    case TokenType::bool_type: return "bool_type";
    case TokenType::dot: return "dot";
    case TokenType::float_lit: return "float_lit";
    case TokenType::float_type: return "float_type";
    case TokenType::quote_s: return "quote_s"; // Single quote
    case TokenType::char_type: return "char_type";
    case TokenType::quote_d: return "quote_d"; // Double quote
    case TokenType::string_type: return "string_type";
    case TokenType::string_lit: return "string_lit";
    case TokenType::list_type: return "list_type";
    default: return "unknown";
  }
}
// ./brouss ; ./out ; echo $?
int main(int argc, char* argv[]) {
  std::ifstream inf{ "test.bro" };

  if (!inf) {
    std::cerr << "Could not open file" << std::endl;
  }
  
  
  std::string contents;
  {
    std::stringstream contents_stream;
    contents_stream << inf.rdbuf();
    contents = contents_stream.str();
  }

  //std::cout << contents << std::endl;

  Tokenizer tokenizer(std::move(contents));
  std::vector<Token> tokens = tokenizer.tokenize();
  //std::cout << tokens.size() << std::endl;
  // for (Token token : tokens) {
  //   std::cout << to_string(token.type) << std::endl;
  // }
  Parser parser(std::move(tokens));
  std::optional<NodeProg> prog = parser.parse_prog();
  if (!prog.has_value()) {
    std::cerr << "Invalid program" << std::endl;
    exit(EXIT_FAILURE);
  }

  Generator generator(prog.value());
  {
    std::fstream file("out.asm", std::ios::out);
    file << generator.gen_prog();
  }

  system("nasm -felf64 out.asm");
  system("ld -o out out.o");
  
  return 0;
}

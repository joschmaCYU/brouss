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
  //for (Token token : tokens) {
  //   std::cout << to_string(token.type) << std::endl;
  //}
  tokens.push_back({.type = TokenType::semi});
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

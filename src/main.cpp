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

std::string token_type_to_string(TokenType type) {
    switch (type) {
        case TokenType::exit:    return "EXIT";
        case TokenType::int_lit: return "INT_LIT";
        case TokenType::semi:    return "SEMI";
    }
    return "UNKNOWN";
}

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
  for (const Token& token : tokens) {
    std::cout << "Type: " << token_type_to_string(token.type) << '\n';
  }

  Parser parser(std::move(tokens));
  std::optional<NodeExit> tree = parser.parse();
  std::cout << tree.has_value() << std::endl;

  

  if (!tree.has_value()) {
    std::cerr << "No exit statement found" << std::endl;
    exit(EXIT_FAILURE);
  }

  Generator generator(tree.value());
  {
    std::fstream file("out.asm", std::ios::out);
    file << generator.generate();
  }

  system("nasm -felf64 out.asm");
  system("ld -o out out.o");
  
  return 0;
}

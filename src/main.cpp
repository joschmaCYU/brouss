#include "../src/include/tokenization.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
std::string token_type_to_string(TokenType type) {
    switch (type) {
        case TokenType::exit:    return "EXIT";
        case TokenType::int_lit: return "INT_LIT";
        case TokenType::semi:    return "SEMI";
    }
    return "UNKNOWN";
}
std::string tokens_to_asm(const std::vector<Token>& tokens) {
  std::stringstream output;
  output << "global _start\n_start:\n";

  for (int i =0; i < tokens.size(); i++) {
    const Token& token = tokens.at(i);

    if (token.type == TokenType::exit) {
      if (i+1 < tokens.size() && tokens.at(i+1).type == TokenType::int_lit) {
        if (i+2 < tokens.size() && tokens.at(i+2).type == TokenType::semi) {
          output << "    mov rax, 60\n";
          output << "    mov rdi, " << tokens.at(i+1).value.value() << "\n";
          output << "    syscall";
        }
      }
    }
  }
  return output.str();
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

  Tokenizer tokenizer(std::move(contents));
  std::vector<Token> tokens = tokenizer.tokenize();
  {
    std::fstream file("out.asm", std::ios::out);
    file << tokens_to_asm(tokens);
  }

  for (const Token& token : tokens) {
    std::cout << "Type: " << token_type_to_string(token.type) << '\n';
  }

  
  system("nasm -felf64 out.asm");
  system("ld -o out out.o");
  
  return 0;
}

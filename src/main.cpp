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

enum class TokenType {
  _return,
  int_lit,
  semi
};

struct Token {
  TokenType type;
  std::optional<std::string> value {};
};

std::vector<Token> tokenize(const std::string & str) {
  std::vector<Token> tokens {};
  std::string buf;

  for (int i = 0; i < str.length(); i++) {
    char c = str.at(i);

    if (i < str.length() && std::isalpha(c)) { // Returns non-zero letter
      buf.push_back(c);
      i++;

      while (std::isalnum(str.at(i))) { // Returns non-zero if letter or digit
        buf.push_back(str.at(i));
        i++;
      }
      i--;
      if (buf == "return") {
        tokens.push_back({.type = TokenType::_return});
        buf.clear();
        continue;
      } else {
        std::cerr << "Not good" << std::endl;
        exit(EXIT_FAILURE);
      }
    } else if (std::isdigit(str.at(i))) { // get numbers
      buf.push_back(c);
      i++;

      while (i < str.length() &&std::isdigit(str.at(i))) {
        buf.push_back(str.at(i));
        i++;
      }
      i--;
      tokens.push_back({.type = TokenType::int_lit, .value = buf});
      buf.clear();
    } else if (c == '\n') {
      tokens.push_back({.type = TokenType::semi});
    } else if (std::isspace(c)) { // blank
      continue;
    } else { // oh no not good
      std::cerr << "Not good" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  return tokens;
}

std::string tokens_to_asm(const std::vector<Token>& tokens) {
  std::stringstream output;
  output << "global _start\n_start:\n";

  for (int i =0; i < tokens.size(); i++) {
    const Token& token = tokens.at(i);

    if (token.type == TokenType::_return) {
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
  
  std::vector<Token> tokens;
  std::string strInput{};
  while (std::getline(inf, strInput)) {
    std::cout << strInput << std::endl;
    std::vector<Token> newTokens = tokenize(strInput + '\n');
    tokens.insert(tokens.end(), newTokens.begin(), newTokens.end());
  }

  std::cout << tokens_to_asm(tokens) << "\n";

  {
    std::fstream file("out.asm", std::ios::out);
    file << tokens_to_asm(tokens);
  }
  
  system("nasm -felf64 out.asm");
  system("ld -o out out.o");
  
  return 0;
}

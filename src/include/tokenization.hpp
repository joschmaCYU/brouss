#pragma once

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <optional> 
#include <ostream>
#include <string>
#include <strstream>
#include <vector>

// (LEXER) File to identify the "token" and give it a type

enum class TokenType {
  exit,
  return_, // TODO
  print, // TODO
  semi,
  open_paren,
  close_paren,
  ident,
  eq,
  int_lit,
  int_type,
  bool_true_lit,
  bool_false_lit,
  bool_type, // TODO
  dot,
  float_lit,
  float_type, // TODO
  // mov eax, 0x40A66666    ; Load the bits into an integer register
  // movd xmm0, eax         ; Move the raw bits into xmm0
  quote_s,
  char_type, // TODO
  // movzx rax, al
  // push rax               ; The character is now at the address [rsp]
  quote_d,
  string_lit,
  string_type, // TODO
  // section .data
  //  my_string_len dq 5 
  //  my_string_data db "Hello"
  list_type, // TODO
  plus,
  minus,
  star,
  slash,
  open_curly,
  close_curly,
  if_,
  elif,
  else_,
  // TODO add reasigment x = 1, add negative numbers, add x += 1, x < 5
  // TODO fix empty line
};

std::optional<int> bin_prio(TokenType type) {
  switch (type) {
    case TokenType::plus:
    case TokenType::minus:
      return 0;
    case TokenType::star:
    case TokenType::slash:
      return 1;
    default:
      return {};
  }
}

struct Token {
  TokenType type;
  std::optional<std::string> value {};
};

class Tokenizer{
public:
  inline explicit Tokenizer(const std::string& src) 
  : m_src(std::move(src)), m_index(0){

  }

  inline std::vector<Token> tokenize() {
    std::vector<Token> tokens {};
    std::string buf;

    while (peek().has_value()) {
      if (isalpha(peek().value())) { //Returns non-zero if the parameter is a letter
        buf.push_back(consume());

        while(peek().has_value() && std::isalnum(peek().value())) {
          buf.push_back(consume());
        }

        if (buf == "exit") {
          tokens.push_back({.type = TokenType::exit});
          buf.clear();
        } else if (buf == "return") {
          tokens.push_back({ .type = TokenType::return_});
          buf.clear();
        } else if (buf == "int") {
          tokens.push_back({ .type = TokenType::int_type});
          buf.clear();
        } else if (buf == "float") {
          tokens.push_back({ .type = TokenType::float_type});
          buf.clear();
        } else if (buf == "bool") {
          tokens.push_back({ .type = TokenType::bool_type});
          buf.clear();
        } else if (buf == "true") {
          tokens.push_back({ .type = TokenType::bool_true_lit});
          buf.clear();
        } else if (buf == "false") {
          tokens.push_back({ .type = TokenType::bool_false_lit});
          buf.clear();
        } else if (buf == "char") {
          tokens.push_back({ .type = TokenType::char_type});
          buf.clear();
        } else if (buf == "string") {
          tokens.push_back({ .type = TokenType::string_type});
          buf.clear();
        } else if (buf == "print") {
          tokens.push_back({ .type = TokenType::print});
          buf.clear();
        } else if (buf == "if") {
          tokens.push_back({ .type = TokenType::if_});
          buf.clear();
        } else if (buf == "elif") {
          tokens.push_back({ .type = TokenType::elif});
          buf.clear();
        } else if (buf == "else") {
          tokens.push_back({ .type = TokenType::else_});
          buf.clear();
        } else {
          tokens.push_back({.type = TokenType::ident, .value = buf});
          buf.clear();
        }
          
      } else if (std::isdigit(peek().value())) { // Returns non-zero if the parameter is a digit
        // TODO need to check if float
        buf.push_back(consume());

        while (peek().has_value() && std::isdigit(peek().value())) {
          buf.push_back(consume());
        }
        
        if (peek().has_value() && peek().value() == '.') { // TODO fix edge case: 12.56.48.15.48
          int first = std::stoi(buf);
          buf.clear();
          consume();
          while (peek().has_value() && std::isdigit(peek().value())) {
            buf.push_back(consume());
          }
          int second = std::stoi(buf);
          std::string float_str = std::to_string(first) + "." + std::to_string(second);

          tokens.push_back({.type = TokenType::float_lit, .value = float_str});
        } else {
          tokens.push_back({.type = TokenType::int_lit, .value = buf});
        }
        buf.clear();
      } else if (peek().value() == '(') {
        consume();
        tokens.push_back({ .type = TokenType::open_paren });
      } else if (peek().value() == ')') {
        consume();
        tokens.push_back({ .type = TokenType::close_paren });
      } else if (peek().value() == '=') {
        consume();
        tokens.push_back({ .type = TokenType::eq });
      } else if (peek().value() == '.') {
        consume();
        tokens.push_back({ .type = TokenType::dot });
      } else if (peek().value() == '+') {
        consume();
        tokens.push_back({.type = TokenType::plus });
      } else if (peek().value() == '*') {
        consume();
        tokens.push_back({.type = TokenType::star });
      } else if (peek().value() == '-') {
        consume();
        tokens.push_back({.type = TokenType::minus });
      } else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/') {
        consume();
        consume();

        while (peek().has_value() && peek().value() != '\n') {
          consume();
        }
      } else if (peek().value() == '/') {
        consume();
        tokens.push_back({.type = TokenType::slash });
      } else if (peek().value() == '{') {
        consume();
        tokens.push_back({.type = TokenType::open_curly });
      } else if (peek().value() == '}') {
        consume();
        tokens.push_back({.type = TokenType::close_curly });
      } else if (peek().value() == '\n' || peek().value() == ';') {
        consume();
        tokens.push_back({.type = TokenType::semi });
      } else if (peek().value() == '\'') {
        consume();
        tokens.push_back({.type = TokenType::quote_s });
      } else if (peek().value() == '\"') {
        consume();
        tokens.push_back({.type = TokenType::quote_d });
        while (peek().has_value() && peek().value() != '\"') {
          if (peek().value() == '\n') {
            std::cerr << "Could not found closing \"";
            exit(EXIT_FAILURE);
          }

          buf.push_back(consume());
        }
        tokens.push_back({.type = TokenType::string_lit, .value = buf});
        buf.clear();

        consume(); // last "
        tokens.push_back({.type = TokenType::quote_d });
      } else if (std::isspace(peek().value())) {
        consume();
      } else {
        std::cerr << "Token not recognized " << peek().value() << std::endl;
        exit(EXIT_FAILURE);
      }
    }

    m_index = 0;
    return tokens;
  }

private:
  [[nodiscard]] inline std::optional<char> peek(int offset = 0) const {
    if (m_index + offset >= m_src.size()) {
      return {};
    } else {
      return m_src.at(m_index + offset);
    }
  }

  inline char consume() {
    return m_src.at(m_index++);
  }

  const std::string m_src;
  int m_index;
};

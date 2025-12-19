#pragma once

#include "arena.hpp"
#include "tokenization.hpp"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <ostream>
#include <utility>
#include <vector>
#include <variant>

struct NodeTermIntLit {
  Token int_lit;
};

struct NodeTermIdent {
  Token ident;
};

struct NodeExpr;

struct NodeBinExprAdd {
  NodeExpr* lhs;
  NodeExpr* rhs;
};

struct NodeBinExprMulti {
  NodeExpr* lhs;
  NodeExpr* rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprMulti*> var;
};

struct NodeTerm {
  std::variant<NodeTermIntLit*, NodeTermIdent*> var;
};

struct NodeExpr {
  std::variant<NodeTerm*, NodeBinExpr*> var;
};


struct NodeStatementExit {
  NodeExpr* expr;
};

struct NodeStatementInt {
  Token* ident;
  NodeExpr* expr;
};

struct NodeStatement {
  std::variant<NodeStatementExit*, NodeStatementInt*> var;
};

struct NodeProg {
  std::vector<NodeStatement*> stmts;
};

struct NodeExit {
  NodeExpr* expr;
};

class Parser {
public:
  inline explicit Parser(std::vector<Token> tokens)
  : m_tokens(std::move(tokens)), m_index(0), m_allocator(1024 * 1024 * 4) { // 4Mb

  }

  std::optional<NodeTerm*> parse_term() {
    if (peek().has_value() && peek().value().type == TokenType::int_lit) {
      auto term_int_lit =  m_allocator.alloc<NodeTermIntLit>();
      term_int_lit->int_lit = consume();
      auto term = m_allocator.alloc<NodeTerm>();
      term->var = term_int_lit;
      return term;
    } else if (peek().has_value() && peek().value().type == TokenType::ident) {
      auto expr_ident = m_allocator.alloc<NodeTermIdent>();
      expr_ident->ident = consume();
      auto term = m_allocator.alloc<NodeTerm>();
      term->var = expr_ident;
      return term;
    } else {
      return {};
    }
  }

  std::optional<NodeExpr*> parse_expr() {
    if (auto term = parse_term()) {
      if (peek().has_value() && peek().value().type == TokenType::plus) {
        auto bin_expr = m_allocator.alloc<NodeBinExpr>();
        auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
        auto lhs_expr = m_allocator.alloc<NodeExpr>();
        lhs_expr->var = term.value();
        bin_expr_add->lhs = lhs_expr;

        consume(); //+

        if (auto rhs = parse_expr()) {
          bin_expr_add->rhs = rhs.value();

          bin_expr->var = bin_expr_add;

          auto expr = m_allocator.alloc<NodeExpr>();
          expr->var = bin_expr;
          return expr;
        } else {
          std::cerr << "Expected expression" << std::endl;
          exit(EXIT_FAILURE);
        }
      } else {
        auto expr = m_allocator.alloc<NodeExpr>();
        expr->var = term.value();
        return expr;
      } 
    } else {
      return {};
    }
  }

  std::optional<NodeExit> parse() {
    std::optional<NodeExit> exit_node;

    while (peek().has_value()) {
      if (peek().value().type == TokenType::exit && peek(1).has_value() 
        && peek(1).value().type == TokenType::open_paren) {
        consume();
        consume();

        if (auto node_expr = parse_expr()) {
          exit_node = NodeExit{.expr = node_expr.value()};
        } else {
          std::cerr << "Invalid expression" << std::endl;
          exit(EXIT_FAILURE);
        }

        if (peek().has_value() && peek().value().type == TokenType::close_paren) {
          consume();        
        } else {
          std::cerr << "Expected `)`" << std::endl;
          exit(EXIT_FAILURE);

        }
        if (peek().has_value() && peek().value().type == TokenType::semi) {
          consume();
        } else {
          std::cerr << "Expected a end line or ;" << std::endl;
          exit(EXIT_FAILURE);
        }
      }
    }
    m_index = 0;
    return exit_node;
  }

  std::optional<NodeStatement*> parse_statement() {
    if (!peek().has_value()) {
      return {};
    }
    if (peek().value().type == TokenType::exit && peek(1).has_value() 
      && peek(1).value().type == TokenType::open_paren) { 
      // exit(..
      consume();
      consume();
      auto stmt_exit = m_allocator.alloc<NodeStatementExit>();

      if (auto node_expr = parse_expr()) {
        stmt_exit->expr = node_expr.value();
      } else {
        std::cerr << "Invalid expression" << std::endl;
        exit(EXIT_FAILURE);
      }

      if (peek().has_value() && peek().value().type == TokenType::close_paren) {
        consume();        
      } else {
        std::cerr << "Expected `)`" << std::endl;
        exit(EXIT_FAILURE);
      }

      // This forces the user to go to the next line
      if (peek().has_value() && peek().value().type == TokenType::semi) {
        consume();
      } else {
        std::cerr << "Expected a end line or ;" << std::endl;
        exit(EXIT_FAILURE);
      }

      auto stmt = m_allocator.alloc<NodeStatement>();
      stmt->var = stmt_exit;
      return stmt;
    } else if (peek().has_value() && peek().value().type == TokenType::int_type
      && peek(1).has_value() && peek(1).value().type == TokenType::ident
        && peek(2).has_value() && peek(2).value().type == TokenType::eq) {
      // int x = 5
      consume(); //int 
      auto stmt_int = m_allocator.alloc<NodeStatementInt>();
      auto ident_token = m_allocator.alloc<Token>();
      *ident_token = consume(); // consumes identifier "x"
      stmt_int->ident = ident_token;
      consume();
      if (auto expr = parse_expr()) { // 5
        stmt_int->expr = expr.value(); 
      } else {
        std::cerr << "Invalid expression" << std::endl;
        exit(EXIT_FAILURE);
      }

      if (peek().has_value() && peek().value().type == TokenType::semi) {
        consume();
      } else {
        std::cerr << "Expected a end line or ;" << std::endl;
        exit(EXIT_FAILURE);
      }

      auto stmt = m_allocator.alloc<NodeStatement>();
      stmt->var = stmt_int;
      return stmt;
    } else {
      return {};
    }
  }

  std::optional<NodeProg> parse_prog() {
    NodeProg prog;
    while (peek().has_value()) {
      if (auto stmt = parse_statement()) {
        prog.stmts.push_back(stmt.value());
      } else {
        std::cerr << "Invalid statement at token: " << std::to_string(static_cast<int>(peek().value().type)) << std::endl;
        exit(EXIT_FAILURE);
      }
    }
    return prog;
  }
  
private:
  const std::vector<Token> m_tokens;
  size_t m_index;
  [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const {
    if (m_index + offset >= m_tokens.size()) {
      return {};
    } else {
      return m_tokens.at(m_index + offset);
    }
  }

  inline Token consume() {
    return m_tokens.at(m_index++);
  }
  ArenaAllocator m_allocator;
};

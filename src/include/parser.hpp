#pragma once

#include "arena.hpp"
#include "tokenization.hpp"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
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

struct NodeBinExprMinus {
 NodeExpr* lhs;
 NodeExpr* rhs;
};

struct NodeBinExprDiv {
 NodeExpr* lhs;
 NodeExpr* rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprMinus*, NodeBinExprMulti*, NodeBinExprDiv*> var;
};

struct NodeTermParen {
  NodeExpr* expr;
};

struct NodeTerm {
  std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr {
  std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStatementExit {
  NodeExpr* expr;
};

struct NodeStatementInt {
  Token ident;
  NodeExpr* expr;
};

struct NodeStatement;

struct NodeScope {
  std::vector<NodeStatement*> stmts;
};

struct NodeIfPred;

struct NodePredElif {
  NodeExpr* expr;
  NodeScope* scope;
  std::optional<NodeIfPred*> pred;
};

struct NodePredElse {
  NodeScope* scope;
};

struct NodeIfPred {
  std::variant<NodePredElif*, NodePredElse*> var;
};

struct NodeStmtIf {
  NodeExpr* expr;
  NodeScope* scope;
  std::optional<NodeIfPred*> pred;
};
// I should use using instead of struct
struct NodeStatement {
  std::variant<NodeStatementExit*, NodeStatementInt*, NodeScope*, NodeStmtIf*> var;
};

struct NodeProg {
  std::vector<NodeStatement*> stmts;
};

struct NodeExit {
  NodeExpr* expr;
};


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
    case TokenType::close_curly: return "close_curly";
    case TokenType::open_curly: return "open_curly";
    case TokenType::if_: return "if";
    default: return "unknown";
  }
}

class Parser {
public:
  inline explicit Parser(std::vector<Token> tokens)
  : m_tokens(std::move(tokens)), m_index(0), m_allocator(1024 * 1024 * 4) { // 4Mb

  }

  std::optional<NodeTerm*> parse_term() {
    if (auto int_lit = try_consume(TokenType::int_lit)) {
      auto term_int_lit =  m_allocator.alloc<NodeTermIntLit>();
      term_int_lit->int_lit = int_lit.value();
      auto term = m_allocator.alloc<NodeTerm>();
      term->var = term_int_lit;
      return term;
    } else if (auto ident = try_consume(TokenType::ident)) {
      auto expr_ident = m_allocator.alloc<NodeTermIdent>();
      expr_ident->ident = ident.value();
      auto term = m_allocator.alloc<NodeTerm>();
      term->var = expr_ident;
      return term;
    } else if (auto open_paren = try_consume(TokenType::open_paren)) {
      auto expr = parse_expr();
      if (!expr.has_value()) {
        std::cerr << "Expected expression" << std::endl;
        exit(EXIT_FAILURE);
      }
      try_consume(TokenType::close_paren, "Expected ')'");
      auto term_paren = m_allocator.alloc<NodeTermParen>();
      term_paren->expr = expr.value();
      auto term = m_allocator.alloc<NodeTerm>();
      term->var = term_paren;
      return term;
    }else {
      return {};
    }
  }

  std::optional<NodeExpr*> parse_expr(int min_prio = 0) {
    std::optional<NodeTerm*> term_lhs = parse_term();
    if (!term_lhs.has_value()) {
      return {};
    }

    auto expr_lhs = m_allocator.alloc<NodeExpr>();
    expr_lhs->var = term_lhs.value();

    while (true) {
      std::optional<Token> curr_token = peek();
      std::optional<int> prio; 
      if (curr_token.has_value()) {
        prio = bin_prio(curr_token->type);
        if (!prio.has_value() || prio < min_prio) {
          break;
        }
      } else {
        break;
      }

      Token op = consume();
      int next_min_prio = prio.value() + 1;
      std::optional<NodeExpr*> expr_rhs = parse_expr(next_min_prio);
      if (!expr_rhs.has_value()) {
        std::cerr << "Unable to parse expression" << std::endl;
      }

      auto expr = m_allocator.alloc<NodeBinExpr>();
      auto expr_lhs2 = m_allocator.alloc<NodeExpr>();
      if (op.type == TokenType::plus) {
        auto add = m_allocator.alloc<NodeBinExprAdd>();
        expr_lhs2->var = expr_lhs->var;
        add->lhs = expr_lhs2;
        add->rhs = expr_rhs.value();
        expr->var = add;
      } else if (op.type == TokenType::star) {
        auto multi = m_allocator.alloc<NodeBinExprMulti>();
        expr_lhs2->var = expr_lhs->var;
        multi->lhs = expr_lhs2;
        multi->rhs = expr_rhs.value();
        expr->var = multi;
      } else if (op.type == TokenType::minus) {
        auto minus = m_allocator.alloc<NodeBinExprMinus>();
        expr_lhs2->var = expr_lhs->var;
        minus->lhs = expr_lhs2;
        minus->rhs = expr_rhs.value();
        expr->var = minus;
      } else if (op.type == TokenType::slash) {
        auto div = m_allocator.alloc<NodeBinExprDiv>();
        expr_lhs2->var = expr_lhs->var;
        div->lhs = expr_lhs2;
        div->rhs = expr_rhs.value();
        expr->var = div;
      } else {
        assert(false);
      }

      expr_lhs->var = expr;
    }
    return expr_lhs;
  }

  std::optional<NodeIfPred*> parse_if_pred() {
    if (try_consume(TokenType::elif)) {
      try_consume(TokenType::open_paren, "Expected '('");
      auto elif = m_allocator.alloc<NodePredElif>();
      if (auto expr = parse_expr()) {
        elif->expr = expr.value();
      } else {
        std::cerr << "Expected expression" << std::endl;
        exit(EXIT_FAILURE);
      }
      try_consume(TokenType::close_paren, "Expected ')'");

      if (auto scope = parse_scope()) {
        elif->scope = scope.value();
      } else {
        std::cerr << "Expected scope" << std::endl;
        exit(EXIT_FAILURE);
      }

      try_consume(TokenType::semi);
      elif->pred = parse_if_pred();
      auto pred = m_allocator.emplace<NodeIfPred>(elif);
      return pred;
    }

    if (try_consume(TokenType::else_)) {
      auto else_ = m_allocator.alloc<NodePredElse>();
      if (const auto scope = parse_scope()) {
        else_->scope = scope.value();
      } else {
        std::cerr << "Expected scope" << std::endl;
        exit(EXIT_FAILURE);
      }

      try_consume(TokenType::semi);
      auto pred = m_allocator.emplace<NodeIfPred>(else_);
      return pred;
    }
    return {};
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

        try_consume(TokenType::close_paren, "Expected `)`");
        try_consume(TokenType::semi, "Expected a end line or ; 1");
      }
    }
    m_index = 0;
    return exit_node;
  }

  std::optional<NodeScope*> parse_scope() {
    if (!try_consume(TokenType::open_curly).has_value()) {
      return {};
    }
    
    try_consume(TokenType::semi); // because \n is considered as ;
    auto scope = m_allocator.alloc<NodeScope>();
    while (auto stmt = parse_statement()) {
      scope->stmts.push_back(stmt.value());
    }
    Token token = try_consume(TokenType::close_curly, "Expected '}'");
    try_consume(TokenType::semi); // because \n is considered as ;
    return scope;
  }

  std::optional<NodeStatement*> parse_statement() {
    if (!peek().has_value()) {
      return {};
    }

    try_consume(TokenType::semi); // consume empty line
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

      try_consume(TokenType::close_paren, "Expected `)`");
      try_consume(TokenType::semi, "Expected a end line or ; 2");
      
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
      stmt_int->ident = consume(); // consumes identifier "x"
      consume();
      if (auto expr = parse_expr()) { // 5
        stmt_int->expr = expr.value(); 
      } else {
        std::cerr << "Invalid expression" << std::endl;
        exit(EXIT_FAILURE);
      }

      try_consume(TokenType::semi, "Expected a end line or ; 3");

      auto stmt = m_allocator.alloc<NodeStatement>();
      stmt->var = stmt_int;
      return stmt;
    } else if (peek().has_value() && peek().value().type == TokenType::open_curly) {
      if (auto scope = parse_scope()) {
        auto stmt = m_allocator.alloc<NodeStatement>();
        stmt->var = scope.value();
        return stmt;
      } else {
         std::cerr << "Invalid scope" << std::endl;
        exit(EXIT_FAILURE);
      }
    } else if (auto if_ = try_consume(TokenType::if_)) {
      try_consume(TokenType::open_paren, "Expected '('");
      auto stmt_if = m_allocator.alloc<NodeStmtIf>();

      if (auto expr = parse_expr()) {
        stmt_if->expr = expr.value();
      } else {
        std::cerr << "Invalid expression" << std::endl;
        exit(EXIT_FAILURE);
      }
      
      try_consume(TokenType::close_paren, "Expected ')'");

      try_consume(TokenType::semi);
      if (auto scope = parse_scope()) {
        stmt_if->scope = scope.value();
      } else {
        std::cerr << "Invalid scope" << std::endl;
        exit(EXIT_FAILURE);
      }
      stmt_if->pred = parse_if_pred();
      auto stmt = m_allocator.alloc<NodeStatement>();
      stmt->var = stmt_if;
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
        // std::cout << to_string(peek()->type) << '\n';
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

  inline Token try_consume(TokenType token, const std::string& err_msg) {
    if (peek().has_value() && peek().value().type == token) {
      return consume();
    } else {
      std::cerr << err_msg << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  inline std::optional<Token> try_consume(TokenType token) {
    if (peek().has_value() && peek().value().type == token) {
      return consume();
    } else {
      return {};
    }
  }

  inline Token consume() {
    return m_tokens.at(m_index++);
  }
  ArenaAllocator m_allocator;
};

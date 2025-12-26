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

struct NodeTermNumber {
  Token number;
};

struct NodeTermIdent {
  Token ident;
};

struct NodeTermString {
  Token string;
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
  std::variant<NodeTermNumber*, NodeTermIdent*, NodeTermParen*, NodeTermString*> var;
};

struct NodeExpr {
  std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStatementString {
  Token ident;
  NodeExpr* expr;
};

struct NodeStatementExit {
  NodeExpr* expr;
};

struct NodeStatementPrint {
  NodeExpr* expr;
};

struct NodeStatementInt {
  Token ident;
  NodeExpr* expr;
};

struct NodeStatementFloat {
  Token ident;
  NodeExpr* expr;
};

struct NodeStatementAssign {
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

struct NodeStmtWhile {
  NodeExpr* expr;
  NodeScope* scope;
};
// I should use using instead of struct
struct NodeStatement {
  std::variant<NodeStatementExit*, NodeStatementPrint*, NodeStatementInt*, NodeScope*, NodeStmtIf*, NodeStatementFloat*, 
  NodeStatementString*, NodeStatementAssign*, NodeStmtWhile*> var;
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
    case TokenType::while_: return "while";
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
      auto term_number =  m_allocator.emplace<NodeTermNumber>(int_lit.value());
      auto term = m_allocator.emplace<NodeTerm>(term_number);
      return term;
    } else if (auto float_lit = try_consume(TokenType::float_lit)) {
      auto term_number = m_allocator.emplace<NodeTermNumber>(float_lit.value());
      auto term = m_allocator.emplace<NodeTerm>(term_number);
      return term;
    } else if (auto ident = try_consume(TokenType::ident)) {
      auto expr_ident = m_allocator.emplace<NodeTermIdent>(ident.value());
      auto term = m_allocator.emplace<NodeTerm>(expr_ident);
      return term;
    } else if (auto open_paren = try_consume(TokenType::open_paren)) {
      auto expr = parse_expr();
      if (!expr.has_value()) {
        std::cerr << "Expected expression" << std::endl;
        exit(EXIT_FAILURE);
      }

      try_consume(TokenType::close_paren, "Expected ')'");
      auto term_paren = m_allocator.emplace<NodeTermParen>(expr.value());
      auto term = m_allocator.emplace<NodeTerm>(term_paren);
      return term;
    } else if (try_consume(TokenType::quote_d)) {
      auto string_tok = try_consume(TokenType::string_lit, "Expected string literal after \"");
      // closing quote
      try_consume(TokenType::quote_d, "Expected closing \"");
      auto term_str = m_allocator.emplace<NodeTermString>(string_tok);
      auto term = m_allocator.emplace<NodeTerm>(term_str);
      return term;
    } else {
      return {};
    }
  }

  std::optional<NodeExpr*> parse_expr(int min_prio = 0) {
    NodeExpr* expr_lhs = nullptr;
    std::optional<NodeTerm*> term_lhs = parse_term();
    if (!term_lhs.has_value()) {
      if (try_consume(TokenType::minus)) {
        Token zero_token = {.type = TokenType::int_lit, .value = "0"};
        if (peek().has_value() && peek().value().type == TokenType::float_lit) {
          zero_token = {.type = TokenType::float_lit, .value = "0.0"};
        }

        auto term_number = m_allocator.emplace<NodeTermNumber>(zero_token);
        auto term_zero = m_allocator.emplace<NodeTerm>(term_number);
        auto expr_zero = m_allocator.emplace<NodeExpr>(term_zero);

        auto expr_rhs = parse_expr(2); // Unary precedence
        if (!expr_rhs.has_value()) {
          std::cerr << "Expected expression after unary minus" << std::endl;
          exit(EXIT_FAILURE);
        }

        auto minus = m_allocator.emplace<NodeBinExprMinus>(expr_zero, expr_rhs.value());
        auto expr = m_allocator.emplace<NodeBinExpr>(minus);
        expr_lhs = m_allocator.emplace<NodeExpr>(expr);

      } else {
        return {};
      }
    } else {
      expr_lhs = m_allocator.emplace<NodeExpr>(term_lhs.value());
    }

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

      auto expr_lhs2 = m_allocator.emplace<NodeExpr>(expr_lhs->var);
      NodeBinExpr* expr = nullptr;
      if (op.type == TokenType::plus) {
        auto add = m_allocator.emplace<NodeBinExprAdd>(expr_lhs2, expr_rhs.value());
        expr = m_allocator.emplace<NodeBinExpr>(add);
      } else if (op.type == TokenType::star) {
        auto multi = m_allocator.emplace<NodeBinExprMulti>(expr_lhs2, expr_rhs.value());
        expr = m_allocator.emplace<NodeBinExpr>(multi);
      } else if (op.type == TokenType::minus) {
        // if lhs is not numb so neg numb
        auto minus = m_allocator.emplace<NodeBinExprMinus>(expr_lhs2, expr_rhs.value());
        expr = m_allocator.emplace<NodeBinExpr>(minus);
      } else if (op.type == TokenType::slash) {
        auto div = m_allocator.emplace<NodeBinExprDiv>(expr_lhs2, expr_rhs.value());
        expr = m_allocator.emplace<NodeBinExpr>(div);
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
      auto elif = m_allocator.emplace<NodePredElif>();
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
      auto else_ = m_allocator.emplace<NodePredElse>();
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
  
  std::optional<NodeScope*> parse_scope() {
    if (!try_consume(TokenType::open_curly).has_value()) {
      return {};
    }
    
    try_consume(TokenType::semi); // because \n is considered as ;
    auto scope = m_allocator.emplace<NodeScope>();
    while (auto stmt = parse_statement()) {
      scope->stmts.push_back(stmt.value());
    }
    Token token = try_consume(TokenType::close_curly, "Expected '}'");
    try_consume(TokenType::semi); // because \n is considered as ;
    return scope;
  }

  std::optional<NodeStatement*> parse_statement() {
    while (try_consume(TokenType::semi)) {} // consume empty lines
    if (!peek().has_value()) {
      return {};
    }

    if (peek().has_value() && peek().value().type == TokenType::exit && peek(1).has_value() 
      && peek(1).value().type == TokenType::open_paren) { 
      // exit(..
      consume();
      consume();
      auto stmt_exit = m_allocator.emplace<NodeStatementExit>();

      if (auto node_expr = parse_expr()) {
        stmt_exit->expr = node_expr.value();
      } else {
        std::cerr << "Invalid expression 1" << std::endl;
        exit(EXIT_FAILURE);
      }

      try_consume(TokenType::close_paren, "Expected `)`");
      try_consume(TokenType::semi, "Expected a end line or ; 2");
      
      auto stmt = m_allocator.emplace<NodeStatement>(stmt_exit);

      return stmt;
    } else if (peek().has_value() && peek().value().type == TokenType::print && peek(1).has_value() 
      && peek(1).value().type == TokenType::open_paren) { 
      // print(..
      consume();
      consume();
      auto stmt_exit = m_allocator.emplace<NodeStatementPrint>();

      if (auto node_expr = parse_expr()) {
        stmt_exit->expr = node_expr.value();
      } else {
        std::cerr << "Invalid expression 1" << std::endl;
        exit(EXIT_FAILURE);
      }

      try_consume(TokenType::close_paren, "Expected `)`");
      try_consume(TokenType::semi, "Expected a end line or ; 2");
      
      auto stmt = m_allocator.emplace<NodeStatement>(stmt_exit);

      return stmt;
    } else if (peek().has_value() && peek().value().type == TokenType::int_type
      && peek(1).has_value() && peek(1).value().type == TokenType::ident
        && peek(2).has_value() && peek(2).value().type == TokenType::eq) {
      // int x = 5
      consume(); //int 
      auto stmt_int = m_allocator.emplace<NodeStatementInt>();
      stmt_int->ident = consume(); // consumes identifier "x"
      consume();
      if (auto expr = parse_expr()) { // 5
        stmt_int->expr = expr.value(); 
      } else {
        std::cerr << "Invalid expression 2" << std::endl;
        exit(EXIT_FAILURE);
      }

      try_consume(TokenType::semi, "Expected a end line or ; 3");

      auto stmt = m_allocator.emplace<NodeStatement>(stmt_int);
      return stmt;
    } else if (peek().has_value() && peek().value().type == TokenType::float_type
      && peek(1).has_value() && peek(1).value().type == TokenType::ident
        && peek(2).has_value() && peek(2).value().type == TokenType::eq) {
      // float x = 5.2
      consume(); //float 
      auto stmt_float = m_allocator.emplace<NodeStatementFloat>();
      stmt_float->ident = consume(); // consumes identifier "x"
      consume();
      if (auto expr = parse_expr()) { // 5.2
        stmt_float->expr = expr.value(); 
      } else {
        std::cerr << "Invalid expression 3" << std::endl;
        exit(EXIT_FAILURE);
      }

      try_consume(TokenType::semi, "Expected a end line or ; 3");

      auto stmt = m_allocator.emplace<NodeStatement>(stmt_float);
      return stmt;
    } else if (peek().has_value() && peek().value().type == TokenType::string_type
      && peek(1).has_value() && peek(1).value().type == TokenType::ident
        && peek(2).has_value() && peek(2).value().type == TokenType::eq) {
      // string s = "..."
      consume(); // string
      auto stmt_string = m_allocator.emplace<NodeStatementString>();
      stmt_string->ident = consume(); // identifier
      consume(); // '='
      if (auto expr = parse_expr()) {
        stmt_string->expr = expr.value();
      } else {
        std::cerr << "Invalid expression for string assignment" << std::endl;
        exit(EXIT_FAILURE);
      }
      try_consume(TokenType::semi, "Expected a end line or ; 4");
      auto stmt = m_allocator.emplace<NodeStatement>(stmt_string);
      return stmt;
    } else if (peek().has_value() && peek().value().type == TokenType::open_curly) {
      if (auto scope = parse_scope()) {
        auto stmt = m_allocator.emplace<NodeStatement>(scope.value());
        return stmt;
      } else {
         std::cerr << "Invalid scope" << std::endl;
        exit(EXIT_FAILURE);
      }
    } else if (auto if_ = try_consume(TokenType::if_)) {
      try_consume(TokenType::open_paren, "Expected '('");
      auto stmt_if = m_allocator.emplace<NodeStmtIf>();

      if (auto expr = parse_expr()) {
        stmt_if->expr = expr.value();
      } else {
        std::cerr << "Invalid expression 4" << std::endl;
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
      auto stmt = m_allocator.emplace<NodeStatement>(stmt_if);
      return stmt;
    } else if (auto while_ = try_consume(TokenType::while_)) {
      try_consume(TokenType::open_paren, "Expected '('");
      auto stmt_while = m_allocator.emplace<NodeStmtWhile>();
      if (auto expr = parse_expr()) {
        stmt_while->expr = expr.value();
      } else {
        std::cerr << "Invalid expression in while" << std::endl;
        exit(EXIT_FAILURE);
      }
      try_consume(TokenType::close_paren, "Expected ')'");
      try_consume(TokenType::semi);
      if (auto scope = parse_scope()) {
        stmt_while->scope = scope.value();
      } else {
        std::cerr << "Invalid scope in while" << std::endl;
        exit(EXIT_FAILURE);
      }
      auto stmt = m_allocator.emplace<NodeStatement>(stmt_while);
      return stmt;
    } else if (peek().has_value() && peek().value().type == TokenType::ident
      && peek(1).has_value() && peek(1).value().type == TokenType::eq) {
      // x = expr
      auto assign = m_allocator.emplace<NodeStatementAssign>();
      assign->ident = consume();
      consume(); // '='
      if (auto expr = parse_expr()) {
        assign->expr = expr.value();
      } else {
        std::cerr << "Invalid expression in assignment" << std::endl;
        exit(EXIT_FAILURE);
      }
      try_consume(TokenType::semi, "Expected a end line or ; for assignment");
      auto stmt = m_allocator.emplace<NodeStatement>(assign);
      return stmt;
    } else {
      return {};
    }
  }

  std::optional<NodeProg> parse_prog() {
    NodeProg prog;
    while (peek().has_value()) {
      if (auto stmt = parse_statement()) {
        // std::cout << to_string(peek()->type) << '\n';
        prog.stmts.push_back(stmt.value());
      } else {
        if (!peek().has_value()) {
            break;
        }
        // std::cout << to_string(peek()->type) << '\n';
        std::cerr << "Invalid statement at token: " << std::to_string(static_cast<int>(peek().value().type))
          << "\nFor token " << to_string(peek()->type) << std::endl;
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

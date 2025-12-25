# pragma once
#include "./parser.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

// File to generate the asmebly code



class Generator {
public:
  inline Generator(NodeProg prog) 
    : m_prog(std::move(prog)) {

  }

  void gen_term(const NodeTerm* term) {
    struct TermVisitor {
      Generator& gen;

      void operator()(const NodeTermIntLit* term_int_lit) const {
        gen.m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << "\n";
        gen.push("rax");
      }

      void operator()(const NodeTermIdent* term_ident) const {
        auto it = std::find_if(gen.m_vars.cbegin(), gen.m_vars.cend(), [&](const Var& var) {
        return var.name == term_ident->ident.value.value();
      });
        if (it == gen.m_vars.cend()) {
          std::cerr << "Undeclared identifier: " << term_ident->ident.value.value() << "\n";
          exit(EXIT_FAILURE);
        }
        std::stringstream offset;
        offset << "QWORD [rsp + " << (gen.m_stack_size - it->stack_loc - 1) * 8 << "]";

        gen.push(offset.str());
      }

      void operator()(const NodeTermParen* term_paren) {
        gen.gen_expr(term_paren->expr);
      }
    };

    TermVisitor visitor({.gen = *this});
    std::visit(visitor, term->var);
  }

  void gen_bin_expr(const NodeBinExpr* bin_expr) {
    struct BinExprVisitor {
      Generator& gen;
      void operator()(const NodeBinExprAdd* add) const {
        gen.gen_expr(add->rhs);
        gen.gen_expr(add->lhs); // pushed on the top of the stack
        gen.pop("rax");
        gen.pop("rbx");
        gen.m_output << "    add rax, rbx\n";
        gen.push("rax");
      }

      void operator()(const NodeBinExprMulti* mult) const {
        gen.gen_expr(mult->rhs);
        gen.gen_expr(mult->lhs); // pushed on the top of the stack
        gen.pop("rax");
        gen.pop("rbx");
        gen.m_output << "    mul rbx\n";
        gen.push("rax");
      }

      void operator()(const NodeBinExprMinus* minus) const {
        gen.gen_expr(minus->rhs);
        gen.gen_expr(minus->lhs); // pushed on the top of the stack
        gen.pop("rax");
        gen.pop("rbx");
        gen.m_output << "    sub rax, rbx\n";
        gen.push("rax");
      }

      void operator()(const NodeBinExprDiv* div) const {
        gen.gen_expr(div->rhs);
        gen.gen_expr(div->lhs); // pushed on the top of the stack
        gen.pop("rax");
        gen.pop("rbx");
        gen.m_output << "    div rbx\n";
        gen.push("rax");
      }
    };

    BinExprVisitor visitor = {.gen = *this};
    std::visit(visitor, bin_expr->var);
  }
  void gen_expr(const NodeExpr* expr) {
    struct ExprVisitor {
      Generator& gen;

      void operator()(const NodeTerm* term) const { // can only push 64 bit
        gen.gen_term(term);
      }

      void operator()(const NodeBinExpr* bin_expr) const {
        gen.gen_bin_expr(bin_expr);
      }
    };

    ExprVisitor visitor{.gen = *this};
    std::visit(visitor, expr->var) ;
  }

  void gen_if_pred(const NodeIfPred* pred, const std::string& end_label) {
    struct PredVisitor {
      Generator& gen;
      const std::string& end_label;

      void operator()(const NodePredElif* elif) const
      {
          gen.m_output << "    ;; elif\n";
          gen.gen_expr(elif->expr);
          gen.pop("rax");
          const std::string label = gen.create_label();
          gen.m_output << "    test rax, rax\n";
          gen.m_output << "    jz " << label << "\n";
          gen.gen_scope(elif->scope);
          gen.m_output << "    jmp " << end_label << "\n";
          if (elif->pred.has_value()) {
              gen.m_output << label << ":\n";
              gen.gen_if_pred(elif->pred.value(), end_label);
          }
      }

      void operator()(const NodePredElse* else_) const
      {
          gen.m_output << "    ;; else\n";
          gen.gen_scope(else_->scope);
      }

    };

    PredVisitor visitor { .gen = *this, .end_label = end_label };
    std::visit(visitor, pred->var);
  }

  void gen_stmt(const NodeStatement* stmt) {
    struct StmtVisitor {
      Generator& gen;
      void operator()(const NodeStatementExit* stmt_exit) const {
        gen.gen_expr(stmt_exit->expr); 
        gen.m_output << "    mov rax, 60\n";
        gen.pop("rdi");
        gen.m_output << "    syscall\n";
      }

      void operator()(const NodeStatementInt* stmt_int) {
        auto it = std::find_if(gen.m_vars.cbegin(), gen.m_vars.cend(), [&](const Var& var) {
        return var.name == stmt_int->ident.value.value();
      });
        if (it != gen.m_vars.cend()) {
          std::cerr << "Undeclared identifier: " << stmt_int->ident.value.value() << "\n";
          exit(EXIT_FAILURE);
        }
        
        gen.m_vars.push_back({ .name = stmt_int->ident.value.value(), .stack_loc = gen.m_stack_size });
        gen.gen_expr(stmt_int->expr);
      }

      void operator()(const NodeScope* scope) const {
        gen.gen_scope(scope);
      }

      void operator()(const NodeStmtIf* stmt_if) const {
        gen.m_output << "    ;; if\n";
        gen.gen_expr(stmt_if->expr);
        gen.pop("rax");
        const std::string label = gen.create_label();
        gen.m_output << "    test rax, rax\n";
        gen.m_output << "    jz " << label << "\n";
        gen.gen_scope(stmt_if->scope);
        if (stmt_if->pred.has_value()) {
            const std::string end_label = gen.create_label();
            gen.m_output << "    jmp " << end_label << "\n";
            gen.m_output << label << ":\n";
            gen.gen_if_pred(stmt_if->pred.value(), end_label);
            gen.m_output << end_label << ":\n";
        }
        else {
            gen.m_output << label << ":\n";
        }
        gen.m_output << "    ;; /if\n";
      }
    };

    StmtVisitor visitor{ .gen = *this };
    std::visit(visitor, stmt->var);
  } 

  void gen_scope(const NodeScope* scope) {
    begin_scope();
    for (const NodeStatement*stmt : scope->stmts) {
      gen_stmt(stmt);
    }
    end_scope();
  }

  [[nodiscard]] std::string gen_prog() {
    std::stringstream output;
    m_output << "global _start\n_start:\n";

    for (const NodeStatement* stmt : m_prog.stmts) {
      gen_stmt(stmt);
    }

    m_output << "    mov rax, 60\n";
    m_output << "    mov rdi, 0\n";
    m_output << "    syscall\n";
    return m_output.str();
  }

private:

  struct Var {
    std::string name;
    size_t stack_loc;
    // some type ?
  };

  void pop(const std::string& reg) {
    m_output << "    pop " << reg << "\n";
    m_stack_size--;
  }
  void push(const std::string& reg) {
    m_output << "    push " << reg << "\n";
    m_stack_size++;
  }

  std::string create_label() {
      std::stringstream ss;
      ss << "label" << m_label_count++;
      return ss.str();
  }

  void begin_scope() {
    m_scopes.push_back(m_vars.size());
  }

  void end_scope() { //pop var until last begin scope
    size_t pop_count = m_vars.size() - m_scopes.back();
    m_output << "    add rsp, " << pop_count * 8 << "\n"; // * 8 because we only have 8 bytes integers
    m_stack_size -= pop_count;
    for (int i = 0; i < pop_count; i++) {
      m_vars.pop_back();
    }
    m_scopes.pop_back();
  }
  const NodeProg m_prog;
  std::stringstream m_output;
  std::size_t m_stack_size = 0;
  std::vector<Var> m_vars {};
  std::vector<size_t> m_scopes {};
  int m_label_count = 0;
};

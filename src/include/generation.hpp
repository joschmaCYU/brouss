# pragma once
#include "./parser.hpp"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>

// File to generate the asmebly code

class Generator {
public:
  inline Generator(NodeProg prog) 
    : m_prog(std::move(prog)) {

  }

  void gen_term(const NodeTerm* term) {
    struct TermVisitor {
      Generator* gen;

      void operator()(const NodeTermIntLit* term_int_lit) const {
        gen->m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << "\n";
        gen->push("rax");
      }

      void operator()(const NodeTermIdent* term_ident) const {
        if (!gen->m_vars.contains(term_ident->ident.value.value())) {
          std::cerr << "Undeclared identifier: " << term_ident->ident.value.value() << "\n";
          exit(EXIT_FAILURE);
        }
        const auto& var = gen->m_vars.at(term_ident->ident.value.value());
        std::stringstream offset;
        offset << "QWORD [rsp + " << 8 * (gen->m_stack_size - var.stack_loc - 1) << "]\n";

        gen->push(offset.str());
      }
    };

    TermVisitor visitor({.gen = this});
    std::visit(visitor, term->var);
  }

    void gen_exp(const NodeExpr* expr) {
    struct ExprVisitor {
      Generator* gen;

      void operator()(const NodeTerm* term) const { // can only push 64 bit
        gen->gen_term(term);
      }

      void operator()(const NodeBinExpr* bin_expr) const {
        assert(false); // Not implemented
      }
    };

    ExprVisitor visitor{.gen = this};
    std::visit(visitor, expr->var) ;
  }

  void gen_stmt(const NodeStatement* stmt) {
    struct StmtVisitor {
      Generator* gen;
      void operator()(const NodeStatementExit* stmt_exit) const {
        gen->gen_exp(stmt_exit->expr); 
        gen->m_output << "    mov rax, 60\n";
        gen->pop("rdi");
        gen->m_output << "    syscall\n";
      }

      void operator()(const NodeStatementInt* stmt_int) {
        if (gen->m_vars.contains(stmt_int->ident->value.value())) {
           std::cerr << "Identifier already used: " << stmt_int->ident->value.value() << "\n";
           exit(EXIT_FAILURE);
        }
        
        gen->m_vars.insert({stmt_int->ident->value.value(), Var { 
          .stack_loc = gen->m_stack_size }});
        
        gen->gen_exp(stmt_int->expr);
      }
    };

    StmtVisitor visitor{ .gen = this };
    std::visit(visitor, stmt->var);
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
    std::size_t stack_loc;
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

  const NodeProg m_prog;
  std::stringstream m_output;
  std::size_t m_stack_size = 0;
  std::unordered_map<std::string, Var> m_vars;
};

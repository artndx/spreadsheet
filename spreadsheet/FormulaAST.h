#pragma once

#include <FormulaLexer.h>
#include "common.h"

#include <forward_list>
#include <functional>
#include <stdexcept>

namespace ASTImpl {
class Expr;
}

class ParsingError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class FormulaAST {
public:
    explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr,
                        std::forward_list<Position> cells);
    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;
    ~FormulaAST();

    double Execute(std::function<double(Position)> value_getter) const;
    void PrintCells(std::ostream& out) const;
    void Print(std::ostream& out) const;
    void PrintFormula(std::ostream& out) const;

    std::set<Position> GetCells() {
        std::set<Position> poses;
        for(Position pos : cells_){
            poses.insert(pos);
        }
        return poses;
    }

    const std::set<Position> GetCells() const {
        std::set<Position> poses;
        for(Position pos : cells_){
            poses.insert(pos);
        }
        return poses;
    }

private:
    std::unique_ptr<ASTImpl::Expr> root_expr_;
    // physically stores cells so that they can be
    // efficiently traversed without going through
    // the whole AST
    std::forward_list<Position> cells_;
};

FormulaAST ParseFormulaAST(std::istream& in);
FormulaAST ParseFormulaAST(const std::string& in_str);
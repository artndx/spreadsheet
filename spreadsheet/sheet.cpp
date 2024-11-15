#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm> 
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    IsValidPos(pos);
    // Добавление ячейки
    std::unique_ptr<Cell> cell = std::make_unique<Cell>(*this);
    cell->Set(text);

    FindCircularDependency(cell.get(), pos);
    cells_[pos] = std::move(cell);

    // Добавление позиции для расчета
    // минимальной печатной области
    poses_.insert(pos);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    IsValidPos(pos);

    if(cells_.count(pos) == 0){
        return nullptr;
    }

    return cells_.at(pos).get();
}

CellInterface* Sheet::GetCell(Position pos) {
    IsValidPos(pos);

    if(cells_.count(pos) == 0){
        return nullptr;
    }

    return cells_.at(pos).get();
}

Cell* Sheet::GetChangeableCell(Position pos){
    IsValidPos(pos);

    if(cells_.count(pos) == 0){
        return nullptr;
    }

    return cells_.at(pos).get();
}

void Sheet::ClearCell(Position pos) {
    IsValidPos(pos);

    if(cells_.count(pos) == 0){
        return;
    }
    
    // Удаление ячейки
    cells_.erase(pos);

    // Удаление позиции
    poses_.erase(pos);
}

Size Sheet::GetPrintableSize() const {
    Size new_printable_size {0, 0};
    for(const Position& pos : poses_){
        new_printable_size.rows = std::max(new_printable_size.rows, pos.row + 1);
        new_printable_size.cols = std::max(new_printable_size.cols, pos.col + 1);
    }

    return new_printable_size;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size printable_size = GetPrintableSize();
    for(int row = 0; row < printable_size.rows; ++row){
        for(int col = 0; col < printable_size.cols; ++col){
            Position pos{row, col};
            if(cells_.count(pos) > 0){
                CellInterface::Value val = cells_.at(pos)->GetValue();
                if(std::holds_alternative<double>(val)){
                    output << std::get<double>(val);
                } else if(std::holds_alternative<std::string>(val)){
                    output << std::get<std::string>(val);
                } else if(std::holds_alternative<FormulaError>(val)){
                    output << std::get<FormulaError>(val);
                }
            }
            if(col != printable_size.cols - 1){
                output << "\t";
            }
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size printable_size = GetPrintableSize();
    for(int row = 0; row < printable_size.rows; ++row){
        for(int col = 0; col < printable_size.cols; ++col){
            Position pos{row, col};
            if(cells_.count(pos) > 0){
                output << cells_.at(pos)->GetText(); 
            }

            if(col != printable_size.cols - 1){
                output << "\t";
            }
        }
        output << "\n";
    }
}

void Sheet::IsValidPos(const Position& pos) const {
    if(!pos.IsValid()){
        throw InvalidPositionException("Invalid cell position"); 
    }
}

void Sheet::FindCircularDependency(const CellInterface* cell, std::unordered_set<Position, PositionHasher>& passed_cells) const{
    for (Position ref_pos : cell->GetReferencedCells()) {
        if (passed_cells.count(ref_pos) > 0) {
            throw CircularDependencyException("there is circular dependency"s);
        }
        passed_cells.insert(ref_pos);
        if (cells_.count(ref_pos)) {
            FindCircularDependency(GetCell(ref_pos), passed_cells);
        }
    }
}
    
void Sheet::FindCircularDependency(const CellInterface* cell, Position pos) const{
    std::unordered_set<Position, PositionHasher> passed_cells;
    passed_cells.insert(pos);
    FindCircularDependency(cell, passed_cells);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Cell;

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    Cell* GetChangeableCell(Position pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами
private:
    void IsValidPos(const Position& pos) const;
    void FindCircularDependency(const CellInterface* cell, std::unordered_set<Position, PositionHasher>& passed_cells) const;
    void FindCircularDependency(const CellInterface* cell, Position pos) const;
    std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher> cells_;
    std::unordered_set<Position, PositionHasher> poses_;
};
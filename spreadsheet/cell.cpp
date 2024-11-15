#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

/*========================================*/            /* Реализация пустого типа ячейки */
Cell::EmptyImpl::EmptyImpl() = default;
void Cell::EmptyImpl::Clear(){
    content_.clear();
}

Cell::Value Cell::EmptyImpl::GetValue() const{
    return content_;
}

std::string Cell::EmptyImpl::GetText() const {
    return content_;
}

/*========================================*/            /* Реализация текстового типа ячейки */
Cell::TextImpl::TextImpl(std::string str)
: content_(str){}

void Cell::TextImpl::Clear(){
    content_.clear();
}

Cell::Value Cell::TextImpl::GetValue() const{
    if(content_[0] == ESCAPE_SIGN){
        return content_.substr(1);
    }

    return content_;
}

std::string Cell::TextImpl::GetText() const {
    return content_;
}

/*========================================*/            /* Реализация формульного типа ячейки */
Cell::FormulaImpl::FormulaImpl(std::string str, SheetInterface& sheet)
: formula_(ParseFormula(str.substr(1))), sheet_(sheet){}

void Cell::FormulaImpl::Clear(){
    formula_.release();
}

Cell::Value Cell::FormulaImpl::GetValue() const{
    if(!cache_.has_value()){
        try{
            cache_ = formula_->Evaluate(sheet_);
        } catch(const FormulaException& exc){
            throw exc;
        }
    }

    if(std::holds_alternative<double>(*cache_)){
        return std::get<double>(*cache_);
    } else {
        return std::get<FormulaError>(*cache_);
    }
}

std::string Cell::FormulaImpl::GetText() const {
    std::string formula;
    try {
        formula = formula_->GetExpression();
    } catch(const FormulaException& exc){
        throw exc;
    }
    return FORMULA_SIGN + formula;
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

bool Cell::FormulaImpl::HasCache() const{
    return cache_.has_value();
}

void Cell::FormulaImpl::InvalidateCache(){
    cache_.reset();
}

/*========================================*/  

// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
:impl_(std::make_unique<EmptyImpl>()), sheet_(sheet){}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> temp = std::make_unique<EmptyImpl>();
    if(text[0] == FORMULA_SIGN && text.size() > 1){ 
        temp = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    } else if(!text.empty()) {
        temp = std::make_unique<TextImpl>(text);
    }

    impl_.swap(temp);

    UpdateReferences();
    InvalidateCache(true);
}

void Cell::Clear() {
    Set("");
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const{
    return !dependent_cells_.empty();
}

void Cell::InvalidateCache(bool flag){
    if(impl_->HasCache() || flag){
        impl_->InvalidateCache();
        for(Cell* dependent : dependent_cells_){
            dependent->InvalidateCache();
        }
    }
}

// Проходя по каждой ссылке,
// добавляет указатель на текующую ячейку 
// в контейнер ячеек dependent_cells_ , от которых зависит текущая ячейка
void Cell::UpdateReferences(){
    for(Position pos : impl_->GetReferencedCells()){
        // Если ссылка до этого не создавалась
        // то ячейку нужно создать как пустую
        if(sheet_.GetCell(pos) == nullptr){
            sheet_.SetCell(pos,"");
        }
        Cell* ref_cell = sheet_.GetChangeableCell(pos);
        ref_cell->dependent_cells_.insert(this);
    }
}
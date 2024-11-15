#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;
private:
    /*========================================*/            /* Базовый тип ячейки */
    class Impl{
    public:
        virtual void Clear() = 0;
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        
        virtual std::vector<Position> GetReferencedCells() const{return {};}
        virtual bool HasCache() const{return true;}
        virtual void InvalidateCache(){return;}
        
        virtual ~Impl() = default;
    };
    /*========================================*/            /* Пустой тип ячейки */
    class EmptyImpl : public Impl{
    public:
        EmptyImpl();
        void Clear() override;
        Value GetValue() const override;
        std::string GetText() const override;
    private:
        std::string content_ = "";
    };
    /*========================================*/            /* Текстовый тип ячейки */
    class TextImpl : public Impl{
    public:
        TextImpl(std::string);
        void Clear() override;  
        Value GetValue() const override;
        std::string GetText() const override;
    private:
        std::string content_;
    };
    /*========================================*/            /* Формульный тип ячейки */
    class FormulaImpl : public Impl{
    public:
        FormulaImpl(std::string, SheetInterface&);
        void Clear() override; 
        Value GetValue() const override;
        std::string GetText() const override;
        
        std::vector<Position> GetReferencedCells() const override;
        bool HasCache() const override;
        void InvalidateCache() override;

    private:
        std::unique_ptr<FormulaInterface> formula_;
        mutable std::optional<FormulaInterface::Value> cache_;
        SheetInterface& sheet_;
    };
    /*========================================*/
    void UpdateReferences();
    bool IsReferenced() const;
    void InvalidateCache(bool flag = false);
    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    std::unordered_set<Cell*> dependent_cells_;     // Ячейки, от которых зависит текущая ячейка
};
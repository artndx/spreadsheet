#include "formula.h"

#include "FormulaAST.h"

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    switch (fe.GetCategory())
    {
    case FormulaError::Category::Ref:
        output << "#REF!";
        break;
    case FormulaError::Category::Value:
        output << "#VAL!";
        break;
    case FormulaError::Category::Arithmetic:
        output << "#ARITHM!";
        break;
    default:
        break;
    }
    return output;
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression)
    try : ast_(ParseFormulaAST(expression)){}
    catch (...){
        throw FormulaException("formula is incorrect");
    }
    
    Value Evaluate(const SheetInterface& sheet) const override{
        try{
            std::function<double(Position)> value_getter = GetValueGetter(sheet);
            double res = ast_.Execute(value_getter);
            if(std::abs(res) == std::numeric_limits<double>::infinity()){
                return FormulaError(FormulaError::Category::Arithmetic);
            }
            return res;
        } catch(const FormulaError& exc){
            return exc;
        }
    } 

    std::string GetExpression() const override{
        std::ostringstream result_out;
        ast_.PrintFormula(result_out);
        return result_out.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> poses;
        for(Position pos : ast_.GetCells()){
            if(pos.IsValid()){
                poses.push_back(pos);
            }
        }
        return poses;
    }

private:
    static double GetValueFromString(const std::string& str){
        if(str.empty()){
            return 0.0;
        }
        try{
            return std::stod(str);
        } catch(...){
            throw FormulaError(FormulaError::Category::Value);
        }

        return 0.0;
    }

    std::function<double(Position)> GetValueGetter(const SheetInterface& sheet) const {
        std::function<double(Position)> value_getter = [&sheet](Position pos){
                if(!pos.IsValid()){
                    throw FormulaError(FormulaError::Category::Ref);
                }

                const CellInterface* cell = sheet.GetCell(pos);
                if(cell == nullptr){
                    return 0.0;
                }

                CellInterface::Value value = cell->GetValue();
                if(std::holds_alternative<double>(value)){
                    return std::get<double>(value);
                } else if(std::holds_alternative<std::string>(value)){
                    return GetValueFromString(std::get<std::string>(value));
                } else {
                    throw FormulaError(std::get<FormulaError>(value));
                }
                
            };
            
        return value_getter;
    }

    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
#include "formula.h"
#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
	return output << "#DIV/0!";
}

namespace {
class Formula : public FormulaInterface {
public:
	explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {
	}

	Value Evaluate(const SheetInterface& sheet) const override {
		double result = 0.0;
		try {
			auto lambda = [this, &sheet]() mutable {
				//for (auto position : this->GetReferencedCells()) {
				//}
			};
			result = ast_.Execute(sheet, lambda);
		} catch (const FormulaError& fe) {
			return fe;
		}
		return result;
	}

	std::string GetExpression() const override {
		std::stringstream stream;
		ast_.PrintFormula(stream);
		return stream.str();
	}

	std::vector<Position> GetReferencedCells() const override {
		std::set<Position> test(ast_.GetCells().begin(), ast_.GetCells().end());
		return {test.begin(), test.end()};
	}

private:
	FormulaAST ast_;
};

}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
	try {
		return std::make_unique<Formula>(std::move(expression));
	} catch (...) {
		throw FormulaException("Formula error");
	}
	return {};
}

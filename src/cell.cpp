#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>

class Cell::Impl {
public:
	virtual ~Impl() = default;
	virtual std::string GetText() const = 0;
	virtual CellInterface::Value GetValue(const SheetInterface& sheet) const = 0;
	virtual std::vector<Position> GetReferencedCells() const = 0;
	virtual void ClearValue() { };
};

class Cell::EmptyImpl : public Impl {
public:
	//EmptyImpl();
	std::string GetText() const override {
		return "";
	}

	CellInterface::Value GetValue(const SheetInterface& /*sheet*/) const override {
		return "";
	}

	std::vector<Position> GetReferencedCells() const override{
		return {};
	}
};

class Cell::TextImpl : public Impl {
public:	
	TextImpl(std::string text) : text_(text) { }
	
	std::string GetText() const override {
		return text_;
	}
	
	CellInterface::Value GetValue(const SheetInterface& /*sheet*/) const override {
		if (text_.front() == ESCAPE_SIGN) {
			return text_.substr(1, text_.size());
		}
		return text_;
	}

	std::vector<Position> GetReferencedCells() const override{
		return {};
	}

private:
	std::string text_;
};

class Cell::FormulaImpl : public Impl {
public:	
	FormulaImpl(std::string text) : formula_(ParseFormula(text))  { 
	}

	std::string GetText() const override {
		return FORMULA_SIGN + formula_->GetExpression();
	}

	CellInterface::Value GetValue(const SheetInterface &sheet) const override {
		if (value_.has_value()) {
			return value_.value();
		} else {
			auto result = formula_->Evaluate(sheet);
			if (std::holds_alternative<double>(result)) {
				double x = std::get<double>(result);
				auto ptr = const_cast<FormulaImpl*>(this);
				ptr->SetValue(x);
				return std::get<double>(result);
			} else {
				return std::get<FormulaError>(result);
			}
		}
	}

	std::vector<Position> GetReferencedCells() const override {
		return formula_->GetReferencedCells();
	}

	void ClearValue() override {
		value_.reset();
	}

	void SetValue(double result) {
		value_ = result;
	}

private:
	std::unique_ptr<FormulaInterface> formula_;
	std::optional<double> value_;  
};

Cell::Cell(Sheet& sheet) : sheet_(sheet) { 
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
	if ( text.front() != FORMULA_SIGN || (text.front() == FORMULA_SIGN && text.size() == 1)) {
		impl_ = std::make_unique<Cell::TextImpl>(text);
	} else if (text.front() == FORMULA_SIGN) {
		auto temp_impl = std::make_unique<Cell::FormulaImpl>(text.substr(1, text.size()));
		impl_ = std::move(temp_impl);
	} else {
		impl_ = std::make_unique<Cell::EmptyImpl>();
	}
}

void Cell::Clear() {
	impl_ = std::make_unique<Cell::EmptyImpl>();
	DeleteDepends();
	ResetBroadcastsCache();
}

Cell::Value Cell::GetValue() const {
	return impl_.get()->GetValue(sheet_);
}

std::string Cell::GetText() const {
	return impl_.get()->GetText();
}

std::vector<Position>  Cell::GetReferencedCells() const  {
	return impl_->GetReferencedCells();
}

bool  Cell::IsReferenced() const {
	return !broadcasts_.empty();
}

void Cell::SetDepends() { 
	for (const auto& depend_position : GetReferencedCells()) {
		auto depend = sheet_.FindOrCreateCell(depend_position);
		depend->broadcasts_.insert(this);
		depends_.insert(depend);
	}
}

void Cell::DeleteDepends() {
	for (auto depend : depends_) {
		depend->broadcasts_.erase(this);
	}
	depends_.clear();
}

void Cell::ResetCache() {
	impl_->ClearValue();
}
void Cell::ResetBroadcastsCache() {
	ResetCache(); 
	for (auto cast : broadcasts_) {
		cast->ResetBroadcastsCache();
	}
}

std::set<Cell*> Cell::GetBroadcastList() const {
	return broadcasts_;
}

void Cell::InsertBroadcastList(std::set<Cell*> broadcast) {
	broadcasts_ = broadcast;
}

void Cell::LoopCheck(const std::vector<Position> cells_pos, Position target, std::set<Cell*> checked_ptrs) const { 
	std::vector<Cell*> to_check;
	to_check.reserve(cells_pos.size() +1);
	for (const auto& depend_pos : cells_pos) {
		if (depend_pos == target) {
			throw CircularDependencyException("Loop!");
		}
		auto cell = dynamic_cast<Cell*>(sheet_.GetCell(depend_pos));
		if (cell && !checked_ptrs.count(cell)) { 
			checked_ptrs.insert(cell);
			to_check.push_back(cell);
		}
	}
	for (const auto cell : to_check) {
		 cell->LoopCheck(cell->GetReferencedCells(), target, checked_ptrs);
	}
}




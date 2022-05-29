#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {
	for (auto& input_vect : cells_ptr) {
		for (auto ptr : input_vect) {
			if  (ptr != nullptr) {
				delete ptr;
			}
		}
	}
}

void Sheet::SetCell(Position pos, std::string text) {
	if (!pos.IsValid()) {
		throw InvalidPositionException ("Position is not valid");
	}
	auto ptr =  std::unique_ptr<Cell>(new Cell(*this));
	ptr->Set(text); 
	ptr->LoopCheck(ptr->GetReferencedCells(), pos, {});
	if (cells_ptr.size() <= static_cast<size_t>(pos.row)) {
		AllocateRowAndCol(pos);
	} else {
		if (cells_ptr[pos.row].size() <= static_cast<size_t>(pos.col) ) {
			AllocateCol(pos);
		} else if (cells_ptr[pos.row][pos.col] != nullptr) {
			ptr->InsertBroadcastList(dynamic_cast<Cell*>(cells_ptr[pos.row][pos.col])->GetBroadcastList());
			delete cells_ptr[pos.row][pos.col];
		}
	}
	auto released_ptr = ptr.release();
	cells_ptr[pos.row][pos.col] = released_ptr;
	released_ptr->SetDepends();
}

void Sheet::AllocateRowAndCol(Position pos) {
	size_t new_size = static_cast<size_t>(pos.row) - cells_ptr.size() + 1;
	std::vector<std::vector<CellInterface*>> row_addin(new_size);
	auto it = cells_ptr.begin();
	cells_ptr.insert(std::next(it, cells_ptr.size()), row_addin.begin(), row_addin.end());
	std::vector<CellInterface*> col_addin(static_cast<size_t>(pos.col) + 1);
	cells_ptr[pos.row] = col_addin;
}

void Sheet::AllocateCol(Position pos) {
	size_t new_size = static_cast<size_t>(pos.col) - cells_ptr[pos.row].size() + 1;
	std::vector<CellInterface*> col_addin(new_size);
	cells_ptr[pos.row].insert(std::next(cells_ptr[pos.row].begin(), cells_ptr[pos.row].size()), col_addin.begin(), col_addin.end());
}

bool Sheet::PositionExists(Position pos) const{
	return cells_ptr.size() > static_cast<size_t>(pos.row) && cells_ptr[pos.row].size() > static_cast<size_t>(pos.col);
}

const CellInterface* Sheet::GetCell(Position pos) const {
	if (!pos.IsValid()) {
		throw InvalidPositionException ("invalid position");
	}
	return PositionExists(pos) ? cells_ptr[pos.row][pos.col] : nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
	if (!pos.IsValid()) {
		throw InvalidPositionException("invalid position");
	}
	return PositionExists(pos) ? cells_ptr[pos.row][pos.col] : nullptr;
}

void Sheet::ClearCell(Position pos) {
	if (!pos.IsValid()) {
		throw InvalidPositionException ("invalid position");
	}
	if (PositionExists(pos) && cells_ptr[pos.row][pos.col] != nullptr) {
		if ( dynamic_cast<Cell*>(cells_ptr[pos.row][pos.col])->IsReferenced()) {
			 dynamic_cast<Cell*>(cells_ptr[pos.row][pos.col])->Clear();
		} else {
			delete cells_ptr[pos.row][pos.col];
			cells_ptr[pos.row][pos.col] = nullptr;
			auto last_value = std::find_if(cells_ptr[pos.row].rbegin(), cells_ptr[pos.row].rend(), [](auto& ptr){ return ptr != nullptr; });
			cells_ptr[pos.row] = {cells_ptr[pos.row].rbegin(), last_value};
		}
	}
}

Size Sheet::GetPrintableSize() const {
	Size result;
	if (cells_ptr.size() == 0) {
		return result;
	} else {
		size_t temp_counter = 0;
		for (auto& row : cells_ptr) {
			int current_col_size = 0;
			for (int i = row.size()-1; i+1 > 0; --i) {
				if (row[i] != nullptr) {
					current_col_size = i+1;
					break;
				}
			}
			if (current_col_size) {
				++result.rows;
				result.rows += temp_counter;
				temp_counter = 0;
			} else {
				 ++temp_counter;
			}
			result.cols = std::max(result.cols, current_col_size);

		}
		return result;
	}
}

void Sheet::PrintValues(std::ostream& output) const {
	auto size = GetPrintableSize();
	for (int row = 0; row < size.rows; ++row) {
		for (int col = 0; col < size.cols; ++col) {
				if (cells_ptr[row].size() > static_cast<size_t>(col) && cells_ptr[row][col] != nullptr) {
					auto val = cells_ptr[row][col]->GetValue();
					if ( std::holds_alternative<std::string>(val) ) {
						output << std::get<std::string>(val);
					} else if ( std::holds_alternative<double>(val) ) {
						output << std::get<double>(val);
					} else if ( std::holds_alternative<FormulaError>(val) ) {
						output << std::get<FormulaError>(val);
					}
				}
				if (col != size.cols -1) {
					output << '\t';
				}
			}
		output << '\n';
	}
}

void Sheet::PrintTexts(std::ostream& output) const {
	auto size = GetPrintableSize();
	for (int row = 0; row < size.rows; ++row) {
		for (int col = 0; col < size.cols; ++col) {
			if (cells_ptr[row].size() > static_cast<size_t>(col) && cells_ptr[row][col] != nullptr) {
				output << cells_ptr[row][col]->GetText();
			}
			if (col != size.cols -1 ) {
				output << '\t';
			}
		}
		output << '\n';
	}
}

Cell* Sheet::FindOrCreateCell(Position pos) {
	auto ptr = GetCell(pos);
	if (ptr == nullptr) {
		SetCell(pos, "");
		ptr = GetCell(pos);
	}
	return dynamic_cast<Cell*>(ptr);
}


std::unique_ptr<SheetInterface> CreateSheet() {
	return std::make_unique<Sheet>();
}


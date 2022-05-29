#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <set>
#include <unordered_set>

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
	bool IsReferenced() const; 

	void LoopCheck(const std::vector<Position> cells_pos, Position target, std::set<Cell*> checked_ptrs) const;

	void SetDepends();
	void DeleteDepends();

	void ResetCache();
	void ResetBroadcastsCache();
	std::set<Cell*> GetBroadcastList() const;
	void InsertBroadcastList(std::set<Cell*> broadcast);

private:
	class Impl;
	class EmptyImpl;
	class TextImpl;
	class FormulaImpl;

	Sheet& sheet_;
	std::unique_ptr<Impl> impl_;

	std::set<Cell*> depends_; 
	std::set<Cell*> broadcasts_;

};

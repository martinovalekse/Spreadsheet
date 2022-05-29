#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
	~Sheet();

	void SetCell(Position pos, std::string text) override;

	const CellInterface* GetCell(Position pos) const override;
	CellInterface* GetCell(Position pos) override;

	bool PositionExists(Position pos) const;
	void ClearCell(Position pos) override;

	Size GetPrintableSize() const override;

	void PrintValues(std::ostream& output) const override;
	void PrintTexts(std::ostream& output) const override;

	Cell* FindOrCreateCell(Position pos);

private:
	std::vector<std::vector<CellInterface*>> cells_ptr;

	void AllocateRowAndCol(Position pos);
	void AllocateCol(Position pos);
};


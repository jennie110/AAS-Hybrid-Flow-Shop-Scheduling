#pragma once

class LotCompare
{
public:
	LotCompare(int _cLot, int cValue)
	{
		this->cLot = _cLot;
		this->cValue = cValue;
	}
	int cLot;
	int cValue;
};

class CompareLot
{
public:
	bool operator () (LotCompare &lot1, LotCompare &lot2)
	{
		return lot1.cValue < lot2.cValue;
	}
};

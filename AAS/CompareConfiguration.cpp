#pragma once

#include "Configuration.cpp"

class SortConfiguration
{
public:

	bool operator()(Configuration &obj1, Configuration &obj2)
	{
		return obj1.avgValue < obj2.avgValue;
	}
};

class SortConfigurationRankIndex
{
public:

	bool operator()(Configuration &obj1, Configuration &obj2)
	{
		return obj1.rankIndex < obj2.rankIndex;
	}
};


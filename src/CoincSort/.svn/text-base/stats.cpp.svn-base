/// \file	stats.cpp
/// \brief	Implementation of count statistics classes
/// \author	Axel Wagner
#include <numeric>

#include "stats.h"


Stats::Stats(int moduleCount)
:	counts((int)StatsIndex(moduleCount,0,0,0), counts_type()),
	spectra((int)StatsIndex(moduleCount,0,0,0), spectrum_type(SpectrumSize))
{
}

void Stats::Clear()
{
	counts_type zeroCounts;
	std::fill(counts.begin(), counts.end(), zeroCounts);

	spectrum_type zeroSpec(SpectrumSize);
	std::fill(spectra.begin(), spectra.end(), zeroSpec);
}

void Stats::Add(StatsIndex i, int energy, bool single)
{
	if (single)
		counts.at(i).single++;
	else
		counts.at(i).multi++;

	if (energy >= 0 && energy < SpectrumSize)
		spectra.at(i)[energy]++;
}

Stats::counts_type Stats::Sum(StatsIndex begin, StatsIndex end)
{
	return std::accumulate(counts.begin()+begin, counts.begin()+end, counts_type());
}

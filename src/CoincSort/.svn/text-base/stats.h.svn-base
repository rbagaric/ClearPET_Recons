/// \file	stats.h
/// \brief	Declaration of count statistics classes
/// \author	Axel Wagner
///
/// These statistics are used by tools working on singles data. Here we do not
/// need any error statistics, because errors are filtered out in the DAQ.
///
#ifndef stats_h_included
#define stats_h_included

#include <vector>
#include <stdexcept>

#include "inttypes.h"

/// \brief Multidimensional index for statistics data
///
/// This class serves as a wrapper for multidimensional indexing
/// on statistics data. Those data are stored in a large 1-dimensional
/// vector, but need to be accessed as a 4-dimensional array.
///
/// This class does all the required calculations to translate 4
/// index values, given in its ctor, into a 1-dimensional offset.
///
/// Additional helper functions are used
class StatsIndex {
public:
	enum {
		NumLayers = 2,
		NumPixels = 64,
		NumPmts = 4
	};

	StatsIndex(): vectorIndex(0) {}
	StatsIndex(unsigned int mod, unsigned int pmt, unsigned int pix, unsigned int layer)
	{
		if (layer >= NumLayers || pix >= NumPixels || pmt >= NumPmts)
			throw std::out_of_range("StatsIndex out of range");
		vectorIndex = layer + NumLayers*(pix + NumPixels*(pmt + NumPmts*mod));
	}

	operator int() { return vectorIndex; }

	void NextLayer() { ++vectorIndex; }
	void NextPixel() { vectorIndex += NumLayers; }
	void NextPMT() { vectorIndex += NumLayers*NumPixels; }
	void NextModule() { vectorIndex += NumLayers*NumPixels*NumPmts; }

private:
	int		vectorIndex;
};

/// Statistics data class
///
/// This class stores the statistical data for several modules.
/// This includes:
///  - Spectra for each pixel
///  - Single and multihit counts for each pixel
///
class Stats {
public:
	Stats(int moduleCount);

	enum {
		SpectrumSize = 256
	};

	struct counts_type {
        counts_type(uint32_t s = 0, uint32_t m = 0): single(s), multi(m) {}
        uint32_t	single;
		uint32_t	multi;

		counts_type& operator+= (counts_type rhs)
		{
			single += rhs.single;
			multi += rhs.multi;
			return *this;
		}

		friend counts_type operator+ (counts_type c1, counts_type c2)
		{
			return counts_type(c1.single+c2.single, c1.multi+c2.multi);
		}
	};

    typedef std::vector<uint32_t> spectrum_type;

	void Clear();
	void Add(StatsIndex i, int energy, bool single);

	counts_type Sum(StatsIndex begin, StatsIndex end);

	const spectrum_type&	GetSpectrum(StatsIndex i) const { return spectra.at(i); }
	const counts_type&		GetCounts(StatsIndex i) const { return counts.at(i); }

private:
	std::vector<counts_type>	counts;
	std::vector<spectrum_type>	spectra;
};

#endif // statistics_h_included

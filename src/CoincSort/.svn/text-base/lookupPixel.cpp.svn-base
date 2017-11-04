/// \file	lookupPixel.cpp
/// \brief	function LookupPixel()
///	\author	Axel Wagner
///

#include <vector>

#include "inttypes.h"
#include "lookupPixel.h"

// This is a helper class for pixel index lookup.
// There is only one static instance of this class in the program.
//
class PixelReverseLUT
{
public:
	PixelReverseLUT();

	uint8_t operator[](int index) { return lut[index]; }

private:
	std::vector<uint8_t>	lut;
};

// For some (TODO: yet undocumented) reason, we need to swap pixel numbers.
// To speed up this process, we build a lookup table.
// The pixel swap is actually an reversing of columns.
//
PixelReverseLUT::PixelReverseLUT()
:	lut(64)
{
	for (int i = 0; i < 64; ++i) {
		int row = i / 8;
		int col = i % 8;
		lut[i] = uint8_t(8*row + 7-col);
	}
}

static PixelReverseLUT	pixelReverseLUT;

int LookupPixel(int pix)
{
	return pixelReverseLUT[pix];
}


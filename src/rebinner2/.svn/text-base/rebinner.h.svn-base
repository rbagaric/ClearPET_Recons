///\file	rebinner.h
///\brief	class Rebinner and related declarations
///\author	Axel Wagner
#ifndef rebinner_h_included
#define rebinner_h_included

#include <iostream>
#include <string>

#include "noncopy.h"

struct CoincEvent;

///\brief Rebinner
///
/// This class is doing the rebinning. It reads coincidence events from the
/// LMF input file, calculates the LOR, and increments the corresponding
/// bin in the 3D sinogram.
///
class Rebinner: private noncopyable {
public:
	Rebinner();

	void Run(const std::string& coincFile, int tangentialDimension, int maxCount, const std::string& outputName);

	bool IsSilent() const { return isSilent; }
	bool IsVerbose() const { return isVerbose; }
	void SetNoise(bool silent, bool verbose)
	{
		isSilent = silent;
		isVerbose = verbose;
	}
	void SetFOV(int newFOV);

	void Report(std::ostream& os);

private:
	bool	isSilent;
	bool	isVerbose;
	int		minDeltaDet;	///< minimum differece of cassettes number allowed for coincidence, \see SetFOV()
	int		maxDeltaDet;	///< maximum differece of cassettes number allowed for coincidence, \see SetFOV()
	int		totalEventCount;
	int		goodEventCount;

	bool	InsideFOV(const CoincEvent& coinc) const;
};

#endif // rebinner_h_included

///\file	sinogram.h
///\brief	Declaration of class Sinogram
///\author	Axel Wagner
#ifndef sinogram_h_included
#define sinogram_h_included

#include <memory>
#include <string>
#include <vector>

#include "coincevent.h"
#include "lor.h"
#include "noncopy.h"

class BinStorage;
class Geometry;

///\brief 3D Sinogram
///
/// This class contains all data of a 3D sinogram, as needed by the rebinner.
///
class Sinogram: private noncopyable {
public:
	Sinogram(const Geometry& geometry, unsigned tangentialDimension, bool verbose);
	~Sinogram();

	bool Add(LOR lor);

	void SaveHeaderFile(const std::string& outputFile, const Geometry& geometry);
	void SaveDataFile(const std::string& outputFile);

private:
	bool	isVerbose;
	BinStorage*	bins;
	double	radius;
	double	ringSpacing;
	double	viewSpacing;
	double	binSize;
	std::vector<unsigned>	maxAxialNumber;
};

#endif // sinogram_h_included

///\file	geometry.h
///\brief	Declaration of class Geometry
///\author	Axel Wagner
#ifndef geometry_h_included
#define geometry_h_included

#include "clearpet.h"
#include "coincevent.h"
#include "lor.h"

#include <string>

///\brief Scanner Geometry Description
///
/// This class contains all data describing the scanner geometry. Its main
/// task is the translation of coincidence event data to LORs.
///
/// Some details of the scanner geometry are hard coded for efficiency reasons.
/// Other details are read from the geometry file which is passed into the
/// constructor.
///
class Geometry {
public:
	Geometry(const std::string& geomFile);

	LOR GetLOR(CoincEvent& coincEvent);

	double	GetCrystalDepth() const { return crystalDepth; }
	double	GetRadius() const { return radius; }
	double	GetRingSpacing() const { return crystalPitch; }

private:
	CartCoord	pixelPos[ClearPET::numPixels];	///< table of precalculated pixel positions
	double	crystalDepth;			///< thickness of crystals, in mm
	double	crystalPitch;			///< distance of crystals in axial and tangential direction, in mm
	double	modulePitch;			///< axial distance of modules, in mm
	double	sectorPitch;			///< angular distance of sectors, in rad
	double	axialSectorShift[2];	///< axial shift of even/odd sectors, in mm
	double	azimuthalStep;			///< step of gantry rotation angle
	double	radius;					///< radius of scanner, in mm

	CartCoord calculateCrystalPosition(uint16_t id, double rotation);
};

#endif // geometry_h_included

/**
\file	clearpet.h
\brief	Some constants of the ClearPET system
\author	Axel Wagner
*/
#ifndef clearpet_h_included
#define clearpet_h_included

namespace ClearPET
{

/// \defgroup ScannerConstants    Constants for ClearPET Scanner
///
/// The following constants define the number of elements
/// really existing in the scanner.
/// We use constants here for performance reasons, and because
/// they are really device constants in the ClearPET system.
///\@{
const int numSectors = 20;	///< Number of sectors (=cassettes) in the scanner
const int numModules = 4;	///< Number of modules (=PMTs) per sector 
const int numSubMods = 1;	///< Number of submodules, which are not used in ClearPET systems
const int numPixelsAxial = 8;	///< Number of pixels per module in axial direction
const int numPixelsTangential = 8;	///< Number of pixels per module in tagential direction
const int numPixels = numPixelsAxial*numPixelsTangential;	///< Total number of pixels per module
const int numLayers = 2;	///< Number of layers per pixel
const int numRings = 48;	///< total number of rings per scanner
const int numPixelsTotalTangential = numSectors*numPixelsTangential;	///< total number of pixels along tangential direction
///\@}

} // namespace ClearPET

#endif // clearpet_h_included

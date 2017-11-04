///\file	sinogram.cpp
///\brief	Implementation of class Sinogram
///\author	Axel Wagner

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdexcept>

#include "geometry.h"
#include "sinogram.h"

const double pi = 4.0*std::atan(1.0);
const int maxRingDifference = 15;

///\brief Index class for indexing the 4-dimensional sinogram bin storage
struct BinIndex
{
	unsigned		segment;
	unsigned		view;
	unsigned		ring;
	unsigned		tangential;
};

///\brief Storage class for the 4-dimensional sinogram data
///
///\note In this class, we take the ring dimension as fixed. Actually, this
/// dimension depends on the segment number (is smaller for large segments).
/// We use the maximum value for the ring dimension. This is a waste of
/// memory, but it makes the memory access simpler and faster.
class BinStorage: private noncopyable
{
public:
	typedef float data_type;

	///\brief Allocate memory for the given dimensions
	BinStorage(BinIndex dimensions)
	:	dim(dimensions)
	{
		data = new data_type[dim.segment*dim.view*dim.ring*dim.tangential];
		std::memset(data, 0, sizeof(data));
	}

	///\brief Free allocated memory
	~BinStorage()
	{
		delete[] data;
	}

	///\brief Return dimensions of or storage
	BinIndex GetDimensions() const { return dim; }

	data_type*	GetSegmentData(unsigned seg) const { return data+seg*(dim.tangential*dim.ring*dim.view); }
	data_type operator[](BinIndex i) const { return data[calcDataOffset(i)]; }
	data_type& operator[](BinIndex i) { return data[calcDataOffset(i)]; }

private:
	data_type*	data;
	BinIndex dim;

	// copy ctor and assignment not implemented, for copy protection
	BinStorage(const BinStorage&);
	BinStorage& operator=(const BinStorage&);

	///\brief Helper function for index operators
	///
	/// This function calculates the data offset for a given BinIndex
	std::size_t calcDataOffset(BinIndex i) const
	{
		return i.tangential + dim.tangential
			*(i.ring + dim.ring
			*(i.view + dim.view
			*(i.segment)));
	}
};

Sinogram::Sinogram(const Geometry& geometry, unsigned tangentialDimension, bool verbose)
:	isVerbose(verbose)
{
	using namespace std;

	BinIndex dimensions;
	dimensions.segment = 2*maxRingDifference+1;
	dimensions.view = ClearPET::numPixelsTotalTangential/2;
	dimensions.ring = ClearPET::numRings;
	dimensions.tangential = tangentialDimension;

	radius = geometry.GetRadius()+geometry.GetCrystalDepth()/2.0;
	ringSpacing = geometry.GetRingSpacing();
	viewSpacing = pi / dimensions.view;
	binSize = ringSpacing/2;

	bins = new BinStorage(dimensions);

	maxAxialNumber.resize(dimensions.segment);
	for (int i = -maxRingDifference; i <= maxRingDifference; ++i)
		maxAxialNumber[i+maxRingDifference] = ClearPET::numRings-abs(i);
}

Sinogram::~Sinogram()
{
	delete bins;
}

#if _MSC_VER
///\brief round a number to the nearest integer
inline int round(double x)
{
   return int(x > 0.0 ? x + 0.5 : x - 0.5);
}
#endif

///\brief calculate the square of a number
inline double sqr(double x)
{
	return x*x;
}

///\brief Find intersection of a line with a cylinder
///
/// The line is given by two points in cartesian coordinates. The cylinder
/// has the given radius, while its axis is the z axis of the coordinate
/// system.
///
///\return True if an intersection was found. If so, the function returns the
/// two intersection points in the variables passed into the function. If
/// there is exactly one intersection point, the two points returned are
/// equal.
bool FindIntersectionWithCylinder(CartCoord& p1, CartCoord& p2, double radius)
{
	// We calculate this:
	// The line is parameterized as g = p1 + t*d, d = p2-p1.
	// The intersection points must fullfill the circle equation
	//  g.x^1 + g.y^2 = R^2
	// =>  (p1.x + t*d.x)^2 + (p1.y + t*d.y)^2 = R^2
	// => t^2 * (d.x^2+d.y^2) + t * 2*(p1.x*d.x+p1.y*d.y) + p1.x^2+p1.y^2-R^2 = 0
	//  or a*t^2 + 2*b*t + c = 0  (note: 2*b!)
	// This square equation is solved by
	//  t1 = (-b + sqrt(b^2-a*c)) / a
	//  t2 = (-b - sqrt(b^2-a*c)) / a
	// The term in sqrt can further be simplified to:
	//  R^2*(d.x^2+d.y^2) - (p1.y*d.x-p1.x*d.y)^2

	CartCoord d = p2 - p1;
	double a = sqr(d.x) + sqr(d.y);
	double b = p1.x*d.x + p1.y*d.y;
	double term = sqr(radius)*a - sqr(p1.y*d.x-p1.x*d.y);
	if (term < 0.0)
		return false;

	double t1 = (-b + sqrt(term)) / a;
	double t2 = (-b - sqrt(term)) / a;
	p2 = p1 + t2*d; // calculate p2 first, because p1 will change below!
	p1 = p1 + t1*d;
	return true;
}

///\brief Add a LOR to the sinogram bins
///
/// This function calculates the intersection of the LOR with a hypothetical
/// acceptance cylinder, and calculates the 4 corrdinates identifying the
/// LOR in the sinogram coordinate system.
///
/// The function does an arc correction. This means that the size of bins in
/// the tangential direction is constant, while the densitiy of LOR increases
/// when the tangential coordinate gets larger.
///
/// If the LOR does not intersect the cylinder, or if the sinogram coordiantes
/// are out of range, the function does nothing and returns false.
///
/// If isVerbose is true, the function dumps the bin coordinates on stdout.
/// Use this with care, as it produces very large outputs and slows down
/// the code a lot!
///
///\return true, if the lor was counted, else false.
///
bool Sinogram::Add(LOR lor)
{
	using namespace std;

	// projection from cylindrical to cartesian coordinates:
#if 0 // debug code
cout.precision(3); cout.setf(ios::fixed, ios::floatfield); cout.width(7);
cout << "  Crystal Pos:   p1=("<<lor.p1.x<<','<<lor.p1.y<<','<<lor.p1.z<<") p2=("<<lor.p2.x<<','<<lor.p2.y<<','<<lor.p2.z<<")\n";
#endif
	if (!FindIntersectionWithCylinder(lor.p1, lor.p2, radius))
	{
		// No intersection with cylinder, discard event.
		///\todo Maybe we should stop the program with error if there is no
		/// intersection of LOR and acceptance cylinder, because this points
		/// to some serious error in the coinc sorting or the geometry.
		if (isVerbose)
			cout << "no intersection of LOR and acceptance cylinder!\n";
		return false;
	}
#if 0 // debug code
cout << "  Intersections: p1=("<<lor.p1.x<<','<<lor.p1.y<<','<<lor.p1.z<<") p2=("<<lor.p2.x<<','<<lor.p2.y<<','<<lor.p2.z<<")\n";
#endif

	// swap LOR end point to be sure that z1 <= z2
	if (lor.p1.z > lor.p2.z)
		swap(lor.p1, lor.p2);

	// The elements needed for bin correction are:
	//  - delta, the difference in z direction
	//  - z, the z-coordinate of the first intersection point
	//  - phi, the angle between x-axis and the projection of the LOR
	//    on the xy plane
	//  - s, the distance of the LOR to the axis of the cylinder. This is
	//    the same as the distance of the projection of the LOR on the xy plane
	//    to the origin of the coordinate system.
	// We do not need theta, the angle between the LOR and the xy plane.
	// Using theta would result in a sperical arc correction, not a
	// cylindrical one.
	CartCoord d = lor.p2 - lor.p1;
	double delta = -d.z;
	double phi = atan2(d.y, d.x);
	// phi shall be positive, in range [0,pi), excluding pi!
	if (phi < 0)
	{
		// This is effectively an exchange of the intersection points.
		phi += pi;
		delta *= -1;
	} else if (phi >= pi)
	{
		phi -= pi;
		delta *= -1;
	}
	// Distance from axis to LOR:
	// This formula can be explained like this: Take one intersection point,
	// and rotate it by -phi around the origin. The rotated LOR would then
	// be parallel to the x axis (by the definition of phi). The distance to
	// the origin is then equall to the y coordinate of the rotated point.
	// The formula just calculates the y coordinate of p1 rotated by -phi.
	double s = lor.p1.y*cos(phi) - lor.p1.x*sin(phi);

	BinIndex dims = bins->GetDimensions();

#if 0 // debug code
cout << "d.z=" << d.z << " ringSpacing=" << ringSpacing << " delta=" << delta/ringSpacing << " radius=" << radius << "\n";
#endif

	// calculate index coordinates
	BinIndex index;
	int seg = round(delta/ringSpacing);
	int view = round(phi/viewSpacing);
	int ring = round(lor.p1.z/ringSpacing);
	int tang = round(s/binSize);
	if (view == static_cast<int>(dims.view))
	{
		// by float rounding, we might get a view value equal to the view dimension.
		// To correct that, we switch back to view 0, and negate the other two
		// coordinates which depend on that angle.
		view = 0;
		seg = -seg;
		tang = -tang;
	}

	if (isVerbose)
	{
		cout << "Seg " << setw(4) << seg;
		cout << " view " << setw(4) << view;
		cout << " ax_pos " << setw(4) << ring;
		cout << " tang_pos " << setw(4) << tang;
//		cout << " time " << setw(8) << 0;
	}

	// make index range positive:
	index.segment = static_cast<unsigned>(seg + dims.segment/2);
	index.view = static_cast<unsigned>(view);
	index.ring = static_cast<unsigned>(ring);
	index.tangential = static_cast<unsigned>(tang + dims.tangential/2);

	// check if the bin index is valid
	if (index.segment >= dims.segment
	|| index.view >= dims.view
	|| index.ring >= maxAxialNumber[index.segment]
	|| index.tangential >= dims.tangential)
	{
		// index is out of range, we just give up
		if (isVerbose)
			cout << ": out of range\n";
//			cout << " ignored\n";
		return false;
	}

	// finally, increment the bin
	(*bins)[index]++;
	if (isVerbose)
		cout << ": accepted\n";
//		cout << " stored with incr 1 \n";
	return true;
}

///\brief Write the contents of a container, values separated by commas
///\note std::ostream_iterator is doing nearly the same thing, but adds
///  another separator after the last element. We don't like that, so we
///  must code it on our own.
template <typename C>
void DumpCommaSeparatedList(std::ostream& os, const C& container)
{
	typename C::const_iterator i = container.begin();
	typename C::const_iterator end = container.end();

	os << "{ ";
	if (i != end)
		// write the first element without comma
		os << *i++;
	// write the rest with comma before the element
	while (i != end)
		os << ',' << *i++;
	os << "}\n";
}

///\brief Check at runtime if the system is littleendian
bool IsSystemLittleEndian()
{
	int i = 1;
	return 0 != *reinterpret_cast<char*>(&i);
}

///\brief Write the header file of the sinogram data
///
/// Currently, the output of this code is quite identical to the contents of the
/// original ClearPET seg_31.hs template file. We have not changed anything,
/// because we do not know enough about the meaning of all the entries.
///
///\param outputName	The name of the file
///\throw std::runtime_error if the file cannot be created or written.
///
void Sinogram::SaveHeaderFile(const std::string& outputName, const Geometry& geometry)
{
	using namespace std;

	BinIndex dims = bins->GetDimensions();
	std::vector<int> ringDiffs(dims.segment);
	for (unsigned i = 0; i < dims.segment; ++i)
		ringDiffs[i] = i - maxRingDifference;

	ofstream file(outputName.c_str(), ios::out);
	if (!file)
		throw std::runtime_error("Error opening sinogram header file for output");

	file << "!INTERFILE :=\n";
	file << "name of data file := " << outputName << ".s\n";
	file << "originating system  := ClearPET\n";
	file << "!GENERAL DATA :=\n";
	file << "!GENERAL IMAGE DATA :=\n";
	file << "!type of data := PET\n";
	file << "imagedata byte order := "
		<< (IsSystemLittleEndian() ? "LITTLE" : "BIG") << "ENDIAN\n";
	file << "!PET STUDY (General) :=\n";
	file << "!PET data type := Emission\n";
	file << "applied corrections := {arc correction}\n";
	file << "!number format := float\n";
	file << "!number of bytes per pixel := 4\n";
	file << "number of dimensions := 4\n";
	file << "matrix axis label [4] := segment\n";
	file << "!matrix size [4] := " << dims.segment << "\n";
	file << "matrix axis label [3] := view\n";
	file << "!matrix size [3] := " << dims.view << "\n";
	file << "matrix axis label [2] := axial coordinate\n";
	file << "!matrix size [2] := ";
	DumpCommaSeparatedList(file, maxAxialNumber);
	file << "matrix axis label [1] := tangential coordinate\n";
	file << "!matrix size [1] := " << dims.tangential << "\n";
	file << "minimum ring difference per segment := ";
	DumpCommaSeparatedList(file, ringDiffs);
	file << "maximum ring difference per segment := ";
	DumpCommaSeparatedList(file, ringDiffs);
	file << "\n";
	file << "Scanner parameters:=\n";
	file << "Scanner type := ClearPET\n";
	file << "Number of rings := " << ClearPET::numRings << "\n";
	file << "Number of detectors per ring := " << ClearPET::numPixelsTotalTangential << "\n";
	file << "Inner ring diameter (cm) := " << geometry.GetRadius()*2.0/10.0 << "\n";
	file << "Average depth of interaction (cm) := " << geometry.GetCrystalDepth()/2.0/10.0 << "\n";
	file << "Distance between rings (cm) := " << ringSpacing/10.0 << "\n";
	file << "Default bin size (cm) := " << binSize/10.0 << "\n";
	file << "View offset (degrees) := 0\n";
	file << "Maximum number of non-arc-corrected bins := 0\n";
	file << "Default number of arc-corrected bins := 0\n";
	file << "Number of blocks per bucket in transaxial direction := 1\n";
	file << "Number of blocks per bucket in axial direction := " << ClearPET::numRings/ClearPET::numPixelsAxial << "\n";
	file << "Number of crystals per block in axial direction := " << ClearPET::numPixelsAxial << "\n";
	file << "Number of crystals per block in transaxial direction := " << ClearPET::numPixelsTangential<< "\n";
	file << "Number of detector layers := " << ClearPET::numLayers << "\n";
	file << "Number of crystals per singles unit in axial direction := 1\n";
	file << "Number of crystals per singles unit in transaxial direction := 1\n";
	file << "end scanner parameters:=\n";
	file << "\n";
	file << "effective central bin size (cm) := " << binSize/10.0 << "\n";
	file << "image scaling factor[1] := 1\n";
	file << "data offset in bytes[1] := 0\n";
	file << "number of time frames := 1\n";
	file << "!END OF INTERFILE :=\n";

	file.close();

	if (!file)
		throw std::runtime_error("Error writing sinogram header file");
}

///\brief Write the binary data file of the sinogram data
///
///\param outputName	The name of the file
///\throw std::runtime_error if the file cannot be created or written.
///
void Sinogram::SaveDataFile(const std::string& outputName)
{
	using namespace std;

	ofstream file(outputName.c_str(), ios::out|ios::binary);
	if (!file)
		throw std::runtime_error("Error opening sinogram data file for output");

	// Note: in this function, we have to pay the price for the simple
	// memory layout of the sinogram bin storage. In our memory, we have
	// "holes" which contain no data, and which we must not write to file,
	// Therefore, we can only write data for a whole ring in one operation.
	// 
	const BinIndex dim = bins->GetDimensions();
	const BinStorage::data_type* data = bins->GetSegmentData(0);
	for (unsigned seg = 0; seg < dim.segment; ++seg)
	{
		for (unsigned view = 0; view < dim.view; ++view)
		{
			size_t ringSize = maxAxialNumber[seg]*dim.tangential;
			file.write(reinterpret_cast<const char*>(data), ringSize*sizeof(BinStorage::data_type));
			if (!file)
				throw std::runtime_error("Error writing sinogram data file");
			data += dim.ring*dim.tangential;
		}
	}
}

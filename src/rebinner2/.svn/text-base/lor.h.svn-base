/**
 *\file		lor.h
 *\brief	Declarations related to LORs (line of responses)
 *\author	Axel Wagner
 */
#ifndef lor_h_included
#define lor_h_included

#include <cmath>

///\brief 3D Coordinates in cartesian system
struct CartCoord
{
	CartCoord(): x(0), y(0), z(0) {}
	CartCoord(double ix, double iy, double iz): x(ix), y(iy), z(iz) {}

	double	x;	///< in mm
	double	y;	///< in mm
	double	z;	///< axial coordinate, in mm

	CartCoord& operator*= (double s)
	{
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	friend CartCoord operator+ (const CartCoord& lhs, const CartCoord& rhs)
	{
		return CartCoord(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
	}

	friend CartCoord operator- (const CartCoord& lhs, const CartCoord& rhs)
	{
		return CartCoord(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z);
	}

	friend CartCoord operator* (const CartCoord& lhs, double s)
	{
		return CartCoord(lhs.x*s, lhs.y*s, lhs.z*s);
	}
	friend CartCoord operator* (double s, const CartCoord& rhs)
	{
		return CartCoord(s*rhs.x, s*rhs.y, s*rhs.z);
	}

};

///\brief Line of response
struct LOR
{
	CartCoord	p1;	///< position of first event
	CartCoord	p2; ///< position of second event
};

#endif // lor_h_included

//
// $Id: round.inl,v 1.7 2004/12/07 12:08:54 kris Exp $
//
/*!
  \file
  \ingroup buildblock
  
  \brief Implementation of the round functions
    
  \author Kris Thielemans
  \author Charalampos Tsoumpas
      
  $Date: 2004/12/07 12:08:54 $	
  $Revision: 1.7 $	
*/
/*
    Copyright (C) 2000- $Date: 2004/12/07 12:08:54 $, Hammersmith Imanet Ltd
    This software is distributed under the terms of the GNU Lesser General 
    Public Licence (LGPL).
    See STIR/LICENSE.txt for details
*/

START_NAMESPACE_STIR

template <typename integerT>
inline void
round_to(integerT& result, const float x)
{
  if (x>=0)
    result = static_cast<integerT>(x+0.5F);
  else
    result = -static_cast<integerT>(-x+0.5F);
}


template <typename integerT>
inline void
round_to(integerT& result, const double x)
{
  if (x>=0)
    result = static_cast<integerT>(x+0.5);
  else
    result = -static_cast<integerT>(-x+0.5);
}

/* could be implemented in terms of the above */
int round(const float x)
{
  if (x>=0)
    return static_cast<int>(x+0.5F);
  else
    return -static_cast<int>(-x+0.5F);
}

int round(const double x)
{
  if (x>=0)
    return static_cast<int>(x+0.5);
  else
    return -static_cast<int>(-x+0.5);
}

template <int num_dimensions, class elemT>
BasicCoordinate<num_dimensions,int>
round(const BasicCoordinate<num_dimensions,elemT>& x)
{
	BasicCoordinate<num_dimensions,int> rnd_x;
	for(int i=1;i<=num_dimensions;++i)
		rnd_x[i]=round(x[i]);	
	return rnd_x;  
}

template <int num_dimensions, class integerT, class elemT>
inline void 
round_to(BasicCoordinate<num_dimensions,integerT>& result,
	const BasicCoordinate<num_dimensions,elemT>& x)
{
	for(int i=1;i<=num_dimensions;++i)
		round_to(result[i], x[i]);	
}

END_NAMESPACE_STIR


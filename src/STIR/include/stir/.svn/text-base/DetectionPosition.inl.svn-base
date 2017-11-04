//
// $Id: DetectionPosition.inl,v 1.2 2004/03/19 10:10:10 kris Exp $
//

/*!
  \file
  \ingroup projdata
  \brief Implementation of inline methods of class DetectionPosition
  \author Kris Thielemans
  $Date: 2004/03/19 10:10:10 $
  $Revision: 1.2 $
*/
/*
    Copyright (C) 2002- $Date: 2004/03/19 10:10:10 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/

START_NAMESPACE_STIR
template <typename coordT>
DetectionPosition<coordT>::
DetectionPosition(const coordT tangential_coord,
  	                   const coordT axial_coord, 
			   const coordT radial_coord)
  : tangential(tangential_coord),
    axial(axial_coord),
    radial(radial_coord)
{}

template <typename coordT>
coordT
DetectionPosition<coordT>::
tangential_coord()  const
{ return tangential; }

template <typename coordT>
coordT
DetectionPosition<coordT>::
axial_coord()const
{ return axial;}

template <typename coordT>
coordT
DetectionPosition<coordT>::
radial_coord()const
{ return radial;}

template <typename coordT>
coordT&
DetectionPosition<coordT>::
tangential_coord()
{ return tangential;}

template <typename coordT>
coordT&
DetectionPosition<coordT>::
axial_coord()
{ return axial;}

template <typename coordT>
coordT&
DetectionPosition<coordT>::
radial_coord()
{ return radial;} 

    //! comparison operators
template <typename coordT>
bool
DetectionPosition<coordT>::
operator==(const DetectionPosition& d) const
{
  return 
    tangential == d.tangential &&
    axial == d.axial &&
    radial == d.radial;
}

template <typename coordT>
bool
DetectionPosition<coordT>::
operator!=(const DetectionPosition& d) const
{ return !(*this==d); }

END_NAMESPACE_STIR


//
// $Id: DataSymmetriesForDensels.h,v 1.4 2004/07/12 17:56:51 kris Exp $
//
/*!
  \file
  \ingroup symmetries

  \brief Declaration of class DataSymmetriesForDensels

  \author Kris Thielemans
 
  $Date: 2004/07/12 17:56:51 $
  $Revision: 1.4 $
*/
/*
    Copyright (C) 2001- $Date: 2004/07/12 17:56:51 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/
#ifndef __stir_recon_buildblock_DataSymmetriesForDensels_H__
#define __stir_recon_buildblock_DataSymmetriesForDensels_H__

#include "stir/DataSymmetriesForViewSegmentNumbers.h"
#include "stir/Densel.h"
#include <vector>
#include <memory>

#include "stir/Coordinate2D.h"

#ifndef STIR_NO_NAMESPACES
using std::vector;
#ifndef STIR_NO_AUTO_PTR
using std::auto_ptr;
#endif
#endif


START_NAMESPACE_STIR

//class Densel;
class SymmetryOperation;


#if 0
class DenselIndexRange;
#endif



/*!
  \ingroup symmetries
  \brief A class for encoding/finding symmetries common to the geometry
  of the projection data and the discretised density. 

  This class is mainly (only?) useful for ProjMatrixByDensel classes and their
  'users'. Together with SymmetryOperation, it provides the basic 
  way to be able to write generic code without knowing which 
  particular symmetries the data have.

*/
class DataSymmetriesForDensels 
{
public:
  DataSymmetriesForDensels();

  virtual ~DataSymmetriesForDensels() {};

  virtual 
    DataSymmetriesForDensels 
    * clone() const = 0;

#if 0
  TODO!
  //! returns the range of the indices for basic Densels
  virtual DenselIndexRange
    get_basic_densel_index_range() const = 0;
#endif

  //! fills in a vector with all the Densels that are related to 'b' (including itself)
  /*! 
      \warning \c b has to be a 'basic' Densel
  */
  // next return value could be a RelatedDensels ???
  virtual  void
    get_related_densels(vector<Densel>&, const Densel& b) const = 0;

#if 0
  //! fills in a vector with all the Densels (within the range) that are related to 'b'
  /*! \warning \c b has to be a 'basic' Densel
  */
  virtual void
    get_related_densels(vector<Densel>&, const Densel& b,
                      const int min_axial_pos_num, const int max_axial_pos_num) const;
#endif

  //! returns the number of Densels related to 'b'
  virtual int
    num_related_densels(const Densel& b) const;

  /*! \brief given an arbitrary Densel 'b', find the basic Densel
  
  sets 'b' to the corresponding 'basic' Densel and returns the symmetry 
  transformation from 'basic' to 'b'.
  */
  virtual auto_ptr<SymmetryOperation>
    find_symmetry_operation_from_basic_densel(Densel&) const = 0;

  /*! \brief given an arbitrary Densel 'b', find the basic Densel
  
  sets 'b' to the corresponding 'basic' Densel and returns true if
  'b' is changed (i.e. it was NOT a basic Densel).
  */
  virtual bool
    find_basic_densel(Densel& b) const;


};

END_NAMESPACE_STIR

//#include "stir/recon_buildblock/DataSymmetriesForDensels.inl"


#endif


//
// $Id: TrivialBinNormalisation.h,v 1.5 2004/03/23 11:59:53 kris Exp $
//
/*!
  \file
  \ingroup normalisation

  \brief Declaration of class TrivialBinNormalisation

  \author Kris Thielemans
  $Date: 2004/03/23 11:59:53 $
  $Revision: 1.5 $
*/
/*
    Copyright (C) 2000- $Date: 2004/03/23 11:59:53 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/

#ifndef __stir_recon_buildblock_TrivialBinNormalisation_H__
#define __stir_recon_buildblock_TrivialBinNormalisation_H__

#include "stir/recon_buildblock/BinNormalisation.h"
#include "stir/RegisteredParsingObject.h"

START_NAMESPACE_STIR

/*!
  \ingroup normalisation
  \brief Trivial class which does not do any normalisation whatsoever.
  \todo Make sure that the keyword value \c None corresponds to this class.

*/
class TrivialBinNormalisation : 
   public RegisteredParsingObject<TrivialBinNormalisation, BinNormalisation>
{
public:
  //! Name which will be used when parsing a BinNormalisation object
  static const char * const registered_name; 

  virtual void apply(RelatedViewgrams<float>&,const double start_time, const double end_time) const {}
  virtual void undo(RelatedViewgrams<float>&,const double start_time, const double end_time) const {}
  
  virtual float get_bin_efficiency(const Bin& bin,const double start_time, const double end_time) const { return 1;}
  

private:
  virtual void set_defaults() {}
  virtual void initialise_keymap() {}
  
};

END_NAMESPACE_STIR

#endif

//
// $Id: GeneralisedPrior.cxx,v 1.3 2004/06/30 17:10:00 kris Exp $
//
/*!
  \file
  \ingroup priors
  \brief  implementation of the GeneralisedPrior
    
  \author Kris Thielemans
  \author Sanida Mustafovic      
  $Date: 2004/06/30 17:10:00 $  
  $Revision: 1.3 $
*/
/*
    Copyright (C) 2002- $Date: 2004/06/30 17:10:00 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/

#include "stir/recon_buildblock/GeneralisedPrior.h"

START_NAMESPACE_STIR


template <typename elemT>
void 
GeneralisedPrior<elemT>::initialise_keymap()
{
  parser.add_key("penalisation factor", &penalisation_factor); 
}


template <typename elemT>
void
GeneralisedPrior<elemT>::set_defaults()
{
  penalisation_factor = 0;  
}

#  ifdef _MSC_VER
// prevent warning message on instantiation of abstract class 
#  pragma warning(disable:4661)
#  endif

template class GeneralisedPrior<float>;

END_NAMESPACE_STIR


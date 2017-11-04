//
// $Id: ProjectorByBinPairUsingProjMatrixByBin.h,v 1.3 2004/03/23 11:59:53 kris Exp $
//
/*!
  \file
  \ingroup projection

  \brief Declares class ProjectorByBinPairUsingProjMatrixByBin

  \author Kris Thielemans

  $Date: 2004/03/23 11:59:53 $
  $Revision: 1.3 $
*/
/*
    Copyright (C) 2000- $Date: 2004/03/23 11:59:53 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/
#ifndef __stir_recon_buildblock_ProjectorByBinPairUsingProjMatrixByBin_h_
#define __stir_recon_buildblock_ProjectorByBinPairUsingProjMatrixByBin_h_

#include "stir/RegisteredParsingObject.h"
#include "stir/recon_buildblock/ProjectorByBinPair.h"
#include "stir/recon_buildblock/ProjMatrixByBin.h"

START_NAMESPACE_STIR


/*!
  \ingroup projection
  \brief A projector pair based on a single matrix
*/
class ProjectorByBinPairUsingProjMatrixByBin : 
  public RegisteredParsingObject<ProjectorByBinPairUsingProjMatrixByBin,
                                 ProjectorByBinPair> 
{ 
public:
  //! Name which will be used when parsing a ProjectorByBinPair object
  static const char * const registered_name; 

  //! Default constructor 
  ProjectorByBinPairUsingProjMatrixByBin();

  //! Constructor that sets the pair
  ProjectorByBinPairUsingProjMatrixByBin(const shared_ptr<ProjMatrixByBin>& proj_matrix_ptr);

  //! Stores all necessary geometric info
  virtual void set_up(		 
    const shared_ptr<ProjDataInfo>& proj_data_info_ptr,
    const shared_ptr<DiscretisedDensity<3,float> >& density_info_ptr // TODO should be Info only
    );


  ProjMatrixByBin const * 
    get_proj_matrix_ptr() const;

private:

  shared_ptr<ProjMatrixByBin> proj_matrix_ptr;
  void set_defaults();
  void initialise_keymap();
  bool post_processing();
};

END_NAMESPACE_STIR


#endif // __stir_recon_buildblock_ProjectorByBinPairUsingProjMatrixByBin_h_

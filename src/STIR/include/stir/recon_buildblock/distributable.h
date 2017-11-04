//
// $Id: distributable.h,v 1.5 2004/03/23 11:59:53 kris Exp $
//

#ifndef __stir_recon_buildblock_DISTRIBUTABLE_H__
#define __stir_recon_buildblock_DISTRIBUTABLE_H__

/*!
  \file
  \ingroup distributable

  \brief Declaration of the main function that performs parallel processing

  \author Alexey Zverovich
  \author Kris Thielemans
  \author Matthew Jacobson
  \author PARAPET project

   $Date: 2004/03/23 11:59:53 $
   $Revision: 1.5 $
*/
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- $Date: 2004/03/23 11:59:53 $, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/
#include "stir/common.h"

START_NAMESPACE_STIR

template <typename T> class shared_ptr;
template <typename elemT> class RelatedViewgrams;
template <int num_dimensions, typename elemT> class DiscretisedDensity;
class ProjData;
class ProjDataInfo;
class DataSymmetriesForViewSegmentNumbers;
class ForwardProjectorByBin;
class BackProjectorByBin;

//!@{
//! \ingroup distributable

extern bool RPC_slave_sens_zero_seg0_end_planes; // = false;

extern shared_ptr<ForwardProjectorByBin> forward_projector_ptr;
extern shared_ptr<BackProjectorByBin> back_projector_ptr;
extern shared_ptr<DataSymmetriesForViewSegmentNumbers> symmetries_ptr;

//! Sets projectors and the symmetries to be used by distributable_computation.
/*! \warning It is assumed (but not checked) that the projectors can handle RelatedViewgrams
    as constructed by \a current_symmetries_ptr.
 */
void set_projectors_and_symmetries(
       const shared_ptr<ForwardProjectorByBin>& current_forward_projector_ptr,
       const shared_ptr<BackProjectorByBin>& current_back_projector_ptr,
       const shared_ptr<DataSymmetriesForViewSegmentNumbers>& current_symmetries_ptr);

//! typedef for callback functions for distributable_computation()
/*! Pointers will be NULL when they are not to be used by the callback function.

    \a count and \a count2 are normally incremental counters that accumulate over the loop
    in distributable_computation().

    \warning The data in *measured_viewgrams_ptr are allowed to be overwritten, but the new data 
    will not be used. 
*/
typedef  void RPC_process_related_viewgrams_type (DiscretisedDensity<3,float>* output_image_ptr, 
                                             const DiscretisedDensity<3,float>* input_image_ptr, 
			   RelatedViewgrams<float>* measured_viewgrams_ptr,
			   int& count, int& count2, float* log_likelihood_ptr,
			   const RelatedViewgrams<float>* additive_binwise_correction_ptr);

/*!
  \brief This function essentially implements a loop over segments and all views in the current subset.
  
  Output is in output_image_ptr and in float_out_ptr (but only if they are not NULL).
  What the output is, is really determined by the call-back function
  RPC_process_related_viewgrams.

  A parallel version of this function exists which distributes the computation over the slaves.

  Subsets are currently defined on views. A particular \a subset_num contains all views
  which are symmetry related to 
  \code 
  proj_data_ptr->min_view_num()+subset_num + n*num_subsets
  \endcode
  for n=0,1,,.. \c and for which the above view_num is 'basic' (for some segment_num in the range).

  Symmetries are determined by using the 3rd argument to set_projectors_and_symmetries().

  \param output_image_ptr will store the output image if non-zero.
  \param input_image_ptr input when non-zero.
  \param proj_data_ptr input projection data
  \param read_from_proj_data if true, the \a measured_viewgrams_ptr argument of the call_back function 
         will be constructed using ProjData::get_related_viewgrams, otherwise 
         ProjData::get_empty_related_viewgrams is used.
  \param subset_num the number of the current subset (see above). Should be between 0 and num_subsets-1.
  \param num_subsets the number of subsets to consider. 1 will process all data.
  \param min_segment_num Minimum segment_num to process.
  \param max_segment_num Maximum segment_num to process.
  \param zero_seg0_end_planes if true, the end planes for segment_num=0 in measured_viewgrams_ptr
         (and additive_binwise_correction_ptr when applicable) will be set to 0.
  \param float_out_ptr a potential float output parameter for the call back function.
  \param additive_binwise_correction Additional input projection data (when the shared_ptr is not 0).
  \param RPC_process_related_viewgrams function that does the actual work.
  
  \warning There is NO check that the resulting subsets are balanced.

  \warning The function assumes that \a min_segment_num, \a max_segment_num are such that
  symmetries map this range onto itself (i.e. no segment_num is obtained outside the range). 
  This usually means that \a min_segment_num = -\a max_segment_num. This assumption is checked with 
  assert().

  \todo The subset-scheme should be moved somewhere else (a Subset class?).

 */
void distributable_computation(DiscretisedDensity<3,float>* output_image_ptr,
				    const DiscretisedDensity<3,float>* input_image_ptr,
				    const shared_ptr<ProjData>& proj_data_ptr,
                                    const bool read_from_proj_data,
				    int subset_num, int num_subsets,
				    int min_segment_num, int max_segment_num,
				    bool zero_seg0_end_planes,
				    float* float_out_ptr,
				    const shared_ptr<ProjData>& additive_binwise_correction,
                                    RPC_process_related_viewgrams_type * RPC_process_related_viewgrams);


//!@}

END_NAMESPACE_STIR

#endif // __stir_recon_buildblock_DISTRIBUTABLE_H__


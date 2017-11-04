//
// $Id: TrivialDataSymmetriesForBins.h,v 1.2 2004/03/23 11:59:53 kris Exp $
//
/*!

  \file
  \ingroup symmetries

  \brief Declaration of class TrivialDataSymmetriesForBins

  \author Kris Thielemans

  $Date: 2004/03/23 11:59:53 $
  $Revision: 1.2 $
*/
/*
    Copyright (C) 2003- $Date: 2004/03/23 11:59:53 $, Hammersmith Imanet
    See STIR/LICENSE.txt for details
*/
#ifndef __stir_recon_buildblock_TrivialDataSymmetriesForBins_H__
#define __stir_recon_buildblock_TrivialDataSymmetriesForBins_H__

#include "stir/recon_buildblock/DataSymmetriesForBins.h"

START_NAMESPACE_STIR


/*!
  \ingroup symmetries
  \brief A class derived from DataSymmetriesForBins that says that there are
  no symmetries at all.

*/
class TrivialDataSymmetriesForBins : public DataSymmetriesForBins
{
public:
  TrivialDataSymmetriesForBins(const shared_ptr<ProjDataInfo>& proj_data_info_ptr);

  virtual 
#ifndef STIR_NO_COVARIANT_RETURN_TYPES
    TrivialDataSymmetriesForBins 
#else
    DataSymmetriesForViewSegmentNumbers
#endif
    * clone() const;

  virtual void
    get_related_bins(vector<Bin>&, const Bin& b,
                      const int min_axial_pos_num, const int max_axial_pos_num,
                      const int min_tangential_pos_num, const int max_tangential_pos_num) const;

  virtual void
    get_related_bins_factorised(vector<AxTangPosNumbers>&, const Bin& b,
                                const int min_axial_pos_num, const int max_axial_pos_num,
                                const int min_tangential_pos_num, const int max_tangential_pos_num) const;

  virtual int
    num_related_bins(const Bin& b) const;

  virtual auto_ptr<SymmetryOperation>
    find_symmetry_operation_from_basic_bin(Bin&) const;

  virtual bool
    find_basic_bin(Bin& b) const;

  virtual bool
    is_basic(const Bin& v_s) const;

  virtual auto_ptr<SymmetryOperation>
    find_symmetry_operation_from_basic_view_segment_numbers(ViewSegmentNumbers&) const;

  virtual void
    get_related_view_segment_numbers(vector<ViewSegmentNumbers>&, const ViewSegmentNumbers&) const;

  virtual int
    num_related_view_segment_numbers(const ViewSegmentNumbers&) const;
  virtual bool
    find_basic_view_segment_numbers(ViewSegmentNumbers&) const;
};

END_NAMESPACE_STIR


#endif


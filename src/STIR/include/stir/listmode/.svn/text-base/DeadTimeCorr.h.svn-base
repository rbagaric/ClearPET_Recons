#ifndef __stir_listmode_DeadTimeCorr_H__
#define __stir_listmode_DeadTimeCorr_H__

#include "stir/ProjDataInfo.h"
#include "stir/ProjData.h"
#include "stir/ParsingObject.h"
#include "stir/TimeFrameDefinitions.h"

#include "stir/recon_buildblock/BinNormalisation.h"

START_NAMESPACE_STIR

class CListTime;

class DeadTimeCorr : public ParsingObject
{
public:

  //! Constructor taking a filename for a parameter file
  /*! Will attempt to open and parse the file. */
  DeadTimeCorr(const char * const par_filename);

  //! Default constructor
  /*! \warning leaves parameters ill-defined. Set them by parsing. */
  DeadTimeCorr();

  int num_segments;
  int num_axial_poss;
  int num_views;
  int num_tangential_poss;

  //! This function does the actual work
  virtual void process_data();

protected:

  
  //! will be called when a new time frame starts
  /*! The frame numbers start from 1. */
  virtual void start_new_time_frame(const unsigned int new_frame_num);

  //! will be called after a new timing event is found in the file
  virtual void process_new_time_event(const CListTime&);

  //! will be called to get the bin for a coincidence event
  /*! If bin.get_bin_value()<=0, the event will be ignored. Otherwise,
    the value will be used as a bin-normalisation factor 
    (on top of anything done by normalisation_ptr). 
    \todo Would need timing info or so for e.g. time dependent
    normalisation or angle info for a rotating scanner.*/
  // pqs besoin virtual void get_bin_from_event(Bin& bin, const CListEvent&) const;

  //! A function that should return the number of uncompressed bins in the current bin
  /*! \todo it is not compatiable with e.g. HiDAC doesn't belong here anyway
      (more ProjDataInfo?)
  */
  //int get_compression_count(const Bin& bin) const;
  
  //! Computes a post-normalisation factor (if any) for this bin
  /*! This uses get_compression_count() when do_pre_normalisation=true, or
      post_normalisation_ptr otherwise.
  */
  //void do_post_normalisation(Bin& bin) const;

  //! \name parsing functions
  //@{
  virtual void set_defaults();
  virtual void initialise_keymap();
  virtual bool post_processing();

  //@}

  //! \name parsing variables
  //{@
  string sinoE_filename;
  //string raw_filename;
  string output_filename_prefix;
  
  //! frame definitions
  /*! Will be read using TimeFrameDefinitions */
  string frame_definition_filename;

  shared_ptr<ProjData> sinoE_data_ptr;
  shared_ptr<ProjDataInfo> sinoE_data_info_ptr;

  //shared_ptr<ProjData> raw_data_ptr;
  //shared_ptr<ProjDataInfo> raw_data_info_ptr;

  int time_duration;
  double ITrate;
  double P2;
  double P3;
  double P4;
  double P5;
  double P6;
  double P7;
  double P8;

  TimeFrameDefinitions frame_defs;

  //! stores the time (in secs) recorded in the previous timing event
  double current_time;
  //! stores the current frame number
  /*! The frame numbers start from 1. */
  unsigned int current_frame_num;

  /*! \brief variable that will be set according to if we are using 
    time frames or num_events_to_store
  */
  bool do_time_frame;
  //! A variable that will be set to 1,0 or -1, according to store_prompts and store_delayeds
  //int delayed_increment;

};

END_NAMESPACE_STIR

#endif

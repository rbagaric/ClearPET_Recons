#ifndef __stir_listmode_SubtractSino_H__
#define __stir_listmode_SubtractSino_H__

#include "stir/ProjDataInfo.h"
#include "stir/ProjData.h"
#include "stir/ParsingObject.h"
#include "stir/TimeFrameDefinitions.h"

#include "stir/recon_buildblock/BinNormalisation.h"

START_NAMESPACE_STIR

class CListTime;

class SubtractSino : public ParsingObject
{
public:

  SubtractSino(const char * const par_filename);
  SubtractSino();

  int num_segments;
  int num_axial_poss;
  int num_views;
  int num_tangential_poss;

  virtual void process_data();

protected:

  virtual void start_new_time_frame(const unsigned int new_frame_num);

  virtual void process_new_time_event(const CListTime&);

  virtual void set_defaults();
  virtual void initialise_keymap();
  virtual bool post_processing();

  string sino1_filename;
  string sino2_filename;
  string output_filename_prefix;
  
  string frame_definition_filename;

  shared_ptr<ProjData> sino1_data_ptr;
  shared_ptr<ProjDataInfo> sino1_data_info_ptr;

  shared_ptr<ProjData> sino2_data_ptr;
  shared_ptr<ProjDataInfo> sino2_data_info_ptr;

  TimeFrameDefinitions frame_defs;

  double current_time;
  unsigned int current_frame_num;

  bool do_time_frame;

};

END_NAMESPACE_STIR

#endif

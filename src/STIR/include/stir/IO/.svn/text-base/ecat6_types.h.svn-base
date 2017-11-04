//
// $Id: ecat6_types.h,v 1.9 2003/05/19 06:43:20 kris Exp $ 
//

/*! 
  \file
  \ingroup ECAT
  \brief ECAT 6 CTI matrix parameters
  \author Larry Byars
  \author PARAPET project
  $Revision: 1.9 $
  $Date: 2003/05/19 06:43:20 $
  
  Enumerations of the data type and format.
  
  Structures: <BR>
  - scanner parameters <BR>
  - matrix blocks and parameters <BR>
  - main header, scan and image subheaders
*/
/*
  Copyright (C) CTI PET Inc.
  Copyright (C) 2000 PARAPET partners
  See STIR/LICENSE.txt for details
  */
#ifndef __stir_IO_ecat6_types_h
#define __stir_IO_ecat6_types_h

// include this for namespace macros
#include "stir/IO/stir_ecat_common.h"


START_NAMESPACE_STIR
START_NAMESPACE_ECAT
START_NAMESPACE_ECAT6

const int MatBLKSIZE =          512;
const int MatFirstDirBlk =       2;

// MatFileType
typedef enum {
    matScanFile = 1,   // sinogram data
    matImageFile = 2,  // image file   
    matAttenFile = 3,  // attenuation correction file
    matNormFile = 4    // normalization file
} MatFileType;
  


//#define matScanData   matI2Data
//#define matImageData  matI2Data

typedef short word;
typedef unsigned char byte;

/*!
  \struct ScanInfoRec
  \brief ECAT 6 CTI scanner parameters
  \ingroup ECAT
  \param nprojs      number of projections
  \param nviews      number of views
  \param nblks       number of blocks (planes)
  \param strtblk     first block
  \param data_type   type of data (float, short, ...)
*/
typedef struct ScanInfoRec {
    int nprojs,
        nviews,
        nblks,
        strtblk;
    word data_type;
} ScanInfoRec;

/*!
  \struct MatDir
  \brief ECAT 6 CTI matrix entry parameters
  \ingroup ECAT
  \param matnum    coded number
  \param strtblk   first block in the matrix
  \param endblk    last block in the matrix
*/
typedef struct MatDir { 
    long matnum,
        strtblk,
        endblk,
        matstat;
} MatDir;

/*!
  \struct Matval
  \brief ECAT 6 CTI matrix general parameters
  \ingroup ECAT
  \param frame   number of frame
  \param plane   number of plane
  \param gate    number of gate
  \param data    number of data 
  \param bed     number of bed
*/
typedef struct Matval {
    long frame,
        plane,
        gate,
        data,
        bed;
} Matval;

/*!
  \brief ECAT 6 CTI main header parameters
  \ingroup ECAT
  
  \param num_frames        number of frames
  \param num_planes        number of planes
  \param num_gates         number of gates
  \param num_bed_pos       number of bed positions
  \param file_type         scan or image file type
  \param data_type         data type (byte, I2, I4, Sun short, Sun long, ...)
  \param system_type       camera identification
  \param axial_fov         FOV axial dimension [cm]
  \param transaxial_fov    FOV radius [cm]
  \param plane_separation  spacing between ring [cm]
  \param gantry_tilt       intrinsic tilt
  \param original_file_name acquisition file name
*/
typedef struct cti_main_header {
    float isotope_halflife,
        gantry_tilt,
        gantry_rotation,
        bed_elevation,
        axial_fov,
        transaxial_fov,
        calibration_factor,
        init_bed_position,
        plane_separation,
        collimator,
        bed_offset[15];
    word num_planes,
        num_frames,
        num_gates,
        num_bed_pos,
        sw_version,
        data_type,
        system_type,
        file_type,
        scan_start_day,
        scan_start_month,
        scan_start_year,
        scan_start_hour,
        scan_start_minute,
        scan_start_second,
        rot_source_speed,
        wobble_speed,
        transm_source_type,
        transaxial_samp_mode,
        coin_samp_mode,
        axial_samp_mode,
        calibration_units,
        compression_code,
        acquisition_type,
        bed_type,
        septa_type,
        lwr_sctr_thres,
        lwr_true_thres,
        upr_true_thres;
    char original_file_name[20],
        node_id[10],
        isotope_code[8],
        radiopharmaceutical[32],
        study_name[12],
        patient_id[16],
        patient_name[32],
        patient_age[10],
        patient_height[10],
        patient_weight[10],
        physician_name[32],
        operator_name[32],
        study_description[32],
        facility_name[20],
        user_process_code[10],
        patient_sex,
        patient_dexterity;
}
ECAT6_Main_header;

/*!
  \struct cti_scan_subheader
  \brief ECAT 6 CTI scan subheader parameters
  \ingroup ECAT
  
  \param dimension_1     bins dimension, number of elements in a view
  \param dimension_2     views dimension
  \param scale_factor    scale factor (by sinogram)
  \param data_type       data type (byte, I2, I4, Sun short, Sun long, ...)
  \param sample_distance bin size
  \param loss_correction_fctr loss correction factor
  \param scan_min        minimum value for the segment
  \param scan_max        maximum value for the segment
*/
typedef struct cti_scan_subheader {
    float sample_distance,
        isotope_halflife,
        scale_factor,
        loss_correction_fctr,
        tot_avg_cor,
        tot_avg_uncor,
        cor_singles[16],
        uncor_singles[16];
    long gate_duration,
        r_wave_offset,
        prompts,
        delayed,
        multiples,
        net_trues,
        total_coin_rate,
        frame_start_time,
        frame_duration;
    word data_type,
        dimension_1,
        dimension_2,
        smoothing,
        processing_code,
        frame_duration_sec,
        scan_min,
        scan_max;
}
Scan_subheader;

/*!
  \struct cti_image_subheader
  \brief ECAT 6 CTI image subheader parameters
  \ingroup ECAT
  
  \param dimension_1     x dimension
  \param dimension_2     y dimension
  \param num_dimensions  number of dimensions
  \param slice_width     separation between planes
  \param pixel_size      size of pixels
  \param quant_scale     scale factor (by plane)
  \param data_type       data type (byte, I2, I4, Sun short, Sun long, ...)
  \param recon_scale     image zoom from reconstruction
  \param decay_corr_fctr decay correction factor
  \param loss_corr_fctr  loss correction factor
  \param image_min       minimum value for the plane
  \param image_max       maximum value for the plane
*/
typedef struct cti_image_subheader {
    float x_origin,
        y_origin,
        recon_scale,                    // Image ZOOM from reconstruction
        quant_scale,                    // Scale Factor
        pixel_size,
        slice_width,
        image_rotation,
        plane_eff_corr_fctr,
        decay_corr_fctr,
        loss_corr_fctr,
        ecat_calibration_fctr,
        well_counter_cal_fctr,
        filter_params[6];
    long frame_duration,
        frame_start_time,
        recon_duration,
        scan_matrix_num,
        norm_matrix_num,
        atten_cor_matrix_num;
    word data_type,
        num_dimensions,
        dimension_1,
        dimension_2,
        image_min,
        image_max,
        slice_location,
        recon_start_hour,
        recon_start_minute,
        recon_start_sec,
        filter_code,
        processing_code,
        quant_units,
        recon_start_day,
        recon_start_month,
        recon_start_year;
    char annotation[40];
}
Image_subheader;

//  Norm_subheader
typedef struct cti_norm_subheader {
    float scale_factor,
        fov_source_width;
    word data_type,
        dimension_1,
        dimension_2,
        norm_hour,
        norm_minute,
        norm_second,
        norm_day,
        norm_month,
        norm_year;
}
Norm_subheader;

//  Attn_subheader
typedef struct cti_attn_subheader {
    float scale_factor,
        x_origin,
        y_origin,
        x_radius,
        y_radius,
        tilt_angle,
        attenuation_coeff,
        sample_distance;
    word            data_type,
        attenuation_type,
        dimension_1,
        dimension_2;
}
Attn_subheader;
  
#if 0
//  dsr
typedef struct analyze_db {
    struct header_key {
        long sizeof_hdr;
	char data_type [10];
	char db_name [18];
	long extents;
	short session_error;
	char regular;
	char hkey_un0;
    } hk;
    
    struct image_dimension {
        short dim[8];
	short unused8;
	short unused9;
	short unused10;
	short unused11;
	short unused12;
	short unused13;
	short unused14;
	short datatype;
	short bitpix;
	short dim_un0;
	float pixdim[8];
	float funused8;
	float funused9;
	float funused10;
	float funused11;
	float funused12;
	float funused13;
	float compressed;
	float verified;
	long glmax, glmin;
    } id;
} dsr;
#endif
  
END_NAMESPACE_ECAT
END_NAMESPACE_ECAT6
END_NAMESPACE_STIR
#endif 

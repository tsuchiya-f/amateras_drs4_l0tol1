#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include "vdif_util.h"

// ----------------------------------------------------------------------
// 8-bit compression of low resolution spectrum data
//
// return value : number of over flow points in f_ave_rh and f_ave_lh.
// ----------------------------------------------------------------------
int compress_low_data(
  unsigned char *uc_out_rh,     // [out]  compressed RH spectrum data 
  unsigned char *uc_out_lh,     // [out]  compressed LH spectrum data 
  float *f_max,                 // [out]  maximum values in each RH, LF spectrum
  int n,                        // [in]   number of data points
  float *f_ave_rh,              // [in]   uncompressed RH spectrum data
  float *f_ave_lh,              // [in]   uncompressed LH spectrum data
  float bscale,                 // [in]   BSCALE
  float crval)                  // [in]   CRVAL
{

  int i;
  int i_ovf = 0;

  float f_data;
  unsigned char uc_data;

  f_max[0] = 0.0;
  f_max[1] = 0.0;

  for (i=0; i<n; i++)
  {
    f_data = 10.0*log10(f_ave_rh[i]);
    if (f_data > f_max[0]) f_max[0] = f_data;
    f_data = (f_data - crval) / bscale;
    uc_data = (unsigned char)f_data;
    if (isnan(f_data))
    {
      uc_data = 255;
    }
    else if (f_data < 0.0)
    {
      uc_data = 0;
    }
    else if (f_data > 254.0)
    {
      uc_data = 254;
      i_ovf ++;
    }
    uc_out_rh[i] = uc_data;
    
    f_data = 10.0*log10(f_ave_lh[i]);
    if (f_data > f_max[1]) f_max[1] = f_data;
    f_data = (f_data - crval) / bscale;
    uc_data = (unsigned char)f_data;
    if (isnan(f_data))
    {
      uc_data = 255;
    }
    else if (f_data < 0.0)
    {
      uc_data = 0;
    }
    else if (f_data > 254.0)
    {
      uc_data = 254;
      i_ovf ++;
    }
    uc_out_lh[i] = uc_data;

  }

  return i_ovf;

}

// ----------------------------------------------------------------------
// 8-bit compression of high resolution spectrum data
//
// return value : 0
// ----------------------------------------------------------------------
int compress_high_08bit_data(
  unsigned char *uc_out_rh,     // [out]  compressed RH spectrum data 
  unsigned char *uc_out_lh,     // [out]  compressed LH spectrum data 
  int n,                        // [in]   number of data points
  float *f_data_rh,             // [in]   uncompressed RH spectrum data
  float *f_data_lh,             // [in]   uncompressed LH spectrum data
  float *f_floor_rh,            // [in]   RH noise floor spectrum
  float *f_floor_lh,            // [in]   LH noise floor spectrum
  float bscale,                 // [in]   BSCALE
  float crval)                  // [in]   CRVAL
{

  int i;

  float f_data;
  unsigned char uc_data;

  for (i=0; i<n; i++)
  {
    f_data = (10.0*log10(f_data_rh[i]/f_floor_rh[i]) - crval) / bscale;
    uc_data = (unsigned char)f_data;
    if (isnan(f_data))
    {
      uc_data = 255;
    }
    else if (f_data < 0.0)
    {
      uc_data = 0;
    }
    else if (f_data > 254.0)
    {
      uc_data = 254;
    }
    uc_out_rh[i] = uc_data;
    
    f_data = (10.0*log10(f_data_lh[i]/f_floor_lh[i]) - crval) / bscale;
    uc_data = (unsigned char)f_data;
    if (isnan(f_data))
    {
      uc_data = 255;
    }
    else if (f_data < 0.0)
    {
      uc_data = 0;
    }
    else if (f_data > 254.0)
    {
      uc_data = 254;
    }
    uc_out_lh[i] = uc_data;

  }

  return 0;

}

// ----------------------------------------------------------------------
// get low resolution spectrum data
//
// return value : 0
// ----------------------------------------------------------------------
int get_average(
  float *f_ave_rh,          // [in/out]  low-resolution averaged or compositted RH spectrum data
  float *f_ave_lh,          // [in/out]  low-resolution averaged or compositted HH spectrum data
  int n,                    // [in]      number of data points
  int n_ave,                // [in]      number of average in frequency direction
  int i_cmd,                // [in]      command 0 : composit spectra, 1 : get averaged spectra
  float *f_data_rh,         // [in]      high-resolution RH spectrum data
  float *f_data_lh,         // [in]      high-resolution LH spectrum data
  float *f_floor_rh,        // [in]      high-resolution LH noise floor data
  float *f_floor_lh,        // [in]      high-resolution LH noise floor data
  unsigned int *n_sum)       // [in/out]  numbre of composit
{
  int i, j, ii;
  unsigned int n_sum_tmp;

  n_sum_tmp = *n_sum;

  //fprintf(stderr, "--- %d %d %f\n", n_ave, n_sum_tmp, f_ave_rh[0]);

  if (i_cmd == 0)
  {
    // composit spectra
    for (i=0; i<n; i+=n_ave)
    {
      ii = (int)(i/n_ave);
      for (j=0; j<n_ave; j++)
      {
        f_ave_rh[ii] += f_data_rh[i+j] / f_floor_rh[i+j];
        f_ave_lh[ii] += f_data_lh[i+j] / f_floor_lh[i+j];
      }
    }
    n_sum_tmp ++;
  }
  else
  {
    // get averaged spectra
    for (i=0; i<(int)(n/n_ave); i++)
    {
      if (n_sum_tmp)
      {
        f_ave_rh[i] /= (float)(n_ave * n_sum_tmp);
        f_ave_lh[i] /= (float)(n_ave * n_sum_tmp);
      }
      else
      {
        f_ave_rh[i] = nanf("0");
        f_ave_lh[i] = nanf("0");
      }
    }

  }

  *n_sum = n_sum_tmp;

  return 0;
}


// ----------------------------------------------------------------------
// get noise floor spectrum data
//
// return value : 0
// ----------------------------------------------------------------------
int get_noise_floor(
  float *f_floor_rh,         // [out]     high-resolution RH noise floor data
  float *f_floor_lh,         // [out]     high-resolution LH noise floor data
  unsigned int n)            // [in]      number of data
{

  int i;

  for (i=0;i<n;i++)
  {
    f_floor_rh[i] = 2.0e8;
    f_floor_lh[i] = 2.0e8;
  }

  return 0;
}

//---------------------------------------------------------------------------
// output VDIF header
//---------------------------------------------------------------------------
void vdif_output_log(vdif_header_type vdif_header)
{
  static int i_cnt=0;

  fprintf(stderr, "\n%d\n",     i_cnt++);
  fprintf(stderr, "invalid_data   : %d\n",     vdif_header.invalid_data);
  fprintf(stderr, "legacy_mode    : %d\n",     vdif_header.legacy_mode);
  fprintf(stderr, "second_epoch   : 0x%08x\n", vdif_header.second_epoch);

  fprintf(stderr, "un_assigned    : %d\n",     vdif_header.un_assigned);
  fprintf(stderr, "ref_epoch      : %d\n",     vdif_header.ref_epoch);
  fprintf(stderr, "data_frame_num : %d\n",     vdif_header.data_frame_num);
  
  fprintf(stderr, "version        : %d\n",     vdif_header.version);
  fprintf(stderr, "log_ch         : %d\n",     vdif_header.log_ch);
  fprintf(stderr, "data_frame_len : 0x%08x\n", vdif_header.data_frame_len);

  fprintf(stderr, "data_type      : %d\n",     vdif_header.data_type);
  fprintf(stderr, "bps            : %d\n",     vdif_header.bps);
  fprintf(stderr, "thread_id      : %d\n",     vdif_header.thread_id);
  fprintf(stderr, "station_id     : 0x%04x\n", vdif_header.station_id);

  fprintf(stderr, "evd            : %d\n",     vdif_header.evd);
  fprintf(stderr, "ext_user_data0 : 0x%08x\n", vdif_header.ext_user_data0);

  fprintf(stderr, "ext_user_data1 : 0x%08x\n", vdif_header.ext_user_data1);
  fprintf(stderr, "ext_user_data2 : 0x%08x\n", vdif_header.ext_user_data2);
  fprintf(stderr, "ext_user_data3 : 0x%08x\n", vdif_header.ext_user_data3);

  return;
}

//---------------------------------------------------------------------------
// output VDIF header (1 line)
//---------------------------------------------------------------------------
void vdif_output_log_line(vdif_header_type vdif_header)
{
  static int i_cnt=0;

  if (!i_cnt){
    fprintf(stderr, "   count");
    fprintf(stderr, "  sec_epoch");
    fprintf(stderr, " ref");
    fprintf(stderr, " fr_num");
    fprintf(stderr, " log_ch");
    fprintf(stderr, " data_len");
    fprintf(stderr, " th_id\n");
  }

  fprintf(stderr, "%8d",     i_cnt++);
  fprintf(stderr, " 0x%08x", vdif_header.second_epoch);
  fprintf(stderr, "%4d",     vdif_header.ref_epoch);
  fprintf(stderr, "%7d",    vdif_header.data_frame_num);
  fprintf(stderr, "%7d",     vdif_header.log_ch);
  fprintf(stderr, " 0x%06x", vdif_header.data_frame_len);
  fprintf(stderr, "%6d\n",  vdif_header.thread_id);

  return;
}

// ----------------------------------------------------------------------
// Oputput fits header source
// ----------------------------------------------------------------------
int write_header(FILE *fp, vdif_header_type hdr, int nt, unsigned int nf)
  {
    fprintf(fp, "Ref epoch\n");
    fprintf(fp, "%d\n", hdr.ref_epoch);
    fprintf(fp, "Seconds from Ref epoch of the first data\n");
    fprintf(fp, "%d\n", hdr.second_epoch);
    fprintf(fp, "Number of data (time, freq)\n");
    fprintf(fp, "%d %d\n", nt, nf);
    fprintf(fp, "Expected data size (Byte)\n");
    fprintf(fp, "%lu\n", nt * nf * sizeof(char));

    return 0;
  }

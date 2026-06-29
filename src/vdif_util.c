#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <time.h>
#include <fitsio.h>
#include <omp.h>

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
  float bzero)                  // [in]   CRVAL
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
    f_data = (f_data - bzero) / bscale;
    uc_data = (unsigned char)f_data;
    if (isnan(f_data))
    {
      // dummy data
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
    f_data = (f_data - bzero) / bscale;
    uc_data = (unsigned char)f_data;
    if (isnan(f_data))
    {
      uc_data = DUMMY_UC_DATA;
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
// 16-bit compression of high resolution spectrum data
//
// return value : 0
// ----------------------------------------------------------------------
int compress_high_16bit_data(
  unsigned short *us_out_rh,    // [out]  compressed RH spectrum data 
  unsigned short *us_out_lh,    // [out]  compressed LH spectrum data 
  int n,                        // [in]   number of data points
  float *f_data_rh,             // [in]   uncompressed RH spectrum data
  float *f_data_lh,             // [in]   uncompressed LH spectrum data
  float *f_floor_rh,            // [in]   RH noise floor spectrum
  float *f_floor_lh,            // [in]   LH noise floor spectrum
  float bscale,                 // [in]   BSCALE
  float bzero)                  // [in]   BZERO
{

  int i;

  float f_data;
  unsigned short us_data;

  for (i=0; i<n; i++)
  {
    f_data = (10.0*log10(f_data_rh[i]/f_floor_rh[i]) - bzero) / bscale;
    us_data = (unsigned short)f_data;
    if (isnan(f_data))
    {
      us_data = 65535;
    }
    else if (f_data < 0.0)
    {
      us_data = 0;
    }
    else if (f_data > 65534.0)
    {
      us_data = 65534;
    }
    us_out_rh[i] = us_data;
    
    f_data = (10.0*log10(f_data_lh[i]/f_floor_lh[i]) - bzero) / bscale;
    us_data = (unsigned short)f_data;
    if (isnan(f_data))
    {
      us_data = 65535;
    }
    else if (f_data < 0.0)
    {
      us_data = 0;
    }
    else if (f_data > 65534.0)
    {
      us_data = 65534;
    }
    us_out_lh[i] = us_data;

  }

  return 0;

}

// ----------------------------------------------------------------------
// 8-bit compression of high resolution spectrum data
//
// return value : number of over flow points in f_data_rh and f_data_lh.
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
  float bzero)                  // [in]   BZERO
{

  int i;
  int i_ovf = 0;

  float f_data;
  unsigned char uc_data;

  for (i=0; i<n; i++)
  {
    f_data = (10.0*log10(f_data_rh[i]/f_floor_rh[i]) - bzero) / bscale;
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
    
    f_data = (10.0*log10(f_data_lh[i]/f_floor_lh[i]) - bzero) / bscale;
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
// create Fits file
//
// return value : 0 if no error
// ----------------------------------------------------------------------
int create_fits(char *file_rh, char *file_lh, fits_header_type fits_hdr, int mode, int ver, int sver) {

  char str[256];
  int i_ret;

  FILE *fp_rh, *fp_lh;
  fitsfile *fptr;       // FITS file pointer
  int status = 0;       // CFITSIO status

  int n = fits_hdr.naxis1; // (Width / NAXIS1)
  int m = fits_hdr.naxis2; // (Height / NAXIS2)
    
  int naxis = 3;
  long naxes[3] = {n, m, 2}; // {NAXIS1, NAXIS2, NAXIS3}

  unsigned char *uc_data = (unsigned char *)malloc(2 * m * n * sizeof(unsigned char));
  unsigned char *uc_data_r = (unsigned char *)malloc(m * n * sizeof(unsigned char));
  unsigned char *uc_data_l = (unsigned char *)malloc(m * n * sizeof(unsigned char));

  fp_rh = fopen(file_rh, "rb");
  fp_lh = fopen(file_lh, "rb");
  fread(uc_data_r, m*n, sizeof(unsigned char), fp_rh);
  fread(uc_data_l, m*n, sizeof(unsigned char), fp_lh);

  // Transpose
  transpose_optimized(uc_data, uc_data_r, uc_data_l, m, n);

  // create FITS file
  char filename[256];
  if (mode == 0)
  {
    // for low-resolution data (iprt_amt_l1_low_08bit_yyyymmdd_vXX_XX.fits	)
    i_ret = snprintf(filename, sizeof(filename), "!data/iprt_amt_l1_low_%02dbit_%04d%02d%02d_v%02d_%02d.fits", 
      fits_hdr.bitpix,
      fits_hdr.date_obs[0], fits_hdr.date_obs[1], fits_hdr.date_obs[2],
      ver, sver);
  }else{
    // for high-resolution data (iprt_amt_l1_high_08bit_yyyymmdd-hhnn_vXX_XX.fits	)
    i_ret = snprintf(filename, sizeof(filename), "!data/iprt_amt_l1_high_%02dbit_%04d%02d%02d-%02d%02d_v%02d_%02d.fits", 
      fits_hdr.bitpix,
      fits_hdr.date_obs[0], fits_hdr.date_obs[1], fits_hdr.date_obs[2],
      fits_hdr.time_obs[0], fits_hdr.time_obs[1],
      ver, sver);
  }

  if (fits_create_file(&fptr, filename, &status)) {
    fits_report_error(stderr, status);
    free(uc_data);
    free(uc_data_r);
    free(uc_data_l);
    return status;
  }

  // create primary HDU
  if (fits_create_img(fptr, fits_hdr.bitpix, naxis, naxes, &status)) {
    fits_report_error(stderr, status);
    fits_close_file(fptr, &status);
    free(uc_data);
    free(uc_data_r);
    free(uc_data_l);
    return status;
  }

  // write data
  LONGLONG fpixel = 1;
  long nelements = 2 * m * n;
  int datatype = TBYTE;
  if(fits_hdr.bitpix == 16) datatype = TSHORT;
  if (fits_write_img(fptr, datatype, fpixel, nelements, uc_data, &status)) {
    fits_report_error(stderr, status);
  }

  // add header keywords
  i_ret = snprintf(str, sizeof(str), "%04d-%02d-%02d", fits_hdr.date_obs[0], fits_hdr.date_obs[1], fits_hdr.date_obs[2]);
  fits_update_key(fptr, TSTRING, "DATE", str, "", &status);

  fits_update_key(fptr, TSTRING, "ORIGIN", "PPARC TOHOKU University", "Organization name", &status);
  fits_update_key(fptr, TSTRING, "TELESCOP", "IPRT", "Name of the telescope", &status);
  fits_update_key(fptr, TSTRING, "INSTRUME", "AMATERAS", "Name of the spectrometer", &status);
  fits_update_key(fptr, TSTRING, "OBJECT", "SUN", "Name of the target", &status);

  i_ret = snprintf(str, sizeof(str), "%04d-%02d-%02d", fits_hdr.date_obs[0], fits_hdr.date_obs[1], fits_hdr.date_obs[2]);
  fits_update_key(fptr, TSTRING, "DATE-OBS", str, "Date observation started UT ", &status);
  i_ret = snprintf(str, sizeof(str), "%02d:%02d:%02d", fits_hdr.time_obs[0], fits_hdr.time_obs[1], fits_hdr.time_obs[2]);
  fits_update_key(fptr, TSTRING, "TIME-OBS", str, "Time observation started UT ", &status);
  i_ret = snprintf(str, sizeof(str), "%04d-%02d-%02d", fits_hdr.date_end[0], fits_hdr.date_end[1], fits_hdr.date_end[2]);
  fits_update_key(fptr, TSTRING, "DATE-END", str, "Date observation ended UT ", &status);
  i_ret = snprintf(str, sizeof(str), "%02d:%02d:%02d", fits_hdr.time_end[0], fits_hdr.time_end[1], fits_hdr.time_end[2]);
  fits_update_key(fptr, TSTRING, "TIME-END", str, "Time observation ended UT ", &status);

  fits_update_key(fptr, TFLOAT, "EXPTIME", &(fits_hdr.cdelt1),  "Exposure time in seconds", &status);
  fits_update_key(fptr, TFLOAT, "BZERO",   &(fits_hdr.bzero),   "Scaling offset [dB]",      &status);
  fits_update_key(fptr, TFLOAT, "BSCALE",  &(fits_hdr.bscale),  "Scaling factor [dB]",      &status);
  fits_update_key(fptr, TSTRING,"BUNIT",   "dB",                "Z-axis title",             &status);

  fits_update_key(fptr, TINT,   "DATAMIN", &(fits_hdr.datamin), "Minimum element in image", &status);
  fits_update_key(fptr, TINT,   "DATAMAX", &(fits_hdr.datamax), "Maximum element in image", &status);

  fits_update_key(fptr, TSTRING,"CTYPE1",  "Time in UT",        "Title of axis 1",          &status);
  fits_update_key(fptr, TINT,   "CRPIX1",  &(fits_hdr.crpix1),  "Reference pixel of axis 1",&status);
  fits_update_key(fptr, TFLOAT, "CRVAL1",  &(fits_hdr.crval1),  "Value on axis 1 at the reference pixel", &status);
  fits_update_key(fptr, TFLOAT, "CDELT1",  &(fits_hdr.cdelt1),  "Step between first and second elements in axis 1", &status);

  fits_update_key(fptr, TSTRING,"CTYPE2",  "Frequency in MHz",  "Title of axis 2",          &status);
  fits_update_key(fptr, TINT,   "CRPIX2",  &(fits_hdr.crpix2),  "Reference pixel of axis 2",&status);
  fits_update_key(fptr, TFLOAT, "CRVAL2",  &(fits_hdr.crval2),  "Value on axis 2 at the reference pixel", &status);
  fits_update_key(fptr, TFLOAT, "CDELT2",  &(fits_hdr.cdelt2),  "Step between first and second elements in axis 2", &status);

  fits_update_key(fptr, TSTRING,"CTYPE3",  "Polarization",      "Title of axis 3",          &status);
  fits_update_key(fptr, TINT,   "CRPIX3",  &(fits_hdr.crpix3),  "Reference pixel of axis 3",&status);
  fits_update_key(fptr, TFLOAT, "CRVAL3",  &(fits_hdr.crval3),  "Value on axis 3 at the reference pixel", &status);
  fits_update_key(fptr, TFLOAT, "CDELT3",  &(fits_hdr.cdelt3),  "Step between first and second elements in axis 3", &status);

  // Comments & history
  fits_write_comment(fptr, "----------------------------------------",  &status);
  i_ret = snprintf(str, sizeof(str), "Fits file version %02d.%02d", ver, sver);
  fits_write_comment(fptr, str,                                         &status);
  fits_write_comment(fptr, "Detailed description of AMATERAS: Iwai et al. (2012), Solar Phys. 277(2)",   &status);
  fits_write_comment(fptr, "     Iwai et al. (2012), Solar Phys. 277(2)",   &status);
  fits_write_comment(fptr, "     IPRT/AMATERAS:",   &status);
  fits_write_comment(fptr, "     A New Metric Spectrum Observation System for Solar Radio Bursts",   &status);
  fits_write_comment(fptr, "     https://dx.doi.org/10.1007/s11207-011-9919-y",   &status);
  fits_write_comment(fptr, "Tohoku University, All rights reserved.",   &status);
  fits_write_comment(fptr, "     https://pparc.gp.tohoku.ac.jp/research/iprt/",   &status);
  fits_write_comment(fptr, "     PI name: Hiroaki Misawa.",   &status);
  fits_write_comment(fptr, "     Co-I name: Fuminori Tsuchiya.",   &status);
  fits_write_history(fptr, "Created by amt_drs4_l0_l1.c",              &status);

  time_t rawtime;
  time(&rawtime);
  struct tm *timeinfo;
  timeinfo = localtime(&rawtime);
  strftime(str, sizeof(str), "      at %Y-%m-%dT%H:%M:%S", timeinfo);
  fits_write_history(fptr, str, &status);

  // close file
  if (fits_close_file(fptr, &status)) {
    fits_report_error(stderr, status);
  }
  free(uc_data);
  free(uc_data_r);
  free(uc_data_l);

  return status;
}

// ----------------------------------------------------------------------
// create Fits file (16-bit data)
//
// return value : 0 if no error
// ----------------------------------------------------------------------
int create_fits_16bit(char *file_rh, char *file_lh, fits_header_type fits_hdr, int mode, int ver, int sver) {

  char str[256];
  int i_ret;

  FILE *fp_rh, *fp_lh;
  fitsfile *fptr;       // FITS file pointer
  int status = 0;       // CFITSIO status

  int n = fits_hdr.naxis1; // (Width / NAXIS1)
  int m = fits_hdr.naxis2; // (Height / NAXIS2)
    
  int naxis = 3;
  long naxes[3] = {n, m, 2}; // {NAXIS1, NAXIS2, NAXIS3}

  unsigned short *us_data = (unsigned short *)malloc(2 * m * n * sizeof(unsigned short));
  unsigned short *us_data_r = (unsigned short *)malloc(m * n * sizeof(unsigned short));
  unsigned short *us_data_l = (unsigned short *)malloc(m * n * sizeof(unsigned short));

  fp_rh = fopen(file_rh, "rb");
  fp_lh = fopen(file_lh, "rb");
  fread(us_data_r, m*n, sizeof(unsigned short), fp_rh);
  fread(us_data_l, m*n, sizeof(unsigned short), fp_lh);

  // Transpose
  transpose_optimized_16bit(us_data, us_data_r, us_data_l, m, n);

  // create FITS file
  char filename[256];
  if (mode == 0)
  {
    // for low-resolution data (iprt_amt_l1_low_08bit_yyyymmdd_vXX_XX.fits	)
    i_ret = snprintf(filename, sizeof(filename), "!data/iprt_amt_l1_low_%02dbit_%04d%02d%02d_v%02d_%02d.fits", 
      fits_hdr.bitpix,
      fits_hdr.date_obs[0], fits_hdr.date_obs[1], fits_hdr.date_obs[2],
      ver, sver);
  }else{
    // for high-resolution data (iprt_amt_l1_high_08bit_yyyymmdd-hhnn_vXX_XX.fits	)
    i_ret = snprintf(filename, sizeof(filename), "!data/iprt_amt_l1_high_%02dbit_%04d%02d%02d-%02d%02d_v%02d_%02d.fits", 
      fits_hdr.bitpix,
      fits_hdr.date_obs[0], fits_hdr.date_obs[1], fits_hdr.date_obs[2],
      fits_hdr.time_obs[0], fits_hdr.time_obs[1],
      ver, sver);
  }

  if (fits_create_file(&fptr, filename, &status)) {
    fits_report_error(stderr, status);
    free(us_data);
    free(us_data_r);
    free(us_data_l);
    return status;
  }

  // create primary HDU
  if (fits_create_img(fptr, fits_hdr.bitpix, naxis, naxes, &status)) {
    fits_report_error(stderr, status);
    fits_close_file(fptr, &status);
    free(us_data);
    free(us_data_r);
    free(us_data_l);
    return status;
  }

  // write data
  LONGLONG fpixel = 1;
  long nelements = 2 * m * n;
  int datatype = TBYTE;
  if(fits_hdr.bitpix == 16) datatype = TSHORT;
  if (fits_write_img(fptr, datatype, fpixel, nelements, us_data, &status)) {
    fits_report_error(stderr, status);
  }

  // add header keywords
  i_ret = snprintf(str, sizeof(str), "%04d-%02d-%02d", fits_hdr.date_obs[0], fits_hdr.date_obs[1], fits_hdr.date_obs[2]);
  fits_update_key(fptr, TSTRING, "DATE", str, "", &status);

  fits_update_key(fptr, TSTRING, "ORIGIN", "PPARC TOHOKU University", "Organization name", &status);
  fits_update_key(fptr, TSTRING, "TELESCOP", "IPRT", "Name of the telescope", &status);
  fits_update_key(fptr, TSTRING, "INSTRUME", "AMATERAS", "Name of the spectrometer", &status);
  fits_update_key(fptr, TSTRING, "OBJECT", "SUN", "Name of the target", &status);

  i_ret = snprintf(str, sizeof(str), "%04d-%02d-%02d", fits_hdr.date_obs[0], fits_hdr.date_obs[1], fits_hdr.date_obs[2]);
  fits_update_key(fptr, TSTRING, "DATE-OBS", str, "Date observation started UT ", &status);
  i_ret = snprintf(str, sizeof(str), "%02d:%02d:%02d", fits_hdr.time_obs[0], fits_hdr.time_obs[1], fits_hdr.time_obs[2]);
  fits_update_key(fptr, TSTRING, "TIME-OBS", str, "Time observation started UT ", &status);
  i_ret = snprintf(str, sizeof(str), "%04d-%02d-%02d", fits_hdr.date_end[0], fits_hdr.date_end[1], fits_hdr.date_end[2]);
  fits_update_key(fptr, TSTRING, "DATE-END", str, "Date observation ended UT ", &status);
  i_ret = snprintf(str, sizeof(str), "%02d:%02d:%02d", fits_hdr.time_end[0], fits_hdr.time_end[1], fits_hdr.time_end[2]);
  fits_update_key(fptr, TSTRING, "TIME-END", str, "Time observation ended UT ", &status);

  fits_update_key(fptr, TFLOAT, "EXPTIME", &(fits_hdr.cdelt1),  "Exposure time in seconds", &status);
  fits_update_key(fptr, TFLOAT, "BZERO",   &(fits_hdr.bzero),   "Scaling offset [dB]",      &status);
  fits_update_key(fptr, TFLOAT, "BSCALE",  &(fits_hdr.bscale),  "Scaling factor [dB]",      &status);
  fits_update_key(fptr, TSTRING,"BUNIT",   "dB",                "Z-axis title",             &status);

  fits_update_key(fptr, TINT,   "DATAMIN", &(fits_hdr.datamin), "Minimum element in image", &status);
  fits_update_key(fptr, TINT,   "DATAMAX", &(fits_hdr.datamax), "Maximum element in image", &status);

  fits_update_key(fptr, TSTRING,"CTYPE1",  "Time in UT",        "Title of axis 1",          &status);
  fits_update_key(fptr, TINT,   "CRPIX1",  &(fits_hdr.crpix1),  "Reference pixel of axis 1",&status);
  fits_update_key(fptr, TFLOAT, "CRVAL1",  &(fits_hdr.crval1),  "Value on axis 1 at the reference pixel", &status);
  fits_update_key(fptr, TFLOAT, "CDELT1",  &(fits_hdr.cdelt1),  "Step between first and second elements in axis 1", &status);

  fits_update_key(fptr, TSTRING,"CTYPE2",  "Frequency in MHz",  "Title of axis 2",          &status);
  fits_update_key(fptr, TINT,   "CRPIX2",  &(fits_hdr.crpix2),  "Reference pixel of axis 2",&status);
  fits_update_key(fptr, TFLOAT, "CRVAL2",  &(fits_hdr.crval2),  "Value on axis 2 at the reference pixel", &status);
  fits_update_key(fptr, TFLOAT, "CDELT2",  &(fits_hdr.cdelt2),  "Step between first and second elements in axis 2", &status);

  fits_update_key(fptr, TSTRING,"CTYPE3",  "Polarization",      "Title of axis 3",          &status);
  fits_update_key(fptr, TINT,   "CRPIX3",  &(fits_hdr.crpix3),  "Reference pixel of axis 3",&status);
  fits_update_key(fptr, TFLOAT, "CRVAL3",  &(fits_hdr.crval3),  "Value on axis 3 at the reference pixel", &status);
  fits_update_key(fptr, TFLOAT, "CDELT3",  &(fits_hdr.cdelt3),  "Step between first and second elements in axis 3", &status);

  // Comments & history
  fits_write_comment(fptr, "----------------------------------------",  &status);
  i_ret = snprintf(str, sizeof(str), "Fits file version %02d.%02d", ver, sver);
  fits_write_comment(fptr, str,                                         &status);
  fits_write_comment(fptr, "Detailed description of AMATERAS: Iwai et al. (2012), Solar Phys. 277(2)",   &status);
  fits_write_comment(fptr, "     Iwai et al. (2012), Solar Phys. 277(2)",   &status);
  fits_write_comment(fptr, "     IPRT/AMATERAS:",   &status);
  fits_write_comment(fptr, "     A New Metric Spectrum Observation System for Solar Radio Bursts",   &status);
  fits_write_comment(fptr, "     https://dx.doi.org/10.1007/s11207-011-9919-y",   &status);
  fits_write_comment(fptr, "Tohoku University, All rights reserved.",   &status);
  fits_write_comment(fptr, "     https://pparc.gp.tohoku.ac.jp/research/iprt/",   &status);
  fits_write_comment(fptr, "     PI name: Hiroaki Misawa.",   &status);
  fits_write_comment(fptr, "     Co-I name: Fuminori Tsuchiya.",   &status);
  fits_write_history(fptr, "Created by amt_drs4_l0_l1.c",              &status);

  time_t rawtime;
  time(&rawtime);
  struct tm *timeinfo;
  timeinfo = localtime(&rawtime);
  strftime(str, sizeof(str), "      at %Y-%m-%dT%H:%M:%S", timeinfo);
  fits_write_history(fptr, str, &status);

  // close file
  if (fits_close_file(fptr, &status)) {
    fits_report_error(stderr, status);
  }
  free(us_data);
  free(us_data_r);
  free(us_data_l);

  return status;
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

// ----------------------------------------------------------------------
// Get current time
// ----------------------------------------------------------------------
int get_current_time(char *date_str) {
    time_t rawtime;
    time(&rawtime);

    struct tm *timeinfo;
    timeinfo = localtime(&rawtime);

    strftime(date_str, sizeof(date_str), "%Y-%m-%dT%H:%M:%S", timeinfo);

    return 0;
}

// ----------------------------------------------------------------------
// set Fits header
//
// return value : 0 if no error
// ----------------------------------------------------------------------
int set_fits_header(
    fits_header_type *fits_hdr,
    vdif_header_type hdr,
    int bitpix,
    float bscale,
    float bzero,
    unsigned int nt,
    unsigned int nf,
    float df,
    float dt,
    float start_freq
  )
{

  // find UNIX time
  time_t unix_time;
  vdif2unixtime(hdr.ref_epoch, hdr.second_epoch, &unix_time);

  struct tm *time_info = gmtime(&unix_time);

  if (time_info == NULL) {
    fprintf(stdout, "Error: Conversion failed in set_fits_header");
    return 1;
  }

  // date observation starts UT (yyyy-mm-dd)
  fits_hdr->date_obs[0] = time_info->tm_year + 1900;
  fits_hdr->date_obs[1] = time_info->tm_mon;
  fits_hdr->date_obs[2] = time_info->tm_mday;
  // time observation starts UT (hh:nn:ss)
  fits_hdr->time_obs[0] = time_info->tm_hour;
  fits_hdr->time_obs[1] = time_info->tm_min;
  fits_hdr->time_obs[2] = time_info->tm_sec;

  time_info->tm_sec += (int)(dt * nt);
  timegm(time_info);
  // date observation ends UT   (yyyy-mm-dd)
  fits_hdr->date_end[0] = time_info->tm_year + 1900;
  fits_hdr->date_end[1] = time_info->tm_mon;
  fits_hdr->date_end[2] = time_info->tm_mday;
  // time observation ends UT   (hh:nn:ss)
  fits_hdr->time_end[0] = time_info->tm_hour;
  fits_hdr->time_end[1] = time_info->tm_min;
  fits_hdr->time_end[2] = time_info->tm_sec;

  fits_hdr->bitpix = bitpix;
  fits_hdr->naxis = 3;
  fits_hdr->naxis1 = nt;
  fits_hdr->naxis2 = nf;
  fits_hdr->naxis3 = 2;
  fits_hdr->bzero = bzero;
  fits_hdr->bscale = bscale;
  if (bitpix == 8) fits_hdr->datamax = 254;
  else             fits_hdr->datamax = 65534;
  fits_hdr->datamin = 0;
  fits_hdr->crpix1 = 0;
  fits_hdr->crval1 = 0.0;
  fits_hdr->cdelt1 = dt;
  fits_hdr->crpix2 = 0;
  fits_hdr->crval2 = start_freq;
  fits_hdr->cdelt2 = df;
  fits_hdr->crpix3 = 0;
  fits_hdr->crval3 = 0.0;
  fits_hdr->cdelt3 = 1.0;

  // date of background data
  // fits_hdr->date_bg[10];   

  return 0;
}

// ----------------------------------------------------------------------
// 
// ----------------------------------------------------------------------
void transpose_optimized(unsigned char* uc_data, unsigned char* uc_data_r, unsigned char* uc_data_l, int m, int n) 
{
    int mn = m * n;

    // 1. 外側のブロックループをOpenMPで並列化
    #pragma omp parallel for collapse(2) schedule(static)
    for (int sj = 0; sj < m; sj += BLOCK_SIZE) {
        for (int si = 0; si < n; si += BLOCK_SIZE) {
            
            // 2. ブロック内部の小さなループ（この範囲ならキャッシュに収まる）
            for (int j = sj; j < sj + BLOCK_SIZE && j < m; j++) {
                for (int i = si; i < si + BLOCK_SIZE && i < n; i++) {
                    
                    int dst_idx = j * n + i;
                    int src_idx = j + i * m;
                    
                    // 3. 左右のデータを同時に処理して、メモリへの指令をまとめる
                    uc_data[0 * mn + dst_idx] = uc_data_r[src_idx];
                    uc_data[1 * mn + dst_idx] = uc_data_l[src_idx];
                }
            }
            
        }
    }
}

// ----------------------------------------------------------------------
// 
// ----------------------------------------------------------------------
void transpose_optimized_16bit(unsigned short* us_data, unsigned short* us_data_r, unsigned short* us_data_l, int m, int n) 
{
    int mn = m * n;

    // 1. 外側のブロックループをOpenMPで並列化
    #pragma omp parallel for collapse(2) schedule(static)
    for (int sj = 0; sj < m; sj += BLOCK_SIZE) {
        for (int si = 0; si < n; si += BLOCK_SIZE) {
            
            // 2. ブロック内部の小さなループ（この範囲ならキャッシュに収まる）
            for (int j = sj; j < sj + BLOCK_SIZE && j < m; j++) {
                for (int i = si; i < si + BLOCK_SIZE && i < n; i++) {
                    
                    int dst_idx = j * n + i;
                    int src_idx = j + i * m;
                    
                    // 3. 左右のデータを同時に処理して、メモリへの指令をまとめる
                    us_data[0 * mn + dst_idx] = us_data_r[src_idx];
                    us_data[1 * mn + dst_idx] = us_data_l[src_idx];
                }
            }
            
        }
    }
}

// ----------------------------------------------------------------------
// Get Unix time from VDIF time
// ----------------------------------------------------------------------
void vdif2unixtime(int ref_epoch, unsigned int vdif_seconds, time_t *unix_time) {

  // Get year and month (1 or 7) from ref_epoch
    int year = 2000 + (ref_epoch / 2);
    int is_july = ref_epoch % 2; // 0: 1/1, 1:7/1

    // Get day from 2000/1/1
    int days_since_2000 = 0;
    for (int y = 2000; y < year; y++) {
        // leap year
        if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) {
            days_since_2000 += 366;
        } else {
            days_since_2000 += 365;
        }
    }
    if (is_july) {
        int is_leap = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 1 : 0;
        days_since_2000 += 31 + (28 + is_leap) + 31 + 30 + 31 + 30;
    }

    // Get UNIX time
    *unix_time = (time_t)946684800 + ((time_t)days_since_2000 * 86400) + vdif_seconds;
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
int write_fits_header(FILE *fp, fits_header_type fits_hdr)
  {

    char str[256];
    int i_ret;

    i_ret = snprintf(str, sizeof(str), "%04d-%02d-%02d", fits_hdr.date_obs[0], fits_hdr.date_obs[1], fits_hdr.date_obs[2]);
    fprintf(fp, "%s\n", str);
    i_ret = snprintf(str, sizeof(str), "%02d:%02d:%02d", fits_hdr.time_obs[0], fits_hdr.time_obs[1], fits_hdr.time_obs[2]);
    fprintf(fp, "%s\n", str);
    i_ret = snprintf(str, sizeof(str), "%04d-%02d-%02d", fits_hdr.date_end[0], fits_hdr.date_end[1], fits_hdr.date_end[2]);
    fprintf(fp, "%s\n", str);
    i_ret = snprintf(str, sizeof(str), "%02d:%02d:%02d", fits_hdr.time_end[0], fits_hdr.time_end[1], fits_hdr.time_end[2]);
    fprintf(fp, "%s\n", str);

    fprintf(fp, "%d\n", fits_hdr.bitpix);
    fprintf(fp, "%d\n", fits_hdr.naxis);
    fprintf(fp, "%d\n", fits_hdr.naxis1);
    fprintf(fp, "%d\n", fits_hdr.naxis2);
    fprintf(fp, "%d\n", fits_hdr.naxis3);
    fprintf(fp, "%f\n", fits_hdr.bzero);
    fprintf(fp, "%f\n", fits_hdr.bscale);
    fprintf(fp, "%f\n", fits_hdr.datamax);
    fprintf(fp, "%f\n", fits_hdr.datamin);

    fprintf(fp, "%d\n", fits_hdr.crpix1);
    fprintf(fp, "%f\n", fits_hdr.crval1);
    fprintf(fp, "%f\n", fits_hdr.cdelt1);
    fprintf(fp, "%d\n", fits_hdr.crpix2);
    fprintf(fp, "%f\n", fits_hdr.crval2);
    fprintf(fp, "%f\n", fits_hdr.cdelt2);
    fprintf(fp, "%d\n", fits_hdr.crpix3);
    fprintf(fp, "%f\n", fits_hdr.crval3);
    fprintf(fp, "%f\n", fits_hdr.cdelt3);

    return 0;
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

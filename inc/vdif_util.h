#define DUMMY_UC_DATA 255
#define BLOCK_SIZE 32

// ----------------------------------------------------------------------
// sttucture for FITS header
// ----------------------------------------------------------------------
typedef struct fits_header_type_def
{
  int bitpix;         // bits per data value                             
  int naxis;          // number of axes                                  
  int naxis1;         // time axis                                               
  int naxis2;         // frequency axis
  int naxis3;         // Polarization. 0 is RCP and 1 is LCP             
  int date_obs[3];    // date observation starts UT (yyyy,mm,dd)
  int time_obs[4];    // time observation starts UT (hh,nn,ss,ms)
  int date_end[3];    // date observation ends UT   (yyyy,mm,dd)
  int time_end[4];    // time observation ends UT   (hh,nn,ss,ms)
  float bzero;        // scaling offset
  float bscale;       // scaling factor
  float datamax;      // Minimum element in image
  float datamin;      // Maximum element in image
  int crpix1;         // reference pixel of axis 1                       
  float crval1;       // value on axis 1 at the reference pixel          
  float cdelt1;       // step between first and second elements in axis 1
  int crpix2;         // reference pixel of axis 1                       
  float crval2;       // value on axis 1 at the reference pixel          
  float cdelt2;       // step between first and second elements in axis 1
  int crpix3;         // reference pixel of axis 1                       
  float crval3;       // value on axis 1 at the reference pixel          
  float cdelt3;       // step between first and second elements in axis 1
  int date_bg[3];     // date of background data   (hh,nn,ss,ms)
} fits_header_type;  

// sttucture for VDIF header
typedef struct vdif_header_type_def
{

  unsigned int second_epoch:30;     //  Seconds from reference epoch
  unsigned int legacy_mode:1;       //  Legacy mode (fixed to be 0)
  unsigned int invalid_data:1;      //  Invalid data (0:valid, 1:invalid)

  unsigned int data_frame_num:24;   //  Data frame number in one second
  unsigned int ref_epoch:6;         //  Reference epoch (LSB:6 months, 0x00=2000-01-01/00:00:00 UT)
  unsigned int un_assigned:2;       //  00b

  unsigned int data_frame_len:24;   //  Data length including header
  unsigned int log_ch:5;            //  Log2 (Number of channel in the data array)
  unsigned int version:3;           //  VDIF version (fixed to be 000b)

  unsigned int station_id:16;       //  Station ID (fixed to be 0x0000)
  unsigned int thread_id:10;        //  Thread ID (IF1 = Thread 1, IF2 = Thread 2)
  unsigned int bps:5;               //  bits/sample (fixed to be 0)  
  unsigned int data_type:1;         //  Data type (0:Real, 1:Complex) (fixed to be 0)

  unsigned int ext_user_data0:24;   //  Extended data (not used, 0x000000)  
  unsigned int evd:8;               //  Extended data version (fixed to be 0x00)

  unsigned int ext_user_data1;      //  Extended data (not used, 0x00000000)  
  unsigned int ext_user_data2;      //  Extended data (not used, 0x00000000)  
  unsigned int ext_user_data3;      //  Extended data (not used, 0x00000000)  

} vdif_header_type;

int compress_low_data(
  unsigned char *uc_out_rh,     // [out]  compressed RH spectrum data 
  unsigned char *uc_out_lh,     // [out]  compressed LH spectrum data 
  float *f_max,                 // [out]  maximum values in each RH, LF spectrum
  int n,                        // [in]   number of data points
  float *f_ave_rh,              // [in]   uncompressed RH spectrum data
  float *f_ave_lh,              // [in]   uncompressed LH spectrum data
  float bscale,                 // [in]   BSCALE
  float crval);                 // [in]   CRVAL

int compress_high_16bit_data(
  unsigned short *us_out_rh,    // [out]  compressed RH spectrum data 
  unsigned short *us_out_lh,    // [out]  compressed LH spectrum data 
  int n,                        // [in]   number of data points
  float *f_data_rh,             // [in]   uncompressed RH spectrum data
  float *f_data_lh,             // [in]   uncompressed LH spectrum data
  float *f_floor_rh,            // [in]   RH noise floor spectrum
  float *f_floor_lh,            // [in]   LH noise floor spectrum
  float bscale,                 // [in]   BSCALE
  float bzero);                 // [in]   BZERO

int compress_high_08bit_data(
  unsigned char *uc_out_rh,     // [out]  compressed RH spectrum data 
  unsigned char *uc_out_lh,     // [out]  compressed LH spectrum data 
  int n,                        // [in]   number of data points
  float *f_data_rh,             // [in]   uncompressed RH spectrum data
  float *f_data_lh,             // [in]   uncompressed LH spectrum data
  float *f_floor_rh,            // [in]   RH noise floor spectrum
  float *f_floor_lh,            // [in]   LH noise floor spectrum
  float bscale,                 // [in]   BSCALE
  float bzero);                 // [in]   BZERO

int create_fits(char *file_rh, char *file_lh, fits_header_type fits_hdr, int mode, int ver, int sver);
int create_fits_16bit(char *file_rh, char *file_lh, fits_header_type fits_hdr, int mode, int ver, int sver);

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
  unsigned int *n_sum);     // [in/out]  numbre of composit

int get_noise_floor(
  float *f_floor_rh,         // [out]     high-resolution RH noise floor data
  float *f_floor_lh,         // [out]     high-resolution LH noise floor data
  unsigned int n);           // [in]      number of data

int get_current_time(char *date_str);

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
  float start_freq);

void transpose_optimized(unsigned char* uc_data, unsigned char* uc_data_r, unsigned char* uc_data_l, int m, int n);
void transpose_optimized_16bit(unsigned short* us_data, unsigned short* us_data_r, unsigned short* us_data_l, int m, int n);

void vdif2unixtime(int ref_epoch, unsigned int vdif_seconds, time_t *unix_time);

void vdif_output_log(vdif_header_type vdif_header);
void vdif_output_log_line(vdif_header_type vdif_header);

int write_fits_header(FILE *fp, fits_header_type fits_hdr);
int write_header(FILE *fp, vdif_header_type hdr, int nt, unsigned int nf);

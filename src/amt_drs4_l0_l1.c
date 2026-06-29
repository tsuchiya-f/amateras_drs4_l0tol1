/*************************************************************************
  Read VDIF data (DRS4 for amateras)
  How to compile
   $ gcc -o ../bin/amt_drs4_l0_l1 amt_drs4_l0_l1.c
  How to use
   $ cat /opt/drs4/storage/storagemodule1/test_01.vdif | bin/amt_drs4_l0_l1 > data/data.bin 2> log/data.log
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <time.h>

#include "vdif_util.h"
#include "amt_drs4_l0_l1.h"

// Processing option
//#define INSERT_DUMMY

// ----------------------------------------------------------------------
// set frequency & L1 data index
//
// return value : 0
// ----------------------------------------------------------------------
int set_L1_data_index(
    unsigned int *index_rh,     // 
    unsigned int *index_lh,     // 
    float *f_freq_L1,           // 
    unsigned int *n_freq_l1,    // 
    unsigned int *n_freq_low)   // 
{
  int i;

  unsigned int idx_freq_rh_s = (int)((S_FREQ_L1 + DF*0.5 - S_FREQ_L0)/DF);
  unsigned int idx_freq_rh_e = (int)((E_FREQ_L1 + DF*0.5 - S_FREQ_L0)/DF);
  unsigned int idx_freq_lh_s = (int)((((LO_FREQ - (S_FREQ_L1 - DF*0.5))) - S_FREQ_L0)/DF);
  unsigned int idx_freq_lh_e = (int)((((LO_FREQ - (E_FREQ_L1 - DF*0.5))) - S_FREQ_L0)/DF);

  *n_freq_l1 = idx_freq_rh_e - idx_freq_rh_s + 1;
  *n_freq_low = (int)(*n_freq_l1 / N_AVE_FREQ);

  fprintf(stderr, "RH start freq index : %d\n", idx_freq_rh_s);
  fprintf(stderr, "RH end   freq index : %d\n", idx_freq_rh_e);
  fprintf(stderr, "LH start freq index : %d\n", idx_freq_lh_s);
  fprintf(stderr, "LH end   freq index : %d\n", idx_freq_lh_e);

  for (i=0; i<N*BAND; i++)
  {
    f_freq_L0[i] = (float)(OFFSET+i)*DF;
  }
  for (i=0; i<*n_freq_l1; i++)
  {
    index_rh[i] = idx_freq_rh_s + i;
    index_lh[i] = idx_freq_lh_s - i;
    f_freq_L1[i] = f_freq_L0[index_rh[i]];
  }

  return 0;
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
int main()
{
    int i;
    int i_ret;

    unsigned int seq_num[2];
    vdif_header_type hdr;
    vdif_header_type hdr0;            // header data for the first data
    vdif_header_type hdr_hr;          // header data for the first data (high-resolution data)

    fits_header_type fits_hdr;

    unsigned int n_freq_l1;
    unsigned int n_freq_low;

    FILE *fp_rh_high;
    FILE *fp_lh_high;
    FILE *fp_rh_high16;
    FILE *fp_lh_high16;
    FILE *fp_rh_low;
    FILE *fp_lh_low;

    // open low-resolution binnary data
    fp_rh_low  = fopen("data/rh_low.bin",  "wb");
    fp_lh_low  = fopen("data/lh_low.bin",  "wb");

    // initialize variables
    int idx_start = 0;
    int n_swp = 0;                   // number of sweep count
    int n_file = 0;                  // number of file (high-resolution data)
    int idx_swp_prev = 0;            // sweep index (sweep number since observation start)
    int idx_sec, idx_sec_prev = 0;   // second index (second since observation start)
    int ovf08 = 0;                   // number of overflow points in 8-bit high-resoluton data during 1-min.
    unsigned int n_sum = 0;          // number of composited spectra
    // dummy spectrum
    memset(f_data_nan, nanf("0"), sizeof(f_data_nan)); 
    // composited spectra
    memset(f_ave_rh, 0, sizeof(f_ave_rh)); 
    memset(f_ave_lh, 0, sizeof(f_ave_lh)); 

    // -----------------------------------
    // set frequency & L1 data index
    // -----------------------------------
    i_ret = set_L1_data_index(index_rh, index_lh, f_freq_L1, &n_freq_l1, &n_freq_low);
    fprintf(stderr, "N freq high : %d\n", n_freq_l1);
    fprintf(stderr, "N freq low  : %d\n", n_freq_low);

    // -----------------------------------
    // get noise floor data
    // -----------------------------------
    i_ret = get_noise_floor(f_floor_rh, f_floor_lh, n_freq_l1);

    while(!feof(stdin))
    {

      // -----------------------------------
      // (1-1) read L0 data (one sweep data)
      // -----------------------------------
      for (i=0; i<BAND; i++)
      {
        // read sequence number from stdin
        fread(&seq_num, sizeof(seq_num), 1, stdin);

        // read header from stdin
        fread(&hdr, sizeof(hdr), 1, stdin);
        // store the first VDIF header
        if (idx_start == 0) hdr0 = hdr;

        // read data (one packet)
        fread(&f_data[N*i], sizeof(float), N, stdin);

      }

      // -----------------------------------
      // (1-2) wait until tm_sec = 0
      // -----------------------------------
      if (idx_start == 0)
      {
        time_t unix_time;
        vdif2unixtime(hdr.ref_epoch, hdr.second_epoch, &unix_time);
        struct tm *time_info = gmtime(&unix_time);
        // fprintf(stdout, "sec = %d\n", time_info->tm_sec);
        if (time_info->tm_sec != 0) continue;
      }
      idx_start = 1;

      // -----------------------------------
      // (2-1) store one sweep data (for L1 data) 
      // -----------------------------------
      for (i=0; i<n_freq_l1; i++)
      {
        f_data_rh[i] = f_data[index_rh[i]];
        f_data_lh[i] = f_data[index_lh[i]]; 
      }

#ifdef INSERT_DUMMY
      // sweep index : update every sweep
      int idx_swp = (hdr.second_epoch - hdr0.second_epoch) * (1000 / INTEG) + (hdr.data_frame_num - hdr0.data_frame_num)/BAND;
      // write dummy data if data skip happends
      int n_dummy = idx_swp - idx_swp_prev - 1;
      idx_swp_prev = idx_swp;
      if (n_dummy)
      {
        fprintf(stderr, "find data skip (%d skips) at %d [sec].\n", n_dummy, idx_sec);

        for (i=0; i<n_dummy; i++)
        {
          memset(uc_out_rh, DUMMY_UC_DATA, sizeof(uc_out_rh)); 
          fwrite(uc_out_rh, sizeof(unsigned char), n_freq_l1, fp_rh_high);
          fwrite(uc_out_rh, sizeof(unsigned char), n_freq_l1, fp_lh_high);
          n_swp ++;
        }
      }
#endif

      // -----------------------------------
      // (2-2) write high-resolution data.
      // -----------------------------------
      if (n_swp == 0)
      {
        time_t unix_time;
        vdif2unixtime(hdr.ref_epoch, hdr.second_epoch, &unix_time);
        struct tm *time_info = gmtime(&unix_time);
        fprintf(stdout, "write high-resolution data at t=%d\n", time_info->tm_sec);
        hdr_hr = hdr;

        // open high-resolution data file
        fp_rh_high = fopen("data/rh_high.bin", "wb");
        fp_lh_high = fopen("data/lh_high.bin", "wb");
        fp_rh_high16 = fopen("data/rh_high16.bin", "wb");
        fp_lh_high16 = fopen("data/lh_high16.bin", "wb");
      }

      int i_ovf = compress_high_08bit_data(uc_out_rh, uc_out_lh, n_freq_l1, f_data_rh, f_data_lh, f_floor_rh, f_floor_lh, BSCALE, BZERO);
      ovf08 += i_ovf;
      fwrite(uc_out_rh, sizeof(unsigned char), n_freq_l1, fp_rh_high);
      fwrite(uc_out_lh, sizeof(unsigned char), n_freq_l1, fp_lh_high);

      i_ret = compress_high_16bit_data(us_out_rh, us_out_lh, n_freq_l1, f_data_rh, f_data_lh, f_floor_rh, f_floor_lh, BSCALE16, BZERO);
      fwrite(us_out_rh, sizeof(unsigned short), n_freq_l1, fp_rh_high16);
      fwrite(us_out_lh, sizeof(unsigned short), n_freq_l1, fp_lh_high16);

      n_swp ++;

      // close high-resolution data file
      if (n_swp == SPLIT_SEC * (1000 / INTEG))
      {
        fclose(fp_rh_high);
        fclose(fp_lh_high);
        fclose(fp_rh_high16);
        fclose(fp_lh_high16);
        
        // create high-resolution data header file (8-bit)
        int bitpix = 8;
        unsigned int nt = n_swp;
        float dt = (float)INTEG / 1000.0;
        float bscale = BSCALE;
        float bzero = BZERO;
        i_ret = set_fits_header(&fits_hdr, hdr_hr, bitpix, bscale, bzero, nt, n_freq_l1, DF, dt, f_freq_L1[0]);

        // create fits file
        int mode = 1; // high-resolution
        create_fits("data/rh_high.bin", "data/lh_high.bin", fits_hdr, mode, FITS_VER, FITS_SVER);

        // create high-resolution data header file (16-bit)
        float ovf_rate = (float)ovf08 / (float)(n_freq_l1 * n_swp * 2);
//        if (ovf_rate > 0.3)
        if (ovf_rate > 0.0)
        {
          int bitpix = 16;
          unsigned int nt = n_swp;
          float dt = (float)INTEG / 1000.0;
          float bscale = BSCALE16;
          float bzero = BZERO;
          i_ret = set_fits_header(&fits_hdr, hdr_hr, bitpix, bscale, bzero, nt, n_freq_l1, DF, dt, f_freq_L1[0]);

          // create fits file
          int mode = 1; // high-resolution
          create_fits_16bit("data/rh_high16.bin", "data/lh_high16.bin", fits_hdr, mode, FITS_VER, FITS_SVER);
        }

/*
        fp_rh_high = fopen("data/hdr_high.asc", "w");
        i_ret = write_fits_header(fp_rh_high, fits_hdr);
        fclose(fp_rh_high);
*/
        n_file ++;
        n_swp = 0;
        ovf08 = 0;
      }

      // -----------------------------------
      // (3-1) write low-resolution data.
      // -----------------------------------
      // second index : update every second
      idx_sec = hdr.second_epoch - hdr0.second_epoch;
      if (idx_sec != idx_sec_prev)
      {
        idx_sec_prev = idx_sec;

        // get averaged spectra (low-resolution data)
        int n_ave = N_AVE_FREQ;
        i_ret = get_average(f_ave_rh, f_ave_lh, n_freq_l1, n_ave, 1, f_data_rh, f_data_lh, f_floor_rh, f_floor_lh, &n_sum);

        // write low-resolution data
        float f_max[2];
        int i_ovf = compress_low_data(uc_out_rh, uc_out_lh, f_max, n_freq_low, f_ave_rh, f_ave_lh, BSCALE, BZERO);
        fwrite(uc_out_rh, sizeof(unsigned char), n_freq_low, fp_rh_low);
        fwrite(uc_out_lh, sizeof(unsigned char), n_freq_low, fp_lh_low);

        // write log
        float ovf_rate = (float)i_ovf / (float)(n_freq_low * 2) * 100.0;  // [%]
        fprintf(stderr, "%6d [sec] Epoch: %2d Second: %8d n_sum = %3d / ovf = %5.1f Max = %4.1f(RH) %4.1f(LH)\n", idx_sec, hdr.ref_epoch, hdr.second_epoch, n_sum, ovf_rate, f_max[0], f_max[1]);

        // reset spectra
        n_sum = 0;
        memset(f_ave_rh, 0, sizeof(f_ave_rh)); 
        memset(f_ave_lh, 0, sizeof(f_ave_lh)); 
      }

      // composit spectra
      int n_ave = N_AVE_FREQ;
      i_ret = get_average(f_ave_rh, f_ave_lh, n_freq_l1, n_ave, 0, f_data_rh, f_data_lh, f_floor_rh, f_floor_lh, &n_sum);

    }

    fclose(fp_rh_low);
    fclose(fp_lh_low);

    // -----------------------------------
    // (3-2) open for ascii data (fits header for low-resolution data)
    // -----------------------------------
    // create low-resolution data header file
    int bitpix = 8;
    unsigned int nt = idx_sec;
    float dt = 1.0;
    float df = DF * N_AVE_FREQ;
    float freq_min = f_freq_L1[0] + df * 0.5;
    float bscale = BSCALE;
    float bzero = BZERO;
    i_ret = set_fits_header(&fits_hdr, hdr0, bitpix, bscale, bzero, nt, n_freq_low, df, dt, freq_min);

    // create fits file
    int mode = 0; // low-resolution
    create_fits("data/rh_low.bin", "data/lh_low.bin", fits_hdr, mode, FITS_VER, FITS_SVER);

/*
    fp_rh_low = fopen("data/hdr_low.asc", "w");
    i_ret = write_fits_header(fp_rh_low, fits_hdr);
    fclose(fp_rh_low);
*/
    return 0;

}
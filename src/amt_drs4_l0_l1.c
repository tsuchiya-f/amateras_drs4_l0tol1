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
#include <fitsio.h>

#include "vdif_util.h"

// DRS4 fixed setting
#define N 256               // number of data (4Byte float)/packet
#define DF 0.075            // Frequency step (MHz)

// DRS4 optional setting (AMATERAS Ver.1)
#define INTEG 10            // Integration time [msec] (1, 10, or 100)
#define BAND 46             // Number of Band (883.2MHz/19.2MHz) for one sweep (unit of 19.2MHz = 75kHz x 256)
#define OFFSET 1200         // Frequency offset (90MHz/DF) (unit of 75kHz)

// AMATERAS receiver Local Frequency (AMATERAS Ver.1)
#define LO_FREQ 1050        // Local Frequency (MHz)

// AMATERAS Frequency range (AMATERAS Ver.1)
#define S_FREQ_L1 100       // Start frequency of L1 data [MHz]
#define E_FREQ_L1 500       // End frequency of L1 data [MHz]
#define N_AVE_FREQ 13       // Number of average in freqneucy direction (75 kHz to 975 kHz)
#define BSCALE 0.1
#define CRVAL  -3
#define SPLIT_SEC 60        // time duaration to split the high-resolution data in second

#define S_FREQ_L0 OFFSET*DF             // Start frequency of L0 data [MHz]
#define E_FREQ_L0 S_FREQ_L0+N*DF*BAND   // End frequency of L0 data [MHz]
#define N_FREQ_L0 N*BAND                // Number of frequency of L0 data
#define N_FREQ_L1 N_FREQ_L0/2           // Number of frequency of L1 data (approximate value)

float f_data[N_FREQ_L0];                // Level 0 spectrum data
float f_freq_L0[N_FREQ_L0];             // Level 0 frequency data
float f_freq_L1[N_FREQ_L0];             // Level 1 frequency data (high resolution)
float f_freq_L1_ave[N_FREQ_L0];         // Level 1 frequency data (averaged low resolution)

unsigned int index_rh[N_FREQ_L1];       // index to map Level 0 data to Level 1 RH data
unsigned int index_lh[N_FREQ_L1];       // index to map Level 0 data to Level 1 LH data
float f_data_rh[N_FREQ_L1];             // Level 1 spectrum data (RH, high resolution)
float f_data_lh[N_FREQ_L1];             // Level 1 spectrum data (LH, high resolution)
// composited spectra
float f_ave_rh[N_FREQ_L1];              // Level 1 spectrum data (RH, low resolution)
float f_ave_lh[N_FREQ_L1];              // Level 1 spectrum data (LH, low resolution)
// noise floor spectra
float f_floor_rh[N_FREQ_L1];            // Level 1 noise floor spectrum data (RH, high resolution)
float f_floor_lh[N_FREQ_L1];            // Level 1 noise floor spectrum data (LH, high resolution)
// dummy spectrum
float f_data_nan[N_FREQ_L1];            // Level 1 dummy spectrum data (filled with NaN)
// output buffer
unsigned char uc_out_rh[N_FREQ_L1];
unsigned char uc_out_lh[N_FREQ_L1];

// ----------------------------------------------------------------------
// set frequency & L1 data index
//
// return value : 0
// ----------------------------------------------------------------------
int set_L1_data_index(unsigned int *index_rh, unsigned int *index_lh, float *f_freq_L1, unsigned int *n_freq_l1, unsigned int *n_freq_low)
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

    unsigned int n_freq_l1;
    unsigned int n_freq_low;

    FILE *fp_rh_high;
    FILE *fp_lh_high;
    FILE *fp_rh_low;
    FILE *fp_lh_low;

    // open low-resolution binnary data
    fp_rh_low  = fopen("data/rh_low.bin",  "wb");
    fp_lh_low  = fopen("data/lh_low.bin",  "wb");

    // initialize variables
    int n_swp = 0;                   // number of sweep
    int n_file = 0;                  // number of file (high-resolution data)
    int idx_swp, idx_swp_prev = 0;   // sweep index (sweep number since observation start)
    int idx_sec, idx_sec_prev = 0;   // second index (second since observation start)
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

    while(1)
    {

      // -----------------------------------
      // read L0 data (one sweep data)
      // -----------------------------------
      for (i=0; i<BAND; i++)
      {
        // read sequence number from stdin
        fread(&seq_num, sizeof(seq_num), 1, stdin);

        // read header from stdin
        fread(&hdr, sizeof(hdr), 1, stdin);
        // store the first VDIF header
        if (i==0 && n_swp==0 && n_file==0) hdr0 = hdr;

        // read data (one packet)
        fread(&f_data[N*i], sizeof(float), N, stdin);

      }
      if (feof(stdin)) break;
 
      // -----------------------------------
      // set sweep & second indies (indies since observation start)
      // -----------------------------------
      // sweep index : update every sweep
      idx_swp = (hdr.second_epoch - hdr0.second_epoch) * (1000 / INTEG) + (hdr.data_frame_num - hdr0.data_frame_num)/BAND;
      // second index : update every second
      idx_sec = hdr.second_epoch - hdr0.second_epoch;

//      if (idx_swp > 12000) break;

      // -----------------------------------
      // store one sweep data (for L1 data) 
      // -----------------------------------
      for (i=0; i<n_freq_l1; i++)
      {
        f_data_rh[i] = f_data[index_rh[i]];
        f_data_lh[i] = f_data[index_lh[i]]; 
      }

/*
      // write dummy data if data skip happends
      int n_dummy = idx_swp - idx_swp_prev - 1;
      idx_swp_prev = idx_swp;
      if (n_dummy)
      {
        fprintf(stderr, "find data skip (%d skips) at %d [sec].\n", n_dummy, idx_sec);

        for (i=0; i<n_dummy; i++)
        {
          memset(uc_out_rh, 0, sizeof(uc_out_rh)); 
          fwrite(uc_out_rh, sizeof(unsigned char), n_freq_l1, fp_rh_high);
          fwrite(uc_out_rh, sizeof(unsigned char), n_freq_l1, fp_lh_high);
          n_swp ++;
        }
      }
*/

      // -----------------------------------
      // write high-resolution data.
      // -----------------------------------
      if (n_swp == 0)
      {
        char fname_rh[1024], fname_lh[1024];
        sprintf(fname_rh, "data/rh_high_%04d.bin",n_file);
        sprintf(fname_lh, "data/lh_high_%04d.bin",n_file);
        fp_rh_high = fopen(fname_rh, "wb");
        fp_lh_high = fopen(fname_lh, "wb");
        n_file ++;
      }

      i_ret = compress_high_08bit_data(uc_out_rh, uc_out_lh, n_freq_l1, f_data_rh, f_data_lh, f_floor_rh, f_floor_lh, BSCALE, CRVAL);
      fwrite(uc_out_rh, sizeof(unsigned char), n_freq_l1, fp_rh_high);
      fwrite(uc_out_lh, sizeof(unsigned char), n_freq_l1, fp_lh_high);
      n_swp ++;

      if (n_swp == SPLIT_SEC * (1000 / INTEG))
      {
        fclose(fp_rh_high);
        fclose(fp_lh_high);
        n_swp = 0;
      }

      // -----------------------------------
      // write low-resolution data.
      // -----------------------------------
      if (idx_sec != idx_sec_prev)
      {
        idx_sec_prev = idx_sec;

        // get averaged spectra (low-resolution data)
        int n_ave = N_AVE_FREQ;
        i_ret = get_average(f_ave_rh, f_ave_lh, n_freq_l1, n_ave, 1, f_data_rh, f_data_lh, f_floor_rh, f_floor_lh, &n_sum);

        if (idx_sec < 2)
        {
          for (i=0; i<n_freq_low; i++)
          {
            fprintf(stdout, "%d %d %f %f\n", idx_sec, i, f_ave_rh[i], f_ave_lh[i]);
          }
          fprintf(stdout, "\n");
        }

        // write low-resolution data
        int i_ovf;
        float f_max[2];
        i_ovf = compress_low_data(uc_out_rh, uc_out_lh, f_max, n_freq_low, f_ave_rh, f_ave_lh, BSCALE, CRVAL);
        fwrite(uc_out_rh, sizeof(unsigned char), n_freq_low, fp_rh_low);
        fwrite(uc_out_lh, sizeof(unsigned char), n_freq_low, fp_lh_low);

        // write log
        fprintf(stderr, "%6d [sec] Epoch: %2d Second: %8d n_sum = %3d / ovf = %3d Max = %4.1f(RH) %4.1f(LH)\n", idx_sec, hdr.ref_epoch, hdr.second_epoch, n_sum, i_ovf, f_max[0], f_max[1]);

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
    // open for ascii data (fits header)
    // -----------------------------------
    fp_rh_high = fopen("data/rh_high.asc", "w");
    fp_lh_high = fopen("data/lh_high.asc", "w");
    fp_rh_low  = fopen("data/rh_low.asc",  "w");
    fp_lh_low  = fopen("data/lh_low.asc",  "w");

    i_ret = write_header(fp_rh_high, hdr0, n_swp, n_freq_l1);
    i_ret = write_header(fp_lh_high, hdr0, n_swp, n_freq_l1);
    i_ret = write_header(fp_rh_low,  hdr0, idx_sec, n_freq_low);
    i_ret = write_header(fp_lh_low,  hdr0, idx_sec, n_freq_low);

    fclose(fp_rh_high);
    fclose(fp_lh_high);
    fclose(fp_rh_low);
    fclose(fp_lh_low);

    return 0;

}
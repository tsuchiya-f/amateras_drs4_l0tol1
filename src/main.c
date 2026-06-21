/*************************************************************************
  Read VDIF data (DRS4 for amateras)
  How to compile
   $ gcc -o ../bin/main main.c
  How to use
   $ cat /opt/drs4/storage/storagemodule1/test_01.vdif | bin/main > data/data.bin 2> log/data.log
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "../inc/vdif_util.h"

// DRS4 fixed setting
#define N 256               // number of data (4Byte float)/packet
#define DF 0.075            // Frequency step (MHz)

// DRS4 optional setting (AMATERAS Ver.1)
#define INTEG 10            // Integration time [msec]
#define BAND 46             // Number of Band (883.2MHz/19.2MHz)
#define OFFSET 1200         // Frequency offset (90MHz/DF)

// AMATERAS receiver Local Frequency (AMATERAS Ver.1)
#define LO_FREQ 1050        // Local Frequency (MHz)

// AMATERAS Frequency range (AMATERAS Ver.1)
#define S_FREQ_L1 100       // Start frequency of L1 data [MHz]
#define E_FREQ_L1 500       // End frequency of L1 data [MHz]

#define S_FREQ_L0 OFFSET*DF             // Start frequency of L0 data [MHz]
#define E_FREQ_L0 S_FREQ_L0+N*DF*BAND   // End frequency of L0 data [MHz]
#define N_FREQ_L0 N*BAND                // Number of frequency of L0 data

float f_data[N_FREQ_L0];
float f_freq[N_FREQ_L0];
float f_freq_rh[N_FREQ_L0];
float f_freq_lh[N_FREQ_L0];

int main()
{
    unsigned int ui_integ = INTEG;    // integration time [ms]: 1, 10, or 100
    unsigned int ui_band = BAND;      // number of packet (or band) for one sweep data (unit of 19.2MHz = 75kHz x 256)
    unsigned int ui_offset = OFFSET;  // frequency offset (unit of 75kHz)

    unsigned int seq_num[2];
    vdif_header_type hdr;

    int i, i_cnt=0;

    int n_freq_l0 = (int)((E_FREQ_L0 - S_FREQ_L0)/DF);
    int idx_freq_rh_s = (int)((S_FREQ_L1 + DF*0.5 - S_FREQ_L0)/DF);
    int idx_freq_rh_e = (int)((E_FREQ_L1 + DF*0.5 - S_FREQ_L0)/DF);
    int idx_freq_lh_s = (int)((((LO_FREQ - (S_FREQ_L1 - DF*0.5))) - S_FREQ_L0)/DF);
    int idx_freq_lh_e = (int)((((LO_FREQ - (E_FREQ_L1 - DF*0.5))) - S_FREQ_L0)/DF);
    int n_freq_l1 = idx_freq_rh_e - idx_freq_rh_s + 1;

    float f_data_rh;
    float f_data_lh;    

    fprintf(stderr, "RH start freq index : %d\n", idx_freq_rh_s);
    fprintf(stderr, "RH end   freq index : %d\n", idx_freq_rh_e);
    fprintf(stderr, "LH start freq index : %d\n", idx_freq_lh_s);
    fprintf(stderr, "LH end   freq index : %d\n", idx_freq_lh_e);
    fprintf(stderr, "N freq RH : %d\n", idx_freq_rh_e - idx_freq_rh_s + 1);
    fprintf(stderr, "N freq LH : %d\n", idx_freq_lh_e - idx_freq_lh_s - 1);

    for (i=0; i<N*ui_band; i++)
    {
      f_freq[i] = (float)(ui_offset+i)*DF;
    }
    for (i=0; i<n_freq_l1; i++)
    {
      f_freq_rh[i] = f_freq[idx_freq_rh_s + i];
      f_freq_lh[i] = f_freq[idx_freq_lh_s - i];
    }
    
    while(1)
    {

        // read data (one sweep)
        for (i=0; i<ui_band; i++)
        {
            // read sequence number from stdin
            fread(&seq_num, sizeof(seq_num), 1, stdin);

            // read header from stdin
            fread(&hdr, sizeof(hdr), 1, stdin);
            // output header to stderr
            vdif_output_log_line(hdr);
            //vdif_output_log(hdr);

            // read data (one packet)
            fread(&f_data[N*i], sizeof(float), N, stdin);

            if (feof(stdin)) break;
          }
 
        if (i_cnt < 2)
        {
/*
          for (i=0; i<N*ui_band; i++)
          {
              fprintf(stdout, "%d %f %f\n", (int)(i_cnt/ui_band), f_freq[i], f_data[i]);
          }  
*/
          for (i=0; i<n_freq_l1; i++)
          {
            f_data_rh = f_data[idx_freq_rh_s + i];
            f_data_lh = f_data[idx_freq_lh_s - i];
            fprintf(stdout, "%d %f %f %f %f\n", (int)(i_cnt/ui_band), f_freq_rh[i], f_data_rh, f_freq_lh[i], f_data_lh);
          }
          fprintf(stdout, "\n");
        }

        i_cnt++;
/*
        if (i_cnt%ui_band == 0)
        {
          fprintf(stdout, "\n");
        }
*/
/*
        // read sequence number from stdin
        fread(&seq_num, 1, sizeof(seq_num), stdin);

        // read header from stdin
        fread(&hdr, 1, sizeof(hdr), stdin);
        
        // calculate data size in Byte
        ui_data_sz = hdr.data_frame_len * 8 - sizeof(hdr) - 8;

        // output header to stderr
        vdif_output_log_line(hdr);

        // read data from stdin
        fread(buff, ui_data_sz, sizeof(char), stdin);

        // terminate if end-of-file is detected
        if (feof(stdin)) break;
*/
    }

    return 0;

}
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

// variables for VDIF data input
#define N 256
#define BAND_MAX 256
float f_data[N*BAND_MAX];

int main(int argc, char *argv[])
{
    unsigned int ui_integ;  // integration time [ms]: 1, 10, or 100
    unsigned int ui_band;   // number of packet (or band) for one sweep data (unit of 19.2MHz = 75kHz x 256)
    unsigned int ui_offset; // frequency offset (unit of 75kHz)

    if (argc != 4)
    {
        fprintf(stderr, "Usage $%s integ band offset\n", argv[0]);
        return 0;
    }else{
        sscanf(argv[1], "%d", &ui_integ);
        sscanf(argv[2], "%d", &ui_band);
        sscanf(argv[3], "%d", &ui_offset);
        fprintf(stderr, "Integ time [ms] = %d\n", ui_integ);
        fprintf(stderr, "Number of Band  = %d\n", ui_band);
        fprintf(stderr, "Freq offset     = %d\n", ui_offset);
    }

    unsigned int seq_num[2];
    vdif_header_type hdr;
    
    unsigned int ui_data_sz;
    int i, i_cnt=0;
    float f_freq;

    while(1)
    {

        // read data (one sweep)
        for (i=0; i<ui_band; i++)
        {
            if (feof(stdin)) break;
            // read sequence number from stdin
            fread(&seq_num, sizeof(seq_num), 1, stdin);

            // read header from stdin
            fread(&hdr, sizeof(hdr), 1, stdin);
            // output header to stderr
            vdif_output_log_line(hdr);
            //vdif_output_log(hdr);

            // calculate data size in Byte
            // sizeof(hdr)-32 * header size
            ui_data_sz = hdr.data_frame_len * 8 - sizeof(hdr) - 8;

          fread(&f_data[256*i], sizeof(float), 256, stdin);
        }
 
        for (i=0; i<256*ui_band; i++)
        {
          f_freq = (float)ui_offset*0.075 + (float)(i + (i_cnt%ui_band)*256)*0.075;
          fprintf(stdout, "%d %f %f\n", (int)(i_cnt/ui_band), f_freq, f_data[i]);
        }  

        i_cnt++;
        if (i_cnt%ui_band == 0)
        {
          fprintf(stdout, "\n");
        }






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
    }

    return 0;

}
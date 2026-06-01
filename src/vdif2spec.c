#include <stdio.h>
#include <stdlib.h>
#include "vdif_util.h"

// variables for VDIF data input
unsigned long long ll_seq_num;
vdif_header_type vdif_header;
float f_data[256*256];

//---------------------------------------------------------------------------
// main routine
//---------------------------------------------------------------------------
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

    int i, i_cnt=0;
    float f_freq;

    while (1)
    {
        // read data (one sweep)
        for (i=0; i<ui_band; i++)
        {
          if (feof(stdin)) break;
          fread(&ll_seq_num, sizeof(ll_seq_num), 1, stdin);
          fread(&vdif_header, sizeof(vdif_header), 1, stdin);
          fread(&f_data[256*i], sizeof(float), 256, stdin);
          vdif_output_log(vdif_header);
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

      }

}


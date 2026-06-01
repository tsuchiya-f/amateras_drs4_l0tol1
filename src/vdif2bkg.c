#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vdif_util.h"

// variables for VDIF data input
unsigned long long ll_seq_num;
vdif_header_type vdif_header;
float f_data[256*256];
float f_data_min[256*256];

//---------------------------------------------------------------------------
// main routine
//---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  unsigned int ui_band;   // number of packet (or band) for one sweep data (unit of 19.2MHz = 75kHz x 256)

    if (argc != 2)
    {
      fprintf(stderr, "Usage $%s band\n", argv[0]);
      return 0;
    }else{
      sscanf(argv[1], "%d", &ui_band);
      fprintf(stderr, "Number of Band  = %d\n", ui_band);
    }

    int i, i_cnt=0;

    while (1)
    {
        fprintf(stderr, "%d\n", i_cnt);
        // read data (one sweep)
        for (i=0; i<ui_band; i++)
        {
          if (feof(stdin)) break;
          fread(&ll_seq_num, sizeof(ll_seq_num), 1, stdin);
          fread(&vdif_header, sizeof(vdif_header), 1, stdin);
          fread(&f_data[256*i], sizeof(float), 256, stdin);
        }

        // store minimum spectrum
        if (i_cnt ++)
        {
            for (i=0; i<256*ui_band; i++)
            {
                if (f_data[i] < f_data_min[i]) f_data_min[i] = f_data[i];
            }    
        }else{
            memcpy(f_data_min, f_data, sizeof(float)*256*ui_band);
        }
    }
      
    // write data
    fwrite(f_data_min, sizeof(float), 256*ui_band, stdout);

}


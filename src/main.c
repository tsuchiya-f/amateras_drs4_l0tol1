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
#define N 1024
char buff[N];

int main()
{
    unsigned int seq_num[2];
    vdif_header_type hdr;
    
    unsigned int ui_data_sz;

    while(1)
    {
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
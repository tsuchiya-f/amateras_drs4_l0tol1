#include <stdio.h>
#include <stdlib.h>

// definition of header in VDIF data
struct vdif_header {
    //----------
    unsigned int sec_epoch:30;
    unsigned int legacy:1;                  // 0 fixed
    unsigned int invalid:1;                 // valid=0, invalid=1
    //----------
    unsigned int unassigned:2;              // 00 fixed
    unsigned int ref_epoch:6;
    unsigned int data_frame:24;
    //----------
    unsigned int data_frame_len:24;
    unsigned int log_chns:5;
    unsigned int version:3;                 // VDIF version, 000 fixed
    //----------
    unsigned int station_id:16;             // 0x0000 fixed
    unsigned int thread_id:10;
    unsigned int bits_sample:5;             // 0 fixed
    unsigned int data_type:1;               // Data type, 0 (real data) fixed
    //----------
    unsigned int ext_usr_data0:24;          // 0X000000 fixed
    unsigned int evd:8;                     // 0x00 fixed
    //----------
    unsigned int ext_usr_data1;             // 0X0000 0000 fixed
    //----------
    unsigned int ext_usr_data2;             // 0X0000 0000 fixed
    //----------
    unsigned int ext_usr_data3;             // 0X0000 0000 fixed
    //----------
};

// data buffer size for 1 packet
#define N 1024
char buff[N];
unsigned int seq_num[2];

int main()
{
    struct vdif_header hdr;
    
    unsigned int ui_data_sz;
        
    fprintf(stderr, "seq number  invalid \n");

    while(1)
    {
        // read sequence number from stdin
        fread(&seq_num, 1, sizeof(seq_num), stdin);

        // read header from stdin
        fread(&hdr, 1, sizeof(hdr), stdin);
        
        // calculate data size in Byte
        ui_data_sz = hdr.data_frame_len * 8 - sizeof(hdr) - 8;

        // output header to stderr
        fprintf(stderr, "%08x %08x %d  0x%08x %2d 0x%06x %2d %4d %04x %4d\n",
                seq_num[1], seq_num[0], hdr.invalid, 
                hdr.sec_epoch, hdr.ref_epoch, hdr.data_frame, hdr.log_chns, hdr.data_frame_len, 
                hdr.thread_id,
                ui_data_sz);

        // read data from stdin
        fread(buff, ui_data_sz, sizeof(char), stdin);

        // terminate if end-of-file is detected
        if (feof(stdin)) break;
    }

    return 0;

}
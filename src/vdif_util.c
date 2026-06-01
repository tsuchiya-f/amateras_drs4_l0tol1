#include <stdio.h>
#include <stdlib.h>
#include "vdif_util.h"

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

#define FITS_VER    2
#define FITS_SVER   0

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
#define BZERO  -3
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


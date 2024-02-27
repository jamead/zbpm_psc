
//The PSC interface defines many different MsgID's
#define MSGHDRLEN 8

//This message is for slow status
#define MSGID30 30
#define MSGID30LEN 748   //in bytes

//This message is for 10Hz data
#define MSGID31 31
#define MSGID31LEN 316   //in bytes

//This message is for ADC waveform
//
#define MSGID51 51 
#define MSGID51LEN 80000   //in bytes

//This message is for TbT waveform
//1024 pts * 7vals * 4bytes/val (a,b,c,d,x,y,sum)
#define MSGID52 52 
#define MSGID52LEN 80000   //in bytes



//This message is for system info
#define MSGID32 32 
#define MSGID32LEN 400  //in bytes

//All messages have an 8 byte header
#define MSGHDRLEN 8


// Control Message Offsets
#define SOFT_TRIG_MSG1 0
#define PILOT_TONE_ENB_MSG1 8
#define DMA_TRIG_SRC_MSG1 52
#define MACHINE_SEL_MSG1 76
#define PILOT_TONE_SPI_MSG1 104
#define RF_ATTEN_MSG1 132
#define PT_ATTEN_MSG1 136
#define KX_MSG1 144
#define KY_MSG1 148
#define BBA_XOFF_MSG1 152
#define BBA_YOFF_MSG1 156
#define CHA_GAIN_MSG1 160
#define CHB_GAIN_MSG1 164
#define CHC_GAIN_MSG1 168
#define CHD_GAIN_MSG1 172
#define FINE_TRIG_DLY_MSG1 192  //Geo Delay
#define TBT_GATE_WIDTH_MSG1 196
#define COARSE_TRIG_DLY_MSG1 272
#define TRIGTOBEAM_THRESH_MSG1 276
#define EVENT_NO_MSG1 320

//General Status Readback Offsets
//10 Hz for now
#define TRIG_STRIG_MSG30 0
#define TRIG_SA_CNT_MSG30 4
#define TS_NS_MSG30 8 
#define TS_S_MSG30 12 
#define TS_NS_LAT_MSG30 16 
#define TS_S_LAT_MSG30 20 
#define TRIG_EVENTNO_MSG30 24
#define PLL_LOCKED_MSG30 60
#define TRIG_DMA_CNT_MSG30 76 
#define PT_RFENB_MSG30 128
#define RF_ATTEN_MSG30 132
#define PT_ATTEN_MSG30 136
#define POS_KX_MSG30 144
#define POS_KY_MSG30 148
#define KX_MSG30 144
#define KY_MSG30 148
#define BBA_XOFF_MSG30 152
#define BBA_YOFF_MSG30 156
#define CHA_GAIN_MSG30 160
#define CHB_GAIN_MSG30 164
#define CHC_GAIN_MSG30 168
#define CHD_GAIN_MSG30 172
#define TBT_GATE_DLY_MSG30 192
#define TBT_GATE_WIDTH_MSG30 196
#define COARSE_TRIG_DLY_MSG30 272
#define TRIGTOBEAM_THRESH_MSG30 276
#define TRIGTOBEAM_DLY_MSG30 280

//Position Data Readback
//Inj - On Trigger
//Ring - 10Hz
#define AMPL_ASA_MSG31 20
#define AMPL_BSA_MSG31 24
#define AMPL_CSA_MSG31 28
#define AMPL_DSA_MSG31 32
#define POS_X_MSG31 36
#define POS_Y_MSG31 40
#define AMPL_SUM_MSG31 44
#define TRIG_EVENTNO_MSG31 84



// System Health Status Readback offsets
// 1Hz 
#define FPGA_VER_MSG32 0 
#define TEMP_DFESENSE0_MSG32 4 
#define TEMP_DFESENSE1_MSG32 8 
#define TEMP_DFESENSE2_MSG32 12 
#define TEMP_DFESENSE3_MSG32 16 
#define TEMP_AFESENSE0_MSG32 20 
#define TEMP_AFESENSE1_MSG32 24
#define FPGA_DIETEMP_MSG32 28
#define SYS_UPTIME_MSG32 32
#define SFP_TEMP_MSG32 100  // to 216
#define PWR_MON_MSG32 220



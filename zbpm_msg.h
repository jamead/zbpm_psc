
//The PSC interface defines many different MsgID's
#define MSGHDRLEN 8

//This message is for slow status
#define MSGID30 30
#define MSGID30LEN 748   //in bytes

//This message is for 10Hz data
#define MSGID31 31
#define MSGID31LEN 316   //in bytes

//This message is for ADC waveform
#define MSGID51 51 
#define MSGID51LEN 80000   //in bytes
#define MSGHDRLEN 8



#define SOFT_TRIG_MSG1 0
#define PILOT_TONE_ENB_MSG1 8
#define PILOT_TONE_SPI_MSG1 104
#define RF_ATTEN_MSG1 132
#define PT_ATTEN_MSG1 136
#define KX_MSG1 144
#define KY_MSG1 148
#define CHA_GAIN_MSG1 160
#define CHB_GAIN_MSG1 164
#define CHC_GAIN_MSG1 168
#define CHD_GAIN_MSG1 172
#define FINE_TRIG_DLY_MSG1 192  //Geo Delay
#define TBT_GATE_WIDTH_MSG1 196
#define COARSE_TRIG_DLY_MSG1 272
#define EVENT_NO_MSG1 320


#define TRIG_STRIG_MSG30 0
#define RF_ATTEN_MSG30 132
#define PT_ATTEN_MSG30 136
#define POS_KX_MSG30 144
#define POS_KY_MSG30 148
#define KX_MSG30 144
#define KY_MSG30 148
#define CHA_GAIN_MSG30 160
#define CHB_GAIN_MSG30 164
#define CHC_GAIN_MSG30 168
#define CHD_GAIN_MSG30 172
#define TBT_GATE_DLY_MSG30 192
#define TBT_GATE_WIDTH_MSG30 196
#define FPGA_VER_MSG30 204
#define TEMP_DFESENSE0_MSG30 208
#define TEMP_DFESENSE1_MSG30 212
#define TEMP_DFESENSE2_MSG30 216
#define TEMP_DFESENSE3_MSG30 220
#define TEMP_AFESENSE0_MSG30 224
#define TEMP_AFESENSE1_MSG30 228
#define COARSE_TRIG_DLY_MSG30 272




#define TS_MSG31 12
#define TS_NS_MSG31 16
#define AMPL_ASA_MSG31 20
#define AMPL_BSA_MSG31 24
#define AMPL_CSA_MSG31 28
#define AMPL_DSA_MSG31 32
#define POS_X_MSG31 36
#define POS_Y_MSG31 40
#define AMPL_SUM_MSG31 44
#define TRIG_EVENTNO_MSG31 84
#define CNT_TRIG_MSG31 96


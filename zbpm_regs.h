

//zbpm has 3 different IO memory spaces
#define IOBUS_BASEADDR   0x43C00000
#define FABUS_BASEADDR   0x43C10000
#define LIVEBUS_BASEADDR 0x43C20000

//IO Bus Registers
#define PLL_LOCKED_REG 0
#define RF_DSA_REG 3
#define PT_DSA_REG 6
#define INT_TRIG_ENB_REG 7  
#define TRIG_EVRINT_SEL_REG 9

#define TEMP_DFE0_REG 28
#define TEMP_DFE1_REG 29
#define TEMP_DFE2_REG 30 
#define TEMP_DFE3_REG 31 
#define TEMP_AFE0_REG 32 
#define TEMP_AFE1_REG 33 
#define KX_REG 36
#define KY_REG 37
#define CHA_GAIN_REG 38
#define CHB_GAIN_REG 39
#define CHC_GAIN_REG 40
#define CHD_GAIN_REG 41
#define BBA_XOFF_REG 42
#define BBA_YOFF_REG 43
#define TBT_GATEDLY_REG 44
#define TBT_GATEWID_REG 45

#define COARSE_TRIG_DLY_REG 77
#define FINE_TRIG_DLY_REG 78
#define EVR_DMA_TRIGNUM_REG 79
#define SOFT_DMA_TRIG_REG 80

#define FPGA_VER_REG 100
#define VIVADO_VER_REG 101
#define EVR_TS_NS_REG 102
#define EVR_TS_S_REG 103
#define EVR_TS_NS_LAT_REG 104
#define EVR_TS_S_LAT_REG 105
#define SA_TRIGNUM_REG 106
#define DMA_TRIGCNT_REG 107

#define SA_CHA_REG 110
#define SA_CHB_REG 111
#define SA_CHC_REG 112
#define SA_CHD_REG 113
#define SA_XPOS_REG 114
#define SA_YPOS_REG 115
#define SA_SUM_REG 117


//Live Bus Registers
#define ADCFIFO_DATA_REG 16
#define ADCFIFO_CNT_REG 17
#define ADCFIFO_RST_REG 18
#define ADCFIFO_STREAMENB_REG 19




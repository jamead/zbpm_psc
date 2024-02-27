


#define PORT 7

#define RFATTEN 0
#define PTATTEN 1
#define HOR 0
#define VERT 1
#define CHA 0
#define CHB 1
#define CHC 2
#define CHD 3

#define STRAIGHT 0
#define RING 1


void prog_ad9510();
void GetPwrManagement(char *);

void *psc_cntrl_thread(void *);
void *psc_status_thread(void *);
void *psc_wvfm_thread(void *);






CC = gcc
NAME = zbpm_psc

CFLAGS = -g -Wall
DEPS = zbpm_regs.h zbpm_msgs.h zbpm_defs.h
OBJS = zbpm_main.o ad9510.o ltc2977.o psc_cntrl_thread.o psc_status_thread.o psc_wvfm_thread.o
LIBS = -lpthread -lm -li2c

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(NAME): $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o
	rm -f *~
	rm -f $(NAME)

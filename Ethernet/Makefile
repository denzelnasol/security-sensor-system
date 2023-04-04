OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
# CFLAGS = -Wall -g -std=c99 -Werror -D _POSIX_C_SOURCE=200809L -Wshadow
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Wshadow # TEMPORARY: Replace with commented code when testing is finished

CLIENT_SOURCE = ethernetClient.c clientNet.c 
HOST_SOURCE = ethernetServer.c serverNet.c ../WebCam/Stream/Stream.c ../Utilities/utilities.c ../Timer/timer.c

all:
	$(CC_C) $(CFLAGS) $(CLIENT_SOURCE) -o  $(OUTDIR)/eclient
	$(CC_C) $(CFLAGS) $(HOST_SOURCE) -o $(OUTDIR)/ehost

clean: 
	rm -f $(OUTDIR)/eclient
	rm -f $(OUTDIR)/ehost

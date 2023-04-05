OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c99 -Werror -D _POSIX_C_SOURCE=200809L -Wshadow
# CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Wshadow # TEMPORARY: Replace with commented code when testing is finished

UTILS = ../Utilities/utilities.c ../Timer/timer.c
STREAM = ../WebCam/Stream/Stream.c ../WebCam/Stream/Popen.c
SERVER = Server/ethernetServer.c Server/serverNet.c
HOST_SOURCE = $(UTILS) $(STREAM) $(SERVER)

all:
	$(CC_C) $(CFLAGS) $(HOST_SOURCE) -o $(OUTDIR)/camera

clean: 
	rm -f $(OUTDIR)/camera

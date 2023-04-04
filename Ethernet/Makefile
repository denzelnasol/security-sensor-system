OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
# CFLAGS = -Wall -g -std=c99 -Werror -D _POSIX_C_SOURCE=200809L -Wshadow
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Wshadow # TEMPORARY: Replace with commented code when testing is finished

all:
	$(CC_C) $(CFLAGS) client.c -o  $(OUTDIR)/eclient
	$(CC_C) $(CFLAGS) host.c -o  $(OUTDIR)/ehost

clean:
	rm -f $(OUTDIR)/eclient
	rm -f $(OUTDIR)/ehost

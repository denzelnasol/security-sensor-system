OUTDIR = $(HOME)/cmpt433/public/myApps
OUTFILE = ssh-client

DEPS = Utilities/utilities.c EventLogger/logger.c RemoteLoginClient/clientNet.c
SSH_CLIENT = RemoteLoginClient/remoteLoginClient.c
SOURCE = $(DEPS) $(SSH_CLIENT)

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c99 -Werror -D _POSIX_C_SOURCE=200809L -Wshadow

all:
	@echo "Building ssh client..."
	$(CC_C) $(CFLAGS) $(SOURCE) -o  $(OUTDIR)/$(OUTFILE)
	
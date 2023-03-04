MOTION = MotionSensor/motionSensor.c MotionSensor/motionSensorController.c
UTILITIES = Utilities/utilities.c Timer/timer.c
SOURCE = $(UTILITIES) ${MOTION}

# OUTDIR = $(HOME)/cmpt433/public/myApps
OUTDIR = ../build
OUTFILE = securitySensorSystem

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c99 -Werror -D _POSIX_C_SOURCE=200809L -Wshadow
#CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Wshadow # TEMPORARY: Replace with commented code when testing is finished

all:
	$(CC_C) $(CFLAGS) -pthread securitySensorSystem.c $(SOURCE) -o  $(OUTDIR)/$(OUTFILE) -lpthread


wsl:
	gcc $(CFLAGS) -pthread securitySensorSystem.c $(SOURCE) -o  $(OUTDIR)/$(OUTFILE) -lpthread


clean:
	rm -f *~ *.o $(OUTDIR)/$(OUTFILE)

# AUDIO = Audio/BeatGenerator/beatGenerator.c Audio/AudioMixer/audioMixer.c
UTILITIES = Utilities/utilities.c
WEBCAM = WebCam/Recorder/Recorder.c
# NETWORKING = Server/Networking/network.c Server/Networking/responseHandler.c
# ZENCAPE = ZenCape/Accelerometer/accelerometer.c ZenCape/Accelerometer/accController.c ZenCape/Joystick/joystick.c
SOURCE = $(UTILITIES) ${WEBCAM}

OUTDIR = $(HOME)/cmpt433/public/myApps
OUTFILE = securitySensorSystem

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
# CFLAGS = -Wall -g -std=c99 -Werror -D _POSIX_C_SOURCE=200809L -Wshadow
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Wshadow # TEMPORARY: Replace with commented code when testing is finished

all:
	make src

src:
	$(CC_C) $(CFLAGS) -pthread securitySensorSystem.c $(SOURCE) -o  $(OUTDIR)/$(OUTFILE) $(LFLAGS) -lpthread

clean:
	rm -f *~ *.o $(OUTDIR)/$(OUTFILE)

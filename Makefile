MOTION = MotionSensor/motionSensor.c MotionSensor/motionSensorController.c
UTILITIES = Utilities/utilities.c Timer/timer.c
PASSWORD = PasswordInput/passwordInput.c
JOYSTICK = Joystick/joystick.c
LOGGER = EventLogger/logger.c
SOURCE = $(UTILITIES) ${MOTION} ${JOYSTICK} ${PASSWORD} ${LOGGER}

OUTDIR = $(HOME)/cmpt433/public/myApps
OUTFILE = securitySensorSystem

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c99 -Werror -D _POSIX_C_SOURCE=200809L -Wshadow
#CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Wshadow # TEMPORARY: Replace with commented code when testing is finished

## wsl compile ##
# MOCK_OBJECTS = Menu/MockObjects/joystick.c 
CLIENT_DEPS = Utilities/utilities.c EventLogger/logger.c RemoteLoginClient/clientNet.c
CLIENT_SRC = $(CLIENT_DEPS)
CLIENT = RemoteLoginClient/remoteLoginClient.c

SERVER_DEPS = ResponseHandler/serverNet.c EventLogger/logger.c Settings/settings.c LEDMFA/ledMfa.c DangerAnalyzer/dangerAnalyzer.c
OTHER_DEPS = Utilities/utilities.c Timer/timer.c
SERVER_SRC = $(SERVER_DEPS) $(OTHER_DEPS)
SERVER = ResponseHandler/responseHandler.c

#################

all:
	$(CC_C) $(CFLAGS) -pthread securitySensorSystem.c $(SOURCE) -o  $(OUTDIR)/$(OUTFILE) -lpthread


wsl:
	# gcc $(CFLAGS) $(CLIENT) $(CLIENT_DEPENDENCIES) -o  ../build/client
	# gcc $(CFLAGS) $(SERVER) $(SERVER_DEPENDENCIES) -o  ../build/server
	# gcc $(CFLAGS) $(MENU) $(DEPENDENCIES) -o  ../build/menu

	$(CC_C) $(CFLAGS) -pthread $(SERVER) $(SERVER_SRC) -o  $(OUTDIR)/server -lpthread
	$(CC_C) $(CFLAGS) -pthread $(CLIENT) $(CLIENT_SRC) -o  $(OUTDIR)/client -lpthread


clean:
	rm -f *~ *.o $(OUTDIR)/$(OUTFILE)

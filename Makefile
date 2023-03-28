WEBCAM = WebCam/Recorder/Recorder.c WebCam/Stream/Stream.c WebCam/Stream/StreamController.c
MOTION = MotionSensor/motionSensor.c MotionSensor/motionSensorController.c
UTILITIES = Utilities/utilities.c Timer/timer.c
PASSWORD = PasswordInput/passwordInput.c
JOYSTICK = Joystick/joystick.c
SOURCE = $(UTILITIES) ${MOTION} ${JOYSTICK} ${PASSWORD} ${WEBCAM}

OUTDIR = $(HOME)/cmpt433/public/myApps
OUTFILE = securitySensorSystem

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c99 -Werror -D _POSIX_C_SOURCE=200809L -Wshadow
#CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Wshadow # TEMPORARY: Replace with commented code when testing is finished

all:
	$(CC_C) $(CFLAGS) -pthread securitySensorSystem.c $(SOURCE) -o  $(OUTDIR)/$(OUTFILE) -lpthread
	sudo mkdir -p $(OUTDIR)/Server-copy/
	sudo cp -R Server/* $(OUTDIR)/Server-copy/
	cd $(OUTDIR)/Server-copy/ && sudo npm install

wsl:
	gcc $(CFLAGS) -pthread securitySensorSystem.c $(SOURCE) -o  ../build/$(OUTFILE) -lpthread

src:
	$(CC_C) $(CFLAGS) -pthread securitySensorSystem.c $(SOURCE) -o  $(OUTDIR)/$(OUTFILE) $(LFLAGS) -lpthread

clean:
	rm -f *~ *.o $(OUTDIR)/$(OUTFILE)
	rm -f *~ *.o $(OUTDIR)/Server-Copy


# MOTION = MotionSensor/motionSensor.c MotionSensor/motionSensorController.c
# UTILITIES = Utilities/utilities.c Timer/timer.c
# PASSWORD = PasswordInput/passwordInput.c
# JOYSTICK = Joystick/joystick.c
# LOGGER = EventLogger/logger.c
# SOURCE = $(UTILITIES) ${MOTION} ${JOYSTICK} ${PASSWORD} ${LOGGER}

# OUTDIR = $(HOME)/cmpt433/public/myApps
# OUTFILE = securitySensorSystem

# CROSS_COMPILE = arm-linux-gnueabihf-
# CC_C = $(CROSS_COMPILE)gcc
# CFLAGS = -Wall -g -std=c99 -Werror -D _POSIX_C_SOURCE=200809L -Wshadow
# #CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Wshadow # TEMPORARY: Replace with commented code when testing is finished

# ## wsl compile ##
# # MOCK_OBJECTS = Menu/MockObjects/joystick.c 
# # CLIENT_DEPS = Utilities/utilities.c EventLogger/logger.c RemoteLoginClient/clientNet.c
# # CLIENT_SRC = $(CLIENT_DEPS)
# # CLIENT = RemoteLoginClient/remoteLoginClient.c

# # SERVER_DEPS = ResponseHandler/serverNet.c EventLogger/logger.c Settings/settings.c
# # OTHER_DEPS = Utilities/utilities.c Timer/timer.c LEDMFA/ledMfa.c DangerAnalyzer/dangerAnalyzer.c
# # SERVER_SRC = $(SERVER_DEPS) $(OTHER_DEPS)
# # SERVER = ResponseHandler/responseHandler.c

# # MENU
# #include "../Joystick/joystick.h"
# #include "../LEDDisplay/ledDisplay.h"
# #include "../PasswordInput/passwordInput.h"
# #include "../Timer/timer.h"
# #include "../Utilities/utilities.h"
# #include "../MotionSensor/motionSensor.h"
# #include "../Settings/settings.h"
# #include "../EventLogger/logger.h"
# #include "../DangerAnalyzer/dangerAnalyzer.h"

# MENU_DEPS = Timer/timer.c Utilities/utilities.c PasswordInput/passwordInput.c
# CAPE = Joystick/joystick.c MotionSensor/motionSensor.c LEDDisplay/ledDisplay.c
# ANALYZER = DangerAnalyzer/dangerAnalyzer.c DangerAnalyzer/CircularBuffer/circularBuffer.c
# OTHER_DEPS = Settings/settings.c EventLogger/logger.c
# MENU_SRC = $(MENU_DEPS) $(CAPE) $(ANALYZER) $(OTHER_DEPS)

# #################

# all:
# 	$(CC_C) $(CFLAGS) -pthread securitySensorSystem.c $(SOURCE) -o  $(OUTDIR)/$(OUTFILE) -lpthread


# wsl:
# 	# gcc $(CFLAGS) $(CLIENT) $(CLIENT_DEPENDENCIES) -o  ../build/client
# 	# gcc $(CFLAGS) $(SERVER) $(SERVER_DEPENDENCIES) -o  ../build/server
# 	# gcc $(CFLAGS) $(MENU) $(DEPENDENCIES) -o  ../build/menu

# 	# gcc $(CFLAGS) -pthread $(SERVER) $(SERVER_SRC) -o  ../build/server -lpthread
# 	# gcc $(CFLAGS) -pthread $(CLIENT) $(CLIENT_SRC) -o  ../build/client -lpthread

# 	$(CC_C) $(CFLAGS) -pthread Menu/menu.c $(MENU_SRC) -o  $(OUTDIR)/$(OUTFILE) -lpthread

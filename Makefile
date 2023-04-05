WEBCAM = WebCam/Recorder/Recorder.c WebCam/Stream/Stream.c WebCam/Stream/StreamController.c
MOTION = MotionSensor/motionSensor.c
UTILITIES = Utilities/utilities.c Timer/timer.c WebCam/Stream/Popen.c
PASSWORD = PasswordInput/passwordInput.c
JOYSTICK = Joystick/joystick.c

ANALYZER = DangerAnalyzer/dangerAnalyzer.c DangerAnalyzer/CircularBuffer/circularBuffer.c
HTTP = HttpRequest/http.c
LOGGER = EventLogger/logger.c
LED = LEDMFA/ledMfa.c
SEG_DISPLAY = LEDDisplay/ledDisplay.c
MENU = Menu/menu.c
PWMGR = PasswordManager/passwordManager.c
SERVER = ResponseHandler/responseHandler.c ResponseHandler/serverNet.c
SETTINGS = Settings/settings.c
CONTROLLER = Controller/controller.c

SOURCE1 = $(UTILITIES) ${MOTION} ${JOYSTICK} ${PASSWORD} ${WEBCAM} $(CONTROLLER)
SOURCE2 = $(ANALYZER) $(HTTP) $(LOGGER) $(LED) $(SEG_DISPLAY) $(MENU) $(PWMGR) $(SERVER) $(SETTINGS)
SOURCE = securitySensorSystem.c $(SOURCE1) $(SOURCE2)

OUTDIR = $(HOME)/cmpt433/public/myApps
OUTFILE = beaglecam

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
# CFLAGS = -Wall -g -std=c99 -Werror -D _POSIX_C_SOURCE=200809L -Wshadow
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Wshadow # TEMPORARY: Replace with commented code when testing is finished

LFLAGS = -L$(HOME)/cmpt433/public/curl_lib_BBB/lib/libs

all: bbg sshclient ethcamera

bbg:
	$(CC_C) $(CFLAGS) -pthread $(SOURCE) -o  $(OUTDIR)/$(OUTFILE) $(LFLAGS) -lpthread -lcurl
	@echo "Copying capture..."
	cp capture $(OUTDIR)
	@echo "Building node server..."
	sudo mkdir -p $(OUTDIR)/Server-copy/
	sudo cp -R Server/* $(OUTDIR)/Server-copy/
	cd $(OUTDIR)/Server-copy/ && sudo npm install

sshclient:
	@make --directory=RemoteLoginClient

ethcamera:
	@make --directory=Ethernet

src:
	$(CC_C) $(CFLAGS) -pthread $(SOURCE) -o  $(OUTDIR)/$(OUTFILE) $(LFLAGS) -lpthread

clean:
	rm -f *~ *.o $(OUTDIR)/$(OUTFILE)
	rm -f *~ *.o $(OUTDIR)/Server-Copy

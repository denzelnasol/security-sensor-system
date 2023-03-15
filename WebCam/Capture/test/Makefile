CC_C = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror
LFLAGS = -Llib/v4l2_lib
PUBDIR = $(HOME)/cmpt433/public/myApps

all:
	$(CC_C) $(CFLAGS) capture.c -o capture $(LFLAGS) -lv4l2
	cp capture $(PUBDIR)
	cp *.sh $(PUBDIR)

	# mkdir -p $(PUBDIR)/server-copy/
	# cp -R server/* $(PUBDIR)/server-copy/
	# cd $(PUBDIR)/server-copy/ && npm install

clean:
	rm -f capture
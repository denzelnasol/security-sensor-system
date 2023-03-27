# Security Sensor System

CMPT-433 - Security Sensor System

This is a Security Sensor System program created for CMPT-433. This repository is permitted to be public with the permission of Prof. Brian Fraser at the end of the Spring 2023 semester.

Work Flow Expectations:
- Make PRs only for dev branch (dev branches from master)
- Don't merge your own PRs
- Make a branch for every issue

Important Notes:
- Command to begin streaming data from web cam to web relay server:
    - ./capture -F -o -c0 | ffmpeg -i pipe:0 -f mpegts -codec:v mpeg1video -s 640x480 -b:v 1024k -bf 0 udp://192.168.7.1:8080
    - OR (likely better) ./capture -F -o -c0 | ffmpeg -i pipe:0 -f mpegts -codec:v mpeg1video -s 640x480 -b:v 4000k -minrate 4000k -maxrate 4000k -bufsize 1835k -muxdelay 0.1 -framerate 30 -bf 0 udp://192.168.7.1:8080

    - This command will retrieve the raw data, pass it to ffmpeg which will send it to our web relay server at 192.168.7.1:8080.
    - Once the server's started, it will listen to port 8080 and pass the stream data to the client browser.
    - The client can be opened at http://127.0.0.1:8081

- Command to stream web cam microphone (in-progress):
    - arecord -f cd -D hw:1,0 | ffmpeg -f s16le -ac 2 -ar 44100 -i /dev/stdin -codec:a mp2 -b:a 128k -f mpegts udp://192.168.7.1:8080

Team Members:
-------------
Andy C.
Mathew W.
Harry N.
Denzel N.


Required libraries for webcam:<br>
sudo apt-get install ffmpeg<br>
sudo apt-get install libopencv-dev<br>

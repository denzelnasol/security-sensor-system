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

How to deploy the node server through GCP 
Change IP Address for websocket to 
const WEBSOCKET_IP_ADDRESS = '10.128.0.2';
Change IP Address for udp_Server to 
const STREAM_IP_ADDRESS = '10.128.0.2';
Change nginx config, to access 'cd /etc/nginx/sites-available"
'sudo nano default'
Add following to top of the page
server {
  listen 80;
  server_name 34.123.31.151;

  location /port1 {
    proxy_pass "http://10.128.0.2:8080";
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection 'upgrade';
    proxy_cache_bypass $http_upgrade;
  }
  location /port2 {
    proxy_pass "http://10.128.0.2:8088";
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection 'upgrade';
    proxy_cache_bypass $http_upgrade;
  }
  location / {
    proxy_pass http://10.128.0.2:8088;
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection 'upgrade';
    proxy_cache_bypass $http_upgrade;
  }
}
To start the server head to the security-sensor-system/Server folder 
now type 'pm2 start server.js' to start or 'pm2 stop server.js' to stop

# Security Sensor System

CMPT-433 - Security Sensor System

This is a Security Sensor System program created for CMPT-433. This repository is permitted to be public with the permission of Prof. Brian Fraser at the end of the Spring 2023 semester.

Work Flow Expectations:
- Make PRs only for dev branch (dev branches from master)
- Don't merge your own PRs
- Make a branch for every issue

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

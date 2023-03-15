./capture -F -o -c0 | ffmpeg -i pipe:0 -f mpegts -codec:v mpeg1video -s 640x480 -b:v 512k -bf 0 udp://192.168.7.2:8080 &
cd skype-server-copy && node server.js
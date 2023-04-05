# how to configure beagle-cam ssh client on login

1. create user. The user is called 'pluto'
```
(bbg) sudo useradd -m pluto
(bbg) sudo passwd pluto
```

password can be anything. i used 'temppwd'

2. create shared folder kinda like nfs
```
(bbg) sudo groupadd groupa
(bbg) sudo mkdir /home/sharedFolder/
(bbg) sudo chgrp groupa /home/sharedFolder
(bbg) sudo chmod 777 /home/sharedFolder/
(bbg) sudo chmod +s /home/sharedFolder
(bbg) sudo usermod -a -G groupa debian
(bbg) sudo usermod -a -G groupa pluto
```

3. copy this script into /home/sharedFolder

launch.sh:
```
./ssh-client
if [ $? -eq 1 ]
then
	exit
fi
```

4. give permissions
```
(bbg) sudo chmod 777 /home/sharedFolder/launch.sh
```


5. (Optional) on your host make a script to ssh to beaglecam. go to step 6.

sshToBeagleCam.sh:
```
#!/bin/sh
sshpass -p temppwd ssh pluto@192.168.7.2
```

6. skip this step IF you skipped step 5. give permissions
```
(host) sudo chmod +x sshToBeagleCam.sh
```

7. connect to beaglecam with ssh
if you follow step 5 then the command is:
```
(host) ./sshToBeagleCam.sh
```
Otherwise the command is:
```
(host) ssh pluto@192.168.7.2
```

8. copy the launch.sh script and the ssh-client executable from /home/sharedFolder
```
(pluto) cp /home/sharedFolder/launch.sh .
(pluto) cp /home/sharedFolder/ssh-client .
```

9. add this line to the end of .profile for user pluto:
```
. ./launch.sh
```

10. test it out by ssh into beaglecam (pluto)!
- Test1: you will not be able to ctrl-c out of the program
- Test2: you will not be able to access the os if you are not logged in. it will close connection on you

11. the purpose of this is so that developers can connect through debian@192.168.7.2 and everyone else uses pluto@192.168.7.2. This is to ensure that we can always access the bbg over debian@192.168.7.2 and everyone else has either no access or restricted access

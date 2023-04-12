make sure that you have located dependencies for whatever implementation of
libcurl (commonly located in /usr/lib/arm-linux-gnueabihf on the target).  And 
then you have to copy it to /usr/arm-linux-gnueabihf/lib on the host for the
cross-compiler. Then you need to modify the makefile to propery link the 
libraries.

move everything in this folder to the nfs on the target: 
/mnt/remote/curl_lib_BBB

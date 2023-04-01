# configuration instructions for project

(bbg) sudo mkdir -p /var/lib/beagle-cam
(bbg) sudo mkdir -p /var/log/beagle-cam

(bbg) sudo touch /var/lib/beagle-cam/password.txt
(bbg) sudo touch /var/lib/beagle-cam/mssecret.txt
(bbg) sudo touch /var/lib/beagle-cam/settings.txt
(bbg) sudo touch /var/log/beagle-cam/events.log

(bbg) echo temppwd | sudo tee /var/lib/beagle-cam/password.txt
(bbg) echo 111 | sudo tee /var/lib/beagle-cam/mssecret.txt
(bbg) echo "0 1 60" | sudo tee /var/lib/beagle-cam/settings.txt


usr = `whoami`
sudo usermod -a -G plugdev $usr
sudo chmod 666 /dev/bus/usb/001/003
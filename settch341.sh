#bash
  stty -F /dev/ttyUSB0 cs8 9600 raw ignbrk noflsh -onlcr -iexten -echo -echoe -echok -echoctl -echoke -crtscts;#
#  echo "W1" >/dev/ttyUSB0;# WDT deactivate
#  echo "R1" >/dev/ttyUSB0;# RNG flood off
#  echo "C4" >/dev/ttyUSB0;# unlock DTR 

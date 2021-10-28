# Troubleshooting

Lack of permissions

Quick fix:
sudo chmod a+rw /dev/ttyUSB0

Better
add user to group sudo usermod -a -G dialout grzegorz

Add board to for wif1
https://arduino-esp8266.readthedocs.io/en/latest/installing.html

Remember to set Serial.begin(9600); In code and serial monitor

For the light sensor do not plug INT port

To get an IP address for MQTT, run `hostname -I` command. The first IP address is the right one.

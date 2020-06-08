# bbq-temperature
BBQ Temperature Measurement


### Setup - Arduino
- copy ```config.sample.h``` to ```config.h```
- edit ```config.h``` (edit the SSID and password to match your wifi)
- Upload to Arduino UNO WiFi rev 2 with KTA-259K thermocouple multiplexer shield from electronics123.com

### Setup - Node
- ```npm install```
- ```cp config.sample.json config.json``` and edit with the IP address of the arduino (it will print this to the serial port after connecting)
- ```npm run main```
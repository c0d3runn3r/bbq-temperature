#include <string.h>
#include <ctype.h>
#include <WiFiNINA.h>
#include <SPI.h>
#include "config.h"

// DEFINEs from the TCMux shield demo by Ocean Controls
#define PINEN 7 //Mux Enable pin
#define PINA0 4 //Mux Address 0 pin
#define PINA1 5 //Mux Address 1 pin
#define PINA2 6 //Mux Address 2 pin
#define PINSO 12 //TCAmp Slave Out pin (MISO)
#define PINSC 13 //TCAmp Serial Clock (SCK)
#define PINCS 9  //TCAmp Chip Select Change this to match the position of the Chip Select Link 
#define ERR_VCC		1	// Probe shorted to Vcc
#define ERR_GND		2	// Probe shorted to GND
#define ERR_OPEN	3	// Probe is open
#define ERR_INVALID	4	// Invalid probe index

char ssid[]=WIFI_SSID;
char password[]=WIFI_PASSWORD;


float temperature[8];
char status[8];
unsigned long time;
char probe_index=0;
WiFiServer server(80);

void setup() {
  // put your setup code here, to run once:
	Serial.begin(9600);
	setup_probes();
	setup_server();
}

void loop() {

	// Run the following code every 125ms so we get a full probe refresh every 1s.  Should be relatively overflow-safe, though we may get a short cycle (<125ms) at overflow time.
	unsigned long diff=millis()-time;
	if(diff >= 125) {

		// Reset the time
		time=millis();

		// Read a probe
		status[probe_index]=read_probe(probe_index, &temperature[probe_index]);

		// Increment
		probe_index++;
		if(probe_index >7) probe_index=0;
	}

	serve_requests();



}

void setup_server() {

	if(WiFi.status() == WL_NO_MODULE) {

		Serial.println("No wifi module detected");
	}

	// Connect to wifi
	int status=0;
	Serial.print("Connecting to wifi '");
	Serial.print(ssid);
	Serial.print("'...");
	status=WiFi.begin(ssid, password);

	// Check status every 100ms until connected
	unsigned long t=millis();
	do {
		unsigned long diff=millis()-t;
		if(diff >=100) {
			Serial.print(".");
			t=millis();
			status=WiFi.status();
		}
		
	} while(status != WL_CONNECTED);
	Serial.println("connected");

	// Print out wifi details
	Serial.print("IP Address: ");
	Serial.println(WiFi.localIP());
	Serial.print("RSSI: ");
	Serial.print(WiFi.RSSI());
	Serial.println("dBm");


	// Start the server
	server.begin();
}


/**
 * Serve requests
 * 
 * Parts of this have been copied from the WiFi server example by dlf and Tom Igoe
 */
void serve_requests() {

	char n;

	// See if we have a client waiting to be served
	WiFiClient client=server.available();
	if(!client) return;

	Serial.print("Serving client...");
	boolean currentLineIsBlank = true;
    while (client.connected()) {
    	
    	if (client.available()) {
    		
        	char c = client.read();
        	//Serial.write(c);
        	
        	// if you've gotten to the end of the line (received a newline
       		// character) and the line is blank, the http request has ended,
        	// so you can send a reply
        	if (c == '\n' && currentLineIsBlank) {
          
          		// send a standard http response header
          		client.println("HTTP/1.1 200 OK");
          		client.println("Content-Type: application/json");
          		client.println("Connection: close");  // the connection will be closed after completion of the response
          		client.println();
          		client.println("[");

				for(n=0; n< 8; n++) {
			
					client.print("{ \"status\" : ");
					switch(status[n]) {
				
						case ERR_VCC:
							client.print("\"vcc\"");
							break;
						case ERR_GND:
							client.print("\"gnd\"");
							break;
						case ERR_OPEN:
							client.print("\"open\"");
							break;
						case 0:
							client.print("\"ok\"");
							break;
					}
	
					if(status[n] ==0 ) {
	
						client.print(", \"temperature\" : ");
						client.print(temperature[n]);
					}
		
					client.print("}");
					if(n!=7) client.println(",");
				}
				client.println("]");



         		break;
        	}
        	
			if (c == '\n') {
          		
          		// you're starting a new line
				currentLineIsBlank = true;
				
        	} else if (c != '\r') {
        		
          		// you've gotten a character on the current line
          		currentLineIsBlank = false;
        	}
      	}
    }	

	client.stop();
    Serial.println("ok");

}




/** 
 *  Set up the temperature controls
 *  
 *  Parts of this function were copied from the TCMux shield demo by Ocean Controls
 */
void setup_probes() {

	pinMode(PINEN, OUTPUT);     
	pinMode(PINA0, OUTPUT);    
	pinMode(PINA1, OUTPUT);    
	pinMode(PINA2, OUTPUT);    
	pinMode(PINSO, INPUT);    
	pinMode(PINCS, OUTPUT);    
	pinMode(PINSC, OUTPUT);    
	
  digitalWrite(PINEN, HIGH);   // enable on
  digitalWrite(PINA0, LOW); // low, low, low = channel 1
  digitalWrite(PINA1, LOW); 
  digitalWrite(PINA2, LOW); 
  digitalWrite(PINSC, LOW); //put clock in low
  
}

/**
 * Read a probe
 * 
 * Parts of this function were copied from the TCMux shield demo by Ocean Controls
 * 
 * @param {int} index the probe index
 * @param {float*} temperature a pointer to a float where the temperature should be stored
 * @return {char} error the error code, or 0 for success
 */
char read_probe(int index, float* temperature) {

	int temp=0;
	bool sensor_fail=false;
	char fail_mode=0;
	unsigned int mask=0;
	char i;

    // Tell the mux which probe we are reading
	switch (index) 
	{
		case 0:
		digitalWrite(PINA0, LOW); 
		digitalWrite(PINA1, LOW); 
		digitalWrite(PINA2, LOW);
		break;
		case 1:
		digitalWrite(PINA0, HIGH); 
		digitalWrite(PINA1, LOW); 
		digitalWrite(PINA2, LOW);
		break;
		case 2:
		digitalWrite(PINA0, LOW); 
		digitalWrite(PINA1, HIGH); 
		digitalWrite(PINA2, LOW);
		break;
		case 3:
		digitalWrite(PINA0, HIGH); 
		digitalWrite(PINA1, HIGH); 
		digitalWrite(PINA2, LOW);
		break;
		case 4:
		digitalWrite(PINA0, LOW); 
		digitalWrite(PINA1, LOW); 
		digitalWrite(PINA2, HIGH);
		break;
		case 5:
		digitalWrite(PINA0, HIGH); 
		digitalWrite(PINA1, LOW); 
		digitalWrite(PINA2, HIGH);
		break;
		case 6:
		digitalWrite(PINA0, LOW); 
		digitalWrite(PINA1, HIGH); 
		digitalWrite(PINA2, HIGH);
		break;
		case 7:
		digitalWrite(PINA0, HIGH); 
		digitalWrite(PINA1, HIGH); 
		digitalWrite(PINA2, HIGH);
		break;
		default:
		return ERR_INVALID;
	}

      // Convert a temperature
	delay(5);
    digitalWrite(PINCS, LOW); //stop conversion
    delay(5);
    digitalWrite(PINCS, HIGH); //begin conversion
    delay(100);  //wait 100 ms for conversion to complete
    digitalWrite(PINCS, LOW); //stop conversion, start serial interface
    delay(1);

      // Read 32 bits of data data from the chip
	for (i=31;i>=0;i--) {
		
      	digitalWrite(PINSC, HIGH);
      	delay(1);

      	// Handle thermocouple bits
		if ((i<=31) && (i>=18)) {
		
			// bit 31 sign
			// bit 30 MSB = 2^10
			// bit 18 LSB = 2^-2 (0.25 degC)
			mask = 1<<(i-18);
			if (digitalRead(PINSO)==1) {
			
				if (i == 31) {
			    	temp += (0b11<<14);//pad the temp with the bit 31 value so we can read negative values correctly
				}
			
				temp += mask;
			}
	
		}
	
	    //bit 17 is reserved
	    //bit 16 is sensor fault

	    // Thermocouple fail
	    if (i==16) {
	    	
	    	sensor_fail = digitalRead(PINSO);
	    }

		// Ignore the 12 bits for the internal temp of the chip, [15..4]
    
		// Bit 3 is reserved

		// Read circuit error bits
		if (i==2) {
		
			fail_mode += digitalRead(PINSO)<<2;//bit 2 is set if shorted to VCC
      	}

 		if (i==1) {
 			
          fail_mode += digitalRead(PINSO)<<1;//bit 1 is set if shorted to GND
      	}
      
      	if (i==0) {
      		
          fail_mode += digitalRead(PINSO)<<0;//bit 0 is set if open circuit
      	}
      
		digitalWrite(PINSC, LOW);
		delay(1);
  	}

	if (sensor_fail == 1) {
	
		if ((fail_mode & 0b0100) == 0b0100) {
	
			return ERR_VCC; // Short to Vcc
		}
		if ((fail_mode & 0b0010) == 0b0010) {
			
			return ERR_GND; // Short to gnd
		}
		if ((fail_mode & 0b0001) == 0b0001) {
	
			return ERR_OPEN; // Open circuit
		}
	
	} else {

		// No error, return the temperature
		*temperature = (float)temp * 0.25;
		return 0;
	
	}
}

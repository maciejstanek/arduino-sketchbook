// Headers {{{
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <SPI.h>
#include <Ethernet.h>
// Headers }}}
// Settings {{{
// Relays {{{
byte relay_count = 3;
byte relay[3];
byte relay_pins[3] = {7, 8, 9};
void applyRelaySettings() {
	for(byte i = 0; i < relay_count; i++) {
		digitalWrite(relay_pins[i], relay[i]);
	}
}
// Relays }}}
// Ethernet {{{
byte thisMAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress serverIP(192, 168, 1, 170);
IPAddress thisIP(192,168,1,171);
EthernetClient client;
EthernetServer localServer(80);
// Ethernet }}}
// PIR {{{
byte pin_pir = 2;
boolean pir_state_at_interrupt;
boolean pir_interrupt_flag;
void pir_interrupt_coroutine() {
	pir_state_at_interrupt = digitalRead(pin_pir);
	pir_interrupt_flag = true;
}
// PIR }}}
Adafruit_BMP085 bmp;
// Settings }}}
// Initialization {{{
void setup() {
	Serial.begin(9600);
	// PIR
	pir_interrupt_flag = false;
	pinMode(pin_pir, INPUT);
	attachInterrupt(digitalPinToInterrupt(2), pir_interrupt_coroutine, CHANGE);
	// Ethernet
	Ethernet.begin(thisMAC, thisIP);
	localServer.begin();
	delay(1000);
	Serial.print("--DBG-- Configured ethernet at ");
	Serial.println(Ethernet.localIP());
	// BMP180
	if(!bmp.begin()) {
		Serial.print("--DBG-- Error while initializing BMP180, halting... ");
		for(;;);
	}
	// Relays
	for(byte i = 0; i < relay_count; i++) {
		pinMode(relay_pins[i], OUTPUT);
	}
	relay[0] = LOW;
	relay[1] = LOW;
	relay[2] = LOW;
	applyRelaySettings();
}
// Initialization }}}
// Main loop {{{
void loop() {
	// Handle PIR interrupt {{{
	if(pir_interrupt_flag) {
		Serial.print("--DBG-- PIR status changed to ");
		Serial.println(pir_state_at_interrupt);
		pir_interrupt_flag = false;

		if(client.connect(serverIP, 80)) { // port 80 is default for HTTP
			Serial.println("--DBG-- Sending PIR status succeeded!");
			client.println("POST /mstanek/home_automation_server/php/arduino.php HTTP/1.1");
			client.println("Host: 192.168.1.170");
			client.println("Content-Type: application/x-www-form-urlencoded");
			client.println("Content-Length: 5");
			client.println("Connection: close");
			client.println();
			client.print("pir=");
			client.println(pir_state_at_interrupt);

			Serial.println("--DBG-- This is a response for this request:");
			while(client.connected()) {
				while(client.available()) {
					char c = client.read();
					Serial.print(c);
				}
			}
			Serial.println("");
			Serial.println("--DBG-- Closing the PIR request...");
			client.stop();
		} 
		else {
			Serial.println("--DBG-- But sending PIR status failed!");
		}
	}
	// Handle PIR interrupt }}}
	// Process server requests {{{
	if(client = localServer.available()) {
		Serial.println("--DBG-- New incoming connection with a following HTTP header:");
		String line; // This variable will contain the last line - the POST request body
		while(client.available()) {
				char c = client.read();
				line += c;
				if(c == '\n') {
					line = "";
				}
				Serial.write(c);
		}
		Serial.println("");
		Serial.print("--DBG-- Got the following request: '");
		Serial.print(line);
		Serial.println("'");
		// Parese command and Drive the relays accordingly
		bool badRequest = true;
		if(line.length() > 2 && line[0] == 'Q' && line[1] == '=') {
			client.println();
			switch(line[2]) {
				// 'G' request {{{
				case 'G':
					if(line.length() == 3) {
						// Send a standard HTTP response header
						client.println("HTTP/1.1 200 OK");
						client.println("Content-Type: application/json");
						client.println("Connection: close");
						client.println();
						// Read sensors
						int32_t pressure;
						pressure = bmp.readPressure();
						Serial.print("--DBG-- Pressure: ");
						Serial.println(pressure);
						float temperature;
						temperature = bmp.readTemperature();
						Serial.print("--DBG-- Temperature: ");
						Serial.println(temperature);
						// Print sensors and relays values
						client.print("{\"t\":\"");
						client.print(temperature);
						client.print("\",\"p\":\"");
						client.print(pressure);
						client.print("\",\"pir\":\"");
						client.print(pir_state_at_interrupt);
						client.print("\",\"r\":[");
						for(byte i = 0; i < relay_count; i++) {
							if(i) {
								client.print(",");	
							}
							client.print("\"");
							client.print(relay[i]);
							client.print("\"");
						}
						client.print("]}");
						badRequest = false;
					}
					break;
				// 'G' request }}}
				// 'R[1-3][UDT]' request {{{
				case 'R':
					if (line.length() == 5 && line[3] >= '1' && line[3] < '1' + relay_count) {
						// Parse the command
						byte relay_index = line[3] - '1';	
						Serial.print("--DBG-- Relay #");
						Serial.println(relay_index + 1);
						switch(line[4]) {
							case 'U':
								Serial.println("--DBG-- Turning on relay");
								badRequest = false;
								relay[relay_index] = HIGH;
								break;
							case 'D':
								Serial.println("--DBG-- Turning off relay");
								badRequest = false;
								relay[relay_index] = LOW;
								break;
							case 'T':
								Serial.println("--DBG-- Toggle relay");
								badRequest = false;
								relay[relay_index] ^= 1;
								break;
						} 
						applyRelaySettings();
						if(!badRequest) {
							// Response
							Serial.print("--DBG-- Resulting relay status is ");
							Serial.println(relay[relay_index]);
							client.println("HTTP/1.1 200 OK");
							client.println("Content-Type: text/plain");
							client.println("Connection: close");
							client.println();
							client.print(relay[relay_index]);
						}
					}
					break;
				// 'R[1-3][UDT]' request }}}
			}
		}
		if(badRequest) {
			// Send a response for a bad query
			Serial.println("--DBG-- This request was a bogus!");
			client.println("HTTP/1.1 400 Bad Request");
			client.println("Connection: close");
			client.println();
		}
		// Close connection
		delay(10);
		client.stop();
		Serial.println("--DBG-- Incoming connection stopped");
	}
	// Process server requests }}}
}
// Main loop }}}

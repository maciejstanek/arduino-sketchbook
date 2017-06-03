// Headers {{{
#include <SPI.h>
#include <Ethernet.h>
// Headers }}}
// Settings {{{
// Information sent by the request {{{
int status_temperature;
int status_pressure;
boolean status_relay[3];
// Information sent by the request }}}
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
		Serial.println("--DBG-- New incoming connection");
		// An HTTP request ends with a blank line
		boolean currentLineIsBlank = true;
		while (client.connected()) {
			if (client.available()) {
				char c = client.read();
				Serial.write(c);
				// If you've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				if (c == '\n' && currentLineIsBlank) {
					// Send a standard HTTP response header
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: application/json");
					client.println("Connection: close");
					client.println();
					// TODO: Print sensors values
					client.println("{\"a\":1}");
					break;
				}
				if (c == '\n') {
					// You're starting a new line
					currentLineIsBlank = true;
				} else if (c != '\r') {
					// You've gotten a character on the current line
					currentLineIsBlank = false;
				}
			}
		}
		delay(10);
		client.stop();
		Serial.println();
		Serial.println("--DBG-- Incoming connection stopped");
	}
	// Process server requests }}}
}
// Main loop }}}

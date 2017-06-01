#include <SPI.h>
#include <Ethernet.h>

// Information sent by the request
int status_temperature;
int status_pressure;
boolean status_relay[3];
boolean status_pir;

// Pinout
byte pin_pir = 2;

// Network
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress server(192, 168, 1, 170);
IPAddress ip(192,168,1,171);
EthernetClient client;

// PIR
boolean pir_state_at_interrupt;
boolean pir_interrupt_flag;
void pir_interrupt_coroutine() {
  // TODO: ethernet client
  pir_state_at_interrupt = digitalRead(pin_pir);
  pir_interrupt_flag = true;
}

// Setup
void setup() {
  Serial.begin(9600);
  
  pir_interrupt_flag = false;
  
  pinMode(pin_pir, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), pir_interrupt_coroutine, CHANGE);
  
  Ethernet.begin(mac, ip);
  Serial.println("Connecting to the host...");
  delay(1000);
}

// Loop
void loop() {

  // Handle PIR interrupt
  if(pir_interrupt_flag) {
    Serial.print("PIR status changed to ");
    Serial.println(pir_state_at_interrupt);
    pir_interrupt_flag = false;

    if(client.connect(server, 80)) { // port 80 is default for HTTP
      Serial.println("Sending PIR status succeeded!");
      client.println("POST /mstanek/home_automation_server/php/arduino.php HTTP/1.1");
      client.println("Host: 192.168.1.170");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.println("Content-Length: 5");
      client.println("Connection: close");
      client.println();
      client.print("pir=");
      client.println(pir_state_at_interrupt);
    } 
    else {
      Serial.println("But sending PIR status failed!");
    }
    
    Serial.println("This is a response for this request:");
    for(int i = 0; i < 10; i++) {
      while(client.available()) {
        char c = client.read();
        Serial.print(c);
      }
      delay(100);
    }
    
    while(client.connected()) {
      Serial.println("Closing the PIR request...");
      client.stop();
    }
  }
}


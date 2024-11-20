// INTERMEDIARY NODE - SERVER 1 (S1)
// TOPOLOGY:
// C - S1 - S3 (C and S3 has no direct connection/route)

// rf95_router_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, routed reliable messaging client
// with the RHRouter class.
// It is designed to work with the other examples rf95_router_server*

#include <RHSoftwareSPI.h>
RHSoftwareSPI spi;
#include <RHRouter.h>
#include <RH_RF95.h>
#include <SPI.h>

int led_g = 3;
int led_r = 12;

// In this small artifical network of 5 nodes,
// messages are routed via intermediate nodes to their destination
// node. All nodes can act as routers
// CLIENT_ADDRESS <-> SERVER1_ADDRESS <-> SERVER2_ADDRESS<->SERVER3_ADDRESS <-> SERVER4_ADDRESS
#define CLIENT_ADDRESS 1
#define SERVER1_ADDRESS 2
#define SERVER2_ADDRESS 3
#define SERVER3_ADDRESS 4
#define SERVER4_ADDRESS 5

/*// Arduino
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2 //*/

// ESP32
#define RFM95_CS 5
#define RFM95_RST 14
#define RFM95_INT 2 //*/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHRouter *manager;
int count = 0;

void setup() 
{
  pinMode(led_g, OUTPUT); 
  pinMode(led_r, OUTPUT); 

  Serial.begin(9600);

  spi.setPins(11, 9, 7);

  manager = new RHRouter(driver, SERVER1_ADDRESS);

  if (!manager->init()){
    Serial.println("init failed");
    digitalWrite(led_r, HIGH);
    delay(10);}
    else{
    led_G();
    }
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  
  driver.setTxPower(23, false);
  driver.setFrequency(RF95_FREQ);
  driver.setCADTimeout(0);
  driver.setSpreadingFactor(9);
  driver.setSignalBandwidth(250E3);

  // Manually define the routes for this network
  manager->addRouteTo(CLIENT_ADDRESS, CLIENT_ADDRESS);
  //manager->addRouteTo(SERVER2_ADDRESS, SERVER2_ADDRESS);    
  //manager->addRouteTo(SERVER2_ADDRESS, SERVER2_ADDRESS);
  //manager->addRouteTo(SERVER3_ADDRESS, CLIENT_ADDRESS);
//  manager->addRouteTo(SERVER4_ADDRESS, SERVER4_ADDRESS);
  
  Serial.println("Router Server " + (String) (SERVER1_ADDRESS-2) + ": Up and Running");
  led_G();
}

uint8_t data[] = "And hello back to you from server3";
// Dont put this on the stack:
uint8_t buf[RH_ROUTER_MAX_MESSAGE_LEN];

void loop()
{
  uint8_t len = sizeof(buf);
  uint8_t from;
  if (manager->recvfromAck(buf, &len, &from))
  {
    Serial.print("got request from : 0x");
    Serial.print(from, HEX);
    Serial.print(": ");
    Serial.println((char*)buf);
    count++;
    Serial.println("Count Recieved: " + (String) count + "/1000");
    led_G();

    // Send a reply back to the originator client
    if (manager->sendtoWait(data, sizeof(data), from) != RH_ROUTER_ERROR_NONE)
    {
      Serial.println("sendtoWait failed");
      led_R();
    }
      
  }
}

void led_G(){
    digitalWrite(led_g, HIGH);
    delay(100);
    digitalWrite(led_g, LOW);
    delay(100);
}

void led_R(){
    digitalWrite(led_r, HIGH);
    delay(100);
    digitalWrite(led_r, LOW);
    delay(100);
}

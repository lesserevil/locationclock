#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "FBI Surveillance Van"           // cannot be longer than 32 characters!
//#define WLAN_SSID       "CIA Black Helicopter"           // cannot be longer than 32 characters!
#define WLAN_PASS       ""
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define LISTEN_PORT           934    // What TCP port to listen on for connections.  The echo protocol uses port 7.

#define NUM_OF_HANDS 4  //number of hands on the clock
#define DEFAULT_POSITION 0 //The position the clock should go to at boot

Adafruit_CC3000_Server clockServer(LISTEN_PORT);

#define MIN_PWM 121
#define MAX_PWM 620
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

uint8_t hand_pos[NUM_OF_HANDS];

void setup(void)
{
  Serial.begin(115200);
  Serial.println(F("Hello, CC3000!\n"));

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);

  setupHands();

  for (int i = 1; i <= NUM_OF_HANDS; i++) {
    moveHands(i, DEFAULT_POSITION);
  }

  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while (1);
  }

  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while (1);
  }

  Serial.println(F("Connected!"));

  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }

  /* Display the IP address DNS, Gateway, etc. */
  while (! displayConnectionDetails()) {
    delay(1000);
  }

  delay(5000);

  // Start listening for connections
  clockServer.begin();

  delay(5000);

  Serial.println(F("Listening for connections..."));
}

unsigned int cPerson = 0;
unsigned int cPosition = 0;

void loop(void)
{
  // Try to get a client which is connected.
  Adafruit_CC3000_ClientRef client = clockServer.available();

  if (client) {
    Serial.println("Got a client...");
    while (client.available() > 0) {
      unsigned int tmp = client.read();
      //Is this a number?  If so, the it is the 'person'.
      if (tmp >= 49 && tmp <= 48 + NUM_OF_HANDS) {
        cPerson = tmp - 48;
      }
      if (tmp >= 65 && tmp <= 65 + 12) {
        cPosition = tmp - 64;
      }
      if (cPerson && cPosition) {
        moveHands(cPerson, cPosition);
        cPerson = 0;
        cPosition = 0;
      }
    }
  }
}

void moveHands(unsigned int person, unsigned int pos) {
  Serial.print("Move hand ");
  Serial.print(person);
  Serial.print(" to position ");
  Serial.print(pos);
  Serial.println(".");

  int curpos = map(hand_pos[person-1],0,12,MIN_PWM,MAX_PWM);
  int newpos = map(pos,0,12,MIN_PWM,MAX_PWM);
  int step = (curpos < newpos) ? 1 : -1;
  for (int i = curpos; i != newpos; i += step) {
    pwm.setPWM(person-1,0,i);
    delay(50);
  }
  hand_pos[person-1] = pos;
}

void setupHands() {
  pwm.begin();
  pwm.setPWMFreq(60);
  
  for (int i = 0; i <NUM_OF_HANDS; i++) {
    hand_pos[i]=0;
  }
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if (!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();

    uint32_t octet = ipAddress & 0x000000FF;
    uint8_t digit1 = octet / 100;
    uint8_t digit2 = octet / 10 - digit1 * 10;
    uint8_t digit3 = octet - (digit2 * 10) - (digit1 * 100);
    moveHands(1, digit1);
    moveHands(2, digit2);
    moveHands(3, digit3);

    return true;
  }
}

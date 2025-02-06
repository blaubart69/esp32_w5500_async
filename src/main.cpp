#include <Arduino.h>

#define ESP32_Ethernet_onEvent            ESP32_W5500_onEvent
#define ESP32_Ethernet_waitForConnect     ESP32_W5500_waitForConnect

#include <AsyncUDP_ESP32_Ethernet.h>

void initEthernet()
{
  UDP_LOGWARN(F("Default SPI pinout:"));
  UDP_LOGWARN1(F("SPI_HOST:"), ETH_SPI_HOST);
  UDP_LOGWARN1(F("MOSI:"), MOSI_GPIO);
  UDP_LOGWARN1(F("MISO:"), MISO_GPIO);
  UDP_LOGWARN1(F("SCK:"),  SCK_GPIO);
  UDP_LOGWARN1(F("CS:"),   CS_GPIO);
  UDP_LOGWARN1(F("INT:"),  INT_GPIO);
  UDP_LOGWARN1(F("SPI Clock (MHz):"), SPI_CLOCK_MHZ);
  UDP_LOGWARN(F("========================="));

  ///////////////////////////////////

  // To be called before ETH.begin()
  ESP32_Ethernet_onEvent();

  // start the ethernet connection and the server:
  // Use DHCP dynamic IP and random mac
  //bool begin(int MISO_GPIO, int MOSI_GPIO, int SCLK_GPIO, int CS_GPIO, int INT_GPIO, int SPI_CLOCK_MHZ,
  //           int SPI_HOST, uint8_t *W6100_Mac = W6100_Default_Mac);
  ETH.begin( MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST );
  //ETH.begin( MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac[millis() % NUMBER_OF_MAC] );

  // Static IP, leave without this line to get IP via DHCP
  //bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
  //ETH.config(myIP, myGW, mySN, myDNS);

  ESP32_Ethernet_waitForConnect();

  ///////////////////////////////////
}

AsyncUDP udp;

void parsePacket(AsyncUDPPacket packet)
{
  Serial.print("UDP Packet Type: ");
  Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
  Serial.print(", From: ");
  Serial.print(packet.remoteIP());
  Serial.print(":");
  Serial.print(packet.remotePort());
  Serial.print(", To: ");
  Serial.print(packet.localIP());
  Serial.print(":");
  Serial.print(packet.localPort());
  Serial.print(", Length: ");
  Serial.print(packet.length());
  Serial.print(", Data: ");
  Serial.write(packet.data(), packet.length());
  Serial.println();
  //reply to the client
  packet.printf("Got %u bytes of data", packet.length());
}

void setup() {
	Serial.begin(230400);
	Serial.println("bring up ethernet...");
	initEthernet();
	Serial.printf("ethernet is up. millis passed: %"  PRIu32 "\n", millis());
	Serial.println(ETH.localIP());
	Serial.println(ETH.subnetMask());
	Serial.println(ETH.gatewayIP());
	Serial.println(ETH.dnsIP());

	Serial.print("sizeof(long):   "); Serial.println(sizeof(long));
	Serial.print("sizeof(int):    "); Serial.println(sizeof(int));
	Serial.print("sizeof(float):  "); Serial.println(sizeof(float));
	Serial.print("sizeof(double): "); Serial.println(sizeof(double));

	udp.onPacket([](AsyncUDPPacket packet)
    {
      parsePacket( packet);
    });
}

void loop() {
	delay(1000);
}


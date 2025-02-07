#include <Arduino.h>

#define ESP32_Ethernet_onEvent ESP32_W5500_onEvent
#define ESP32_Ethernet_waitForConnect ESP32_W5500_waitForConnect

#include <AsyncUDP_ESP32_Ethernet.h>

void initEthernet()
{
  UDP_LOGWARN(F("Default SPI pinout:"));
  UDP_LOGWARN1(F("SPI_HOST:"), ETH_SPI_HOST);
  UDP_LOGWARN1(F("MOSI:"), MOSI_GPIO);
  UDP_LOGWARN1(F("MISO:"), MISO_GPIO);
  UDP_LOGWARN1(F("SCK:"), SCK_GPIO);
  UDP_LOGWARN1(F("CS:"), CS_GPIO);
  UDP_LOGWARN1(F("INT:"), INT_GPIO);
  UDP_LOGWARN1(F("SPI Clock (MHz):"), SPI_CLOCK_MHZ);
  UDP_LOGWARN(F("========================="));

  ///////////////////////////////////

  // To be called before ETH.begin()
  ESP32_Ethernet_onEvent();

  // start the ethernet connection and the server:
  // Use DHCP dynamic IP and random mac
  // bool begin(int MISO_GPIO, int MOSI_GPIO, int SCLK_GPIO, int CS_GPIO, int INT_GPIO, int SPI_CLOCK_MHZ,
  //           int SPI_HOST, uint8_t *W6100_Mac = W6100_Default_Mac);
  ETH.begin(MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST);
  // ETH.begin( MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac[millis() % NUMBER_OF_MAC] );

  // Static IP, leave without this line to get IP via DHCP
  // bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
  // ETH.config(myIP, myGW, mySN, myDNS);
  
  ESP32_Ethernet_waitForConnect();
  ETH.enableIpV6();
  Serial.println("ETH.enableIpV6()");
  
  ///////////////////////////////////
}

AsyncUDP udp;
AsyncUDP udp_multi;
AsyncUDP udpv6;

void parsePacket(AsyncUDPPacket packet)
{
  Serial.print("UDP Packet Type: ");
  Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast"
                                                                         : "Unicast");
  Serial.print(", From: ");
  if ( packet.isIPv6() ) {
    Serial.print(packet.remoteIPv6());  
  }
  else {
    Serial.print(packet.remoteIP());
  }
  Serial.print(":");
  Serial.print(packet.remotePort());
  Serial.print(", To: ");

  if ( packet.isIPv6() ) {
    Serial.print(packet.localIPv6());  
  }
  else {
    Serial.print(packet.localIP());
  }

  Serial.print(":");
  Serial.print(packet.localPort());

  char str[128];
  memcpy(str, packet.data(), packet.length());
  str[packet.length()] = '\0';
  

  Serial.printf(", Length: %lu, data: %s\n", packet.length(), str);
  // Serial.print(", Data: ");
  // Serial.write(packet.data(), packet.length());
  // Serial.println();
  // reply to the client
  // packet.printf("Got %u bytes of data", packet.length());
}

void every(void* param) {
  
  auto ipv6_empty = IPv6Address();
  auto ipv6_last = ipv6_empty;

  delay(1000);

  for(;;) {
    delay(3000);

    auto ipv6_curr = ETH.localIPv6();
    if ( !(ipv6_last == ipv6_curr) ) {
      Serial.print("got localIPv6(): "); Serial.println(ipv6_curr);
      ipv6_last = ipv6_curr;
    }
    else if ( ipv6_curr == ipv6_empty ) {
      ipv6_last = ipv6_empty;
      ETH.enableIpV6();
      Serial.println("ETH.enableIpV6()");
    }
  }
  vTaskDelete(NULL);
}

void setup()
{
  Serial.begin(230400);
  initEthernet();
  Serial.printf("ethernet is up. millis passed: %" PRIu32 "\n", millis());
  Serial.printf("hello from core: %lu\n", xPortGetCoreID());
  Serial.println(ETH.localIP());
  Serial.println(ETH.subnetMask());
  Serial.println(ETH.gatewayIP());
  Serial.println(ETH.dnsIP());

  udp.listen(5122);
  udp.listenIPv6();
  udp.onPacket([](AsyncUDPPacket packet)
               { parsePacket(packet); 
               });
  
  udp_multi.listenMulticast(IPAddress(224,0,0,5), 5123);
  udp_multi.onPacket([](AsyncUDPPacket packet)
               { Serial.println("Multipass!");
               });
  
  char  param[] = "Hello from outside";
  TaskHandle_t hEveryTask = NULL;
  xTaskCreatePinnedToCore(every, "every", 8192, (void*)param, tskIDLE_PRIORITY, &hEveryTask, 1);
  if ( hEveryTask == NULL) {
    Serial.println("every task could not be started");
  }
}

void loop()
{
  delay(3000);
}

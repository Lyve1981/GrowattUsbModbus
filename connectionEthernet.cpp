#include "connectionEthernet.h"

#include "UIPEthernet.h"

#include "debug.h"

/*
	Wiring for ENC28J60 to NodeMCU Lolin V3 CH340

	ENC ... NodeMCU

    GND ... GND
    VCC ... 3V3
  RESET ... RST
     CS ... D0 (GPIO16, WAKE)
    SCK ... D5 (GPIO14, HSPI SCLK)
     SO ... D6 (GPIO12, HSPI MISO)
     SI ... D7 (GPIO13, HSPI MOSI)
*/

constexpr int PIN_CS = D0;

constexpr uint8_t g_ethMac[] = {0x5e,0xc0,0xde,0xba,0x5e,0x01};

bool ConnectionEthernet::initialize()
{
	debug("Initialize Ethernet, CS Pin = ");
	debugln(String(PIN_CS));

	Ethernet.init(PIN_CS);
	if(!Ethernet.begin(g_ethMac))
	{
		debugln("Ethernet.begin() failed");
		if(Ethernet.hardwareStatus() == EthernetNoHardware)
		{
			debugln("No Ethernet hardware found");
			return false;
		}
		debugln("Failed to get address via DHCP");
		return false;
	}

	debug("My IP: ");
    debugln(Ethernet.localIP().toString());

	return Connection::initialize();
}

Client* ConnectionEthernet::getClient()
{
	return new EthernetClient();
}

void ConnectionEthernet::loop()
{
	Ethernet.maintain();
}

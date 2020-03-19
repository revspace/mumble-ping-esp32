#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "revspace-pub-2.4ghz";
const char* password = "";

const char* address = "revspace.nl";
const int port = 64738;

WiFiUDP udp;

struct __attribute__ ((packed)) mumbleRequest {
	unsigned int command;
	unsigned long long ident;
};

byte reqBuffer[12];
char respBuffer[24];

void setup() {
	Serial.begin(115200);
	WiFi.begin(ssid, password);
	Serial.println();
	while (WiFi.status() != WL_CONNECTED) {
		delay(100);
		Serial.print(".");
	}
}

void loop() {
	mumbleRequest request = {
		.command = 0,
		.ident = millis()
	};

	memcpy(reqBuffer, &request, sizeof(request));
	udp.beginPacket(address, port);
	udp.write(reqBuffer, 12);
	udp.endPacket();

	udp.parsePacket();
	udp.read(respBuffer, 24);
	// 4 bytes version
	// 8 bytes ident
	// 4 bytes connected
	// 4 bytes maximum connections
	// 4 bytes bandwidth
	int connected = respBuffer[15];
	Serial.print(connected);
	Serial.println(" people are connected");
	delay(100);
}

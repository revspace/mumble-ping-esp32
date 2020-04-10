#if defined(ARDUINO_ARCH_ESP8266)
	#include <ESP8266WiFi.h>
#else
	#include <WiFi.h>
#endif

#include <WiFiUdp.h>
#include <FastLED.h>

const char* ssid = "";
const char* password = "";

const char* address = "revspace.nl";
const int port = 64738;

const int ledpin = 27;
const int numleds = 25;
CRGB leds[numleds];

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
	udp.begin(port);
	Serial.println();
        FastLED.addLeds < WS2812B, ledpin, GRB > (leds, numleds);

        int i = 0;
	while (WiFi.status() != WL_CONNECTED) {
            delay(50);
            if (i % 10 == 0) Serial.print(".");
            CRGB c = CHSV(i++, 255, 64);
            FastLED.showColor(c);
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
        for (int i = 0; i < numleds; i++) {
            leds[i] = CHSV(200, 255, i < connected ? 50 : 0);
        }
        FastLED.show();
	delay(1000);
}

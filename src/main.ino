#include <FastLED.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiConfig.h>
#include <WiFiUdp.h>

const char* address = "revspace.nl";
const int port = 64738;

const int buttonpin = 39;
const int ledpin = 27;
const int numleds = 25;
CRGB leds[numleds];
uint8_t hue = 0;
byte reqBuffer[12];
char respBuffer[24];
int connected = 0;
int lastTime = 0;

WiFiUDP udp;

struct __attribute__ ((packed)) mumbleRequest {
	unsigned int command;
	unsigned long long ident;
};

const bool _ = false;
const bool X = true;
const bool sprites[] = {
	_, X, X, _,
	X, _, _, X,
	X, _, _, X,
	X, _, _, X,
	_, X, X, _,

	_, _, X, _,
	_, X, X, _,
	_, _, X, _,
	_, _, X, _,
	_, X, X, X,

	_, X, X, _,
	X, _, _, X,
	_, _, X, _,
	_, X, _, _,
	X, X, X, X,

	X, X, X, _,
	_, _, _, X,
	_, X, X, _,
	_, _, _, X,
	X, X, X, _,
	
	_, _, X, _,
	_, X, X, _,
	X, _, X, _,
	X, X, X, X,
	_, _, X, _,

	X, X, X, X,
	X, _, _, _,
	_, X, X, _,
	_, _, _, X,
	X, X, X, _,

	_, X, X, _,
	X, _, _, _,
	X, X, X, _,
	X, _, _, X,
	_, X, X, _,

	X, X, X, X,
	_, _, _, X,
	_, _, X, _,
	_, X, _, _,
	_, X, _, _,

	_, X, X, _,
	X, _, _, X,
	_, X, X, _,
	X, _, _, X,
	_, X, X, _,

	_, X, X, _,
	X, _, _, X,
	_, X, X, X,
	_, _, _, X,
	_, X, X, _,

	_, X, X, _,
	X, _, _, X,
	X, X, X, X,
	X, _, _, X,
	X, _, _, X,

	X, X, X, _,
	X, _, _, X,
	X, X, X, _,
	X, _, _, X,
	X, X, X, _,

	_, X, X, _,
	X, _, _, X,
	X, _, _, _,
	X, _, _, X,
	_, X, X, _,

	X, X, X, _,
	X, _, _, X,
	X, _, _, X,
	X, _, _, X,
	X, X, X, _,

	X, X, X, X,
	X, _, _, _,
	X, X, X, _,
	X, _, _, _,
	X, X, X, X,

	X, X, X, X,
	X, _, _, _,
	X, X, X, _,
	X, _, _, _,
	X, _, _, _
};

void number(int num) {
	int overflow = 0;
	while (num > 15) {
		num = num-16;
		overflow += 1;
	}

	if (overflow > 5) {overflow = 5;}

	for (int y=0; y<5; y++) {
		leds[20-y*5] = CRGB::Black;
		if (overflow > 0) {
			leds[20-y*5] = CHSV((hue+128)%255, 255, 255); // inverse of number hue
			overflow -= 1;
		}
		for (int x=0; x<4; x++) {
			leds[y*5+1+x] = sprites[num*5*4 + y*4 + x] ? CHSV((hue+y*5+x)%255, 255, 255) : CHSV(0, 0, 0);
		}
	}
}

void setup() {
	Serial.begin(115200);
	SPIFFS.begin(true);
	pinMode(buttonpin, INPUT);

	FastLED.addLeds < WS2812B, ledpin, GRB > (leds, numleds);
	FastLED.setBrightness(10);

	WiFiConfig.onWaitLoop = []() {
		static CHSV color(0, 255, 255);
		color.hue++;
		FastLED.showColor(color);
		if (! digitalRead(buttonpin)) WiFiConfig.portal();
		return 50;
	};
	WiFiConfig.onPortalWaitLoop = []() {
		static CHSV color(0, 255, 255);
		color.saturation--;
		FastLED.showColor(color);
	};
	WiFiConfig.connect();

	udp.begin(port);

	// number demoreel
	// for (int i=0; i<5*16-1; i++) {
	// 	for (int j=0; j<30; j++) {
	// 		number(i);
	// 		FastLED.show();
	// 		hue += 1;
	// 		delay(25);
	// 	}
	// }
}


void loop() {
	if (millis() - lastTime > 1000) {
		lastTime = millis();
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
		connected = respBuffer[15];
		Serial.print(connected);
		Serial.println(" people are connected");
	}

	number(connected);
	hue += 1;
	FastLED.show();
	delay(25);
}

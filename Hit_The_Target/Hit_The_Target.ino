#include "WS2812.hpp"

const uint8_t dataPin = 0; // GP0
const uint8_t buttonPin = 1; // GP1
const uint8_t LED_OFF_VALUE = 0;
const uint8_t LED_ON_VALUE = 50;
const uint8_t WIN_FLASH_CYCLES = 10;
const uint8_t HIT_FLASH_CYCLES = 6;
const uint8_t MISS_FLASH_CYCLES = 6;
const uint16_t BLINK_DELAY_MS = 50;
const uint16_t TARGET_HOLD_DELAY_MS = 1000;
const uint16_t TARGET_RESET_DELAY_MS = 500;
const uint16_t BUTTON_RELEASE_POLL_DELAY_MS = 10;

// Speeds split in different arrays, because of the different amount of steps in each level
const uint level0Speed[] = {50, 40, 30, 25, 20};
const uint level1Speed[] = {70, 60, 50, 40, 30, 20};
const uint level2Speed[] = {80, 70, 60, 50, 40, 30, 20, 10, 5};

struct Level {
	const uint* speeds;
	uint steps;
	uint targetRange;
};

const Level levels[] = {
	{ level0Speed, sizeof(level0Speed) / sizeof(level0Speed[0]), 3 },
	{ level1Speed, sizeof(level1Speed) / sizeof(level1Speed[0]), 2 },
	{ level2Speed, sizeof(level2Speed) / sizeof(level2Speed[0]), 1 }
};

const uint speedLevels = sizeof(levels) / sizeof(levels[0]);

uint level = 0;
uint score = 0;
uint target = 0;
uint targetRange = 0;
WS2812 strip;

void setup() {
	pinMode(buttonPin, INPUT_PULLUP);

	strip.setPin(dataPin);
	strip.begin();

	setAllLeds(LED_OFF_VALUE, LED_OFF_VALUE, LED_OFF_VALUE);
	
	setTarget();
	strip.setLed(target, LED_ON_VALUE, LED_OFF_VALUE, LED_OFF_VALUE);
	strip.send();
}

void setAllLeds(uint8_t red, uint8_t green, uint8_t blue) {
	for (uint8_t i = 0; i < ledsAmount; ++i) {
		strip.setLed(i, red, green, blue);
	}
}

void wonGame() {
	for (uint8_t i = 0; i < WIN_FLASH_CYCLES; ++i) {
		setAllLeds(LED_ON_VALUE, LED_OFF_VALUE, LED_OFF_VALUE);
		strip.send();
		delay(BLINK_DELAY_MS);
		setAllLeds(LED_OFF_VALUE, LED_OFF_VALUE, LED_OFF_VALUE);
		strip.send();
		delay(BLINK_DELAY_MS);
		setAllLeds(LED_OFF_VALUE, LED_ON_VALUE, LED_OFF_VALUE);
		strip.send();
		delay(BLINK_DELAY_MS);
		setAllLeds(LED_OFF_VALUE, LED_OFF_VALUE, LED_OFF_VALUE);
		strip.send();
		delay(BLINK_DELAY_MS);
		setAllLeds(LED_OFF_VALUE, LED_OFF_VALUE, LED_ON_VALUE);
		strip.send();
		delay(BLINK_DELAY_MS);
		setAllLeds(LED_OFF_VALUE, LED_OFF_VALUE, LED_OFF_VALUE);
		strip.send();
		delay(BLINK_DELAY_MS);
	}

}

void goalHit() {
	++score;
	if (score >= levels[level].steps) {
		score = 0;
		++level;
		if (level >= speedLevels) {
			wonGame();
			level = 0;
			score = 0;
			return;
		}
	}
	for (uint8_t i = 0; i < HIT_FLASH_CYCLES; ++i) {
		setAllLeds(LED_OFF_VALUE, LED_OFF_VALUE, LED_ON_VALUE);
		strip.send();
		delay(BLINK_DELAY_MS);
		setAllLeds(LED_OFF_VALUE, LED_OFF_VALUE, LED_OFF_VALUE);
		strip.send();
		delay(BLINK_DELAY_MS);
	}
	setTarget();
	setTargetLed();
	strip.send();
	delay(TARGET_HOLD_DELAY_MS);
}

void goalMissed(uint missedLed) {
	score = 0;
	for (uint8_t i = 0; i < MISS_FLASH_CYCLES; ++i) {
		strip.setLed(missedLed, LED_OFF_VALUE, LED_ON_VALUE, LED_OFF_VALUE);
		strip.send();
		delay(BLINK_DELAY_MS);
		strip.setLed(missedLed, LED_OFF_VALUE, LED_OFF_VALUE, LED_OFF_VALUE);
		strip.send();
		delay(BLINK_DELAY_MS);
	}
	setTarget();
	setAllLeds(LED_OFF_VALUE, LED_OFF_VALUE, LED_OFF_VALUE);
	setTargetLed();
	strip.send();
	delay(TARGET_RESET_DELAY_MS);
}

void setTarget() {
	target = random(ledsAmount);
	targetRange = levels[level].targetRange;
}

void setTargetLed() {
	for (uint8_t i = 0; i < targetRange; ++i) {
		strip.setLed((target + i) % ledsAmount, LED_ON_VALUE, LED_OFF_VALUE, LED_OFF_VALUE);
	}
}

void loop() {
	uint8_t activeLed = 0;
	while (true) {
		setAllLeds(LED_OFF_VALUE, LED_OFF_VALUE, LED_OFF_VALUE);
		setTargetLed();
		strip.setLed(activeLed, LED_OFF_VALUE, LED_ON_VALUE, LED_OFF_VALUE);
		strip.send();

		bool pressed = (digitalRead(buttonPin) == LOW);

		if (pressed) {
			bool hit = false;
			for (uint8_t i = 0; i < targetRange; ++i) {
				if (activeLed == (target + i) % ledsAmount) {
					hit = true;
					break;
				}
			}
			if (hit) {
				goalHit();
				break;
			} else {
				goalMissed(activeLed);
				break;
			}

			while (digitalRead(buttonPin) == LOW) {
				delay(BUTTON_RELEASE_POLL_DELAY_MS);
			}
		}

		activeLed = (activeLed + 1) % ledsAmount;
		delay(levels[level].speeds[score]);
	}
}

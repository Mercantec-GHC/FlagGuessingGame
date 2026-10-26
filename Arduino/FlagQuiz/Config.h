#pragma once

#include <Arduino.h>

// Generalt
const char WIFI_SSID[] = "MAGS-OLC";
const char WIFI_PASSWORD[] = "Merc1234!";

const char API_HOST[] = "192.168.1.183";
const int API_PORT = 5169;

// Knapperne bruges uden cover.
const int TOUCH_THRESHOLD = 200;
const unsigned long DEBOUNCE_MS = 30;
const unsigned long RELEASE_MS = 50;
const unsigned long COOLDOWN_MS = 120;

// Den runde skaerm er 240 x 240.
const int SCREEN_SIZE = 240;

// Flaget er mindre end skaermen, men har samme format som originalen.
const int FLAG_WIDTH = 144;
const int FLAG_HEIGHT = 96;
const int FLAG_X = 48;
const int FLAG_Y = 29;

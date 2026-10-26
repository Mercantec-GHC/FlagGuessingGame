#pragma once

#include <Arduino_MKRIoTCarrier.h>

// De sider, som spillet kan vise.
enum GameState {
    HOME,
    INITIALS,
    LOADING,
    PLAYING,
    GAME_OVER,
    ERROR_SCREEN
};

// Variabler, som flere filer bruger.
extern MKRIoTCarrier carrier;
extern GameState gameState;

extern String playerId;
extern char initials[3];
extern int activeInitial;

extern String flagCode;
extern String choices[4];
extern int selectedChoice;
extern int correctChoiceIndex;
extern int score;
extern bool scoreNeedsSaving;

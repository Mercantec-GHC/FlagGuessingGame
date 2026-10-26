#include "Input.h"
#include "Config.h"
#include "Globals.h"
#include "Game.h"

// 0 = knap 00, 1 = knap 02, 2 = knap 04.
const touchButtons buttonPins[3] = {TOUCH0, TOUCH2, TOUCH4};

bool rawPressed[3] = {false, false, false};
bool pressed[3] = {false, false, false};
unsigned long changedTime[3] = {0, 0, 0};

bool canPress = false;
unsigned long releasedTime = 0;
unsigned long lastPressTime = 0;

void setupButtons() {
    carrier.Buttons.updateConfig(TOUCH_THRESHOLD);
}

void checkButtons() {
    carrier.Buttons.update();

    unsigned long now = millis();
    int newButton = -1;
    bool allReleased = true;

    // Laes knapperne og undgaa meget korte, falske tryk.
    for (int i = 0; i < 3; i++) {
        bool touching = carrier.Buttons.getTouch(buttonPins[i]);

        if (touching != rawPressed[i]) {
            rawPressed[i] = touching;
            changedTime[i] = now;
        }

        if (now - changedTime[i] >= DEBOUNCE_MS && pressed[i] != rawPressed[i]) {
            pressed[i] = rawPressed[i];

            if (pressed[i] && newButton == -1) newButton = i;
        }

        if (rawPressed[i] || pressed[i]) allReleased = false;
    }

    // Man skal slippe knappen, foer man maa trykke igen.
    if (!canPress) {
        if (!allReleased) {
            releasedTime = 0;
            return;
        }

        if (releasedTime == 0) releasedTime = now;
        if (now - releasedTime >= RELEASE_MS) canPress = true;
        return;
    }

    if (newButton == -1) return;
    if (now - lastPressTime < COOLDOWN_MS) return;

    canPress = false;
    releasedTime = 0;
    lastPressTime = now;

    // Startskaerm
    if (gameState == HOME) {
        if (newButton == 1) startInitials();
        return;
    }

    // Valg af initialer
    if (gameState == INITIALS) {
        if (newButton == 0) changeLetter();
        if (newButton == 1) goBack();
        if (newButton == 2) confirmLetter();
        return;
    }

    // Gæt flag
    if (gameState == PLAYING) {
        if (newButton == 0) changeAnswer();
        if (newButton == 2) confirmAnswer();
        return;
    }

    // Efter et forkert svar
    if (gameState == GAME_OVER) {
        if (newButton == 1) playAgain();
        if (newButton == 2) retrySave();
        return;
    }

    if (gameState == ERROR_SCREEN) {
        if (newButton == 1) retryGame();
    }
}

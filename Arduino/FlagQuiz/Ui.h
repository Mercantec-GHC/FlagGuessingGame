#pragma once

#include <Arduino.h>

// Funktioner til at tegne paa skaermen.
void clearScreen();
void centerText(const String& text, int y, int size = 1, uint16_t color = 0xFFFF);
void showHome();
void showLoading(const String& title, const String& message = "");
void showError(const String& message);
void showInitials();
void drawLetter(int index);
void showGameOver();
void showSaveError();
void clearSaveError();
void showCorrect();
void showQuizHeader();
void drawOption(int index);
void drawAllOptions();
void showQuizInstructions();

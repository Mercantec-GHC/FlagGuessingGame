#include "Ui.h"
#include "Config.h"
#include "Globals.h"

// Gør lange landenavne kortere, saa de kan vaere paa skaermen.
String shortenText(const String& text, int maxLetters) {
    if (text.length() <= (unsigned int)maxLetters) return text;
    if (maxLetters < 3) return text.substring(0, maxLetters);

    return text.substring(0, maxLetters - 2) + "..";
}

void centerText(const String& text, int y, int size, uint16_t color) {
    carrier.display.setTextSize(size);
    carrier.display.setTextColor(color);
    carrier.display.setTextWrap(false);

    int width = text.length() * 6 * size;
    int x = (SCREEN_SIZE - width) / 2;

    carrier.display.setCursor(max(0, x), y);
    carrier.display.print(text);
}

void clearScreen() {
    carrier.display.fillScreen(ST77XX_BLACK);
}

void showHome() {
    gameState = HOME;
    clearScreen();

    centerText("FLAGSPIL", 77, 2);
    centerText("Gaet landet", 113);
    centerText("Tryk 02 for start", 159, 1, ST77XX_YELLOW);
}

void showLoading(const String& title, const String& message) {
    gameState = LOADING;
    clearScreen();

    centerText(title, 102, 2);

    if (!message.isEmpty()) centerText(message, 132);
}

void showError(const String& message) {
    gameState = ERROR_SCREEN;
    clearScreen();

    centerText("FEJL", 83, 2, ST77XX_RED);
    centerText(shortenText(message, 26), 120);
    centerText("02 - PROEV IGEN", 163, 1, ST77XX_YELLOW);

    Serial.print("Fejl: ");
    Serial.println(message);
}

void drawLetter(int index) {
    int x = index == 0 ? 55 : 130;

    carrier.display.fillRect(x, 89, 60, 72, ST77XX_BLACK);
    carrier.display.setTextSize(5);

    if (index == activeInitial) {
        carrier.display.setTextColor(ST77XX_YELLOW);
    } else {
        carrier.display.setTextColor(ST77XX_WHITE);
    }

    carrier.display.setCursor(x + 15, 99);
    carrier.display.print(initials[index]);

    uint16_t lineColor = index == activeInitial ? ST77XX_YELLOW : ST77XX_WHITE;
    carrier.display.drawFastHLine(x + 9, 151, 43, lineColor);
}

void showInitials() {
    gameState = INITIALS;
    clearScreen();

    centerText("INITIALER", 48, 2);

    carrier.display.setTextSize(1);
    carrier.display.setTextColor(ST77XX_WHITE);
    carrier.display.setCursor(62, 75);
    carrier.display.print("FORNAVN");
    carrier.display.setCursor(128, 75);
    carrier.display.print("EFTERNAVN");

    drawLetter(0);
    drawLetter(1);

    centerText("00 SKIFT  04 OK", 190, 1, ST77XX_YELLOW);
    centerText("02 TILBAGE", 207);
}

void showGameOver() {
    gameState = GAME_OVER;
    clearScreen();

    centerText("SPIL SLUT", 47, 2, ST77XX_RED);
    centerText("DINE POINT", 86);
    centerText(String(score), 105, 4, ST77XX_YELLOW);

    if (correctChoiceIndex >= 0 && correctChoiceIndex < 4) {
        centerText("Rigtigt svar:", 151);
        centerText(shortenText(choices[correctChoiceIndex], 24), 168);
    }

    centerText("02 SPIL IGEN", 192, 1, ST77XX_YELLOW);
}

// Vis kun en fejl, hvis scoren IKKE blev gemt.
void showSaveError() {
    carrier.display.fillRect(45, 207, 150, 15, ST77XX_BLACK);
    centerText("04 GEM IGEN", 207, 1, ST77XX_RED);
}

void clearSaveError() {
    carrier.display.fillRect(45, 207, 150, 15, ST77XX_BLACK);
}

void showCorrect() {
    gameState = LOADING;
    clearScreen();

    centerText("KORREKT!", 99, 2, ST77XX_GREEN);
    centerText("Point: " + String(score), 130);
}

void showQuizHeader() {
    clearScreen();
    centerText("POINT: " + String(score), 15, 1, ST77XX_YELLOW);
}

void drawOption(int index) {
    const int widths[4] = {156, 148, 138, 120};
    const int yPositions[4] = {139, 156, 173, 190};

    int width = widths[index];
    int x = (SCREEN_SIZE - width) / 2;
    int y = yPositions[index];

    uint16_t background = ST77XX_BLACK;
    if (index == selectedChoice) background = ST77XX_BLUE;

    carrier.display.fillRect(x, y, width, 16, background);

    String text = String(index + 1) + ". " + choices[index];
    text = shortenText(text, (width - 8) / 6);

    centerText(text, y + 4);
}

void drawAllOptions() {
    for (int i = 0; i < 4; i++) {
        drawOption(i);
    }
}

void showQuizInstructions() {
    centerText("00 SKIFT", 215, 1, ST77XX_YELLOW);
    centerText("04 BEKRAEFT", 226, 1, ST77XX_YELLOW);
}

#include "Game.h"
#include "Globals.h"
#include "Ui.h"
#include "Api.h"

void startInitials() {
    initials[0] = 'A';
    initials[1] = 'A';
    activeInitial = 0;
    playerId = "";
    scoreNeedsSaving = false;

    showInitials();
}

void changeLetter() {
    initials[activeInitial]++;

    if (initials[activeInitial] > 'Z') {
        initials[activeInitial] = 'A';
    }

    drawLetter(activeInitial);
}

void goBack() {
    if (activeInitial == 0) {
        showHome();
        return;
    }

    activeInitial = 0;
    drawLetter(0);
    drawLetter(1);
}

void confirmLetter() {
    if (activeInitial == 0) {
        activeInitial = 1;
        drawLetter(0);
        drawLetter(1);
        return;
    }

    startGame();
}

void startGame() {
    score = 0;
    selectedChoice = 0;
    scoreNeedsSaving = false;

    // Det er nok at vise STARTER... - ingen besked om spilleren.
    showLoading("STARTER...");

    if (!createPlayer()) {
        showError("Kunne ikke starte");
        return;
    }

    nextQuestion();
}

void nextQuestion() {
    showLoading("HENTER", "NYT LAND...");

    if (!getQuestion()) {
        showError("Kunne ikke hente land");
        return;
    }

    showQuizHeader();

    if (!drawFlag()) {
        showError("Kunne ikke hente flag");
        return;
    }

    drawAllOptions();
    showQuizInstructions();
    gameState = PLAYING;
}

void changeAnswer() {
    int oldAnswer = selectedChoice;
    selectedChoice = (selectedChoice + 1) % 4;

    // Tegn kun de to svar, som har skiftet farve.
    drawOption(oldAnswer);
    drawOption(selectedChoice);
}

void confirmAnswer() {
    if (selectedChoice != correctChoiceIndex) {
        scoreNeedsSaving = true;
        showGameOver();
        retrySave();
        return;
    }

    score++;
    showCorrect();
    delay(1500);
    nextQuestion();
}

void retrySave() {
    if (!scoreNeedsSaving) return;

    bool saved = saveScore();
    scoreNeedsSaving = !saved;

    if (saved) {
        clearSaveError();
    } else {
        showSaveError();
    }
}

void playAgain() {
    if (scoreNeedsSaving) {
        retrySave();
        if (scoreNeedsSaving) return;
    }

    startInitials();
}

void retryGame() {
    if (playerId.length() == 0) {
        startGame();
    } else {
        nextQuestion();
    }
}

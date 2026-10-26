#include "Globals.h"

MKRIoTCarrier carrier;
GameState gameState = HOME;

String playerId = "";
char initials[3] = {'A', 'A', '\0'};
int activeInitial = 0;

String flagCode = "";
String choices[4];
int selectedChoice = 0;
int correctChoiceIndex = -1;
int score = 0;
bool scoreNeedsSaving = false;


#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <Arduino_JSON.h>
#include <Arduino_MKRIoTCarrier.h>

MKRIoTCarrier carrier;
WiFiClient client;

const char SERVER[] = "192.168.1.183";
const uint16_t SERVER_PORT = 5169;

const char CREATE_PLAYER_PATH[] = "/api/Player/Create";
const char QUESTION_PATH[] = "/api/Quiz/GetRandomCountry";

const unsigned long RESPONSE_TIMEOUT = 15000UL;

const int FLAG_WIDTH = 160;
const int FLAG_HEIGHT = 107;
const int FLAG_X = 40;
const int FLAG_Y = 10;

int playerId = 0;
int score = 0;
int selectedChoice = 0;
int correctChoiceIndex = -1;
int playerNumber = 1;

String flagCode;
String choices[4];

bool gameActive = false;
bool questionReady = false;
bool requestBusy = false;

// --------------------------------------------------
// Display helpers
// --------------------------------------------------

void showMessage(const String &line1,
                 const String &line2 = "",
                 const String &line3 = "") {
  carrier.display.fillScreen(0x0000);
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(10, 35);
  carrier.display.println(line1);

  carrier.display.setTextSize(1);
  carrier.display.setCursor(10, 85);
  carrier.display.println(line2);
  carrier.display.setCursor(10, 105);
  carrier.display.println(line3);
}

void showStartScreen() {
  gameActive = false;
  questionReady = false;
  showMessage("Flag Quiz", "Press 02 to start", "00: select  04: confirm");
}

void showLoadingScreen() {
  showMessage("Loading...", "Please wait");
}

void showGameOver() {
  gameActive = false;
  questionReady = false;

  carrier.display.fillScreen(0x0000);
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(10, 25);
  carrier.display.println("Game over!");

  carrier.display.setTextSize(1);
  carrier.display.setCursor(10, 70);
  carrier.display.print("Score: ");
  carrier.display.println(score);

  carrier.display.setCursor(10, 100);
  carrier.display.println("Press 02 to restart");
}

// --------------------------------------------------
// Wi-Fi
// --------------------------------------------------

void connectWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    showMessage("Connecting WiFi...");
    WiFi.begin(SECRET_SSID, SECRET_PASS);

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < 15000UL) {
      delay(500);
    }

    if (WiFi.status() != WL_CONNECTED) {
      showMessage("WiFi failed", "Retrying...");
      delay(2000);
    }
  }

  Serial.print("WiFi IP: ");
  Serial.println(WiFi.localIP());
}

// --------------------------------------------------
// HTTP request helper for JSON endpoints
// --------------------------------------------------

bool readHttpResponse(int &statusCode, String &body) {
  statusCode = 0;
  body = "";

  unsigned long start = millis();

  while (!client.available() && client.connected() &&
         millis() - start < RESPONSE_TIMEOUT) {
    delay(10);
  }

  if (!client.available()) {
    client.stop();
    return false;
  }

  String statusLine = client.readStringUntil('\n');
  statusLine.trim();

  int firstSpace = statusLine.indexOf(' ');
  if (firstSpace >= 0) {
    statusCode = statusLine.substring(firstSpace + 1).toInt();
  }

  int contentLength = -1;
  bool chunked = false;

  while (client.connected() || client.available()) {
    String header = client.readStringUntil('\n');
    header.trim();

    if (header.length() == 0) break;

    String lower = header;
    lower.toLowerCase();

    if (lower.startsWith("content-length:")) {
      contentLength = header.substring(15).toInt();
    }

    if (lower.startsWith("transfer-encoding:") &&
        lower.indexOf("chunked") >= 0) {
      chunked = true;
    }
  }

  if (chunked) {
    while (client.connected() || client.available()) {
      String sizeLine = client.readStringUntil('\n');
      sizeLine.trim();

      int chunkSize = (int)strtol(sizeLine.c_str(), nullptr, 16);

      if (chunkSize <= 0) {
        client.readStringUntil('\n');
        break;
      }

      for (int i = 0; i < chunkSize; i++) {
        unsigned long waitStart = millis();

        while (!client.available() && client.connected() &&
               millis() - waitStart < RESPONSE_TIMEOUT) {
          delay(1);
        }

        if (!client.available()) {
          client.stop();
          return false;
        }

        body += (char)client.read();
      }

      client.readStringUntil('\n');
    }
  } else if (contentLength >= 0) {
    for (int i = 0; i < contentLength; i++) {
      unsigned long waitStart = millis();

      while (!client.available() && client.connected() &&
             millis() - waitStart < RESPONSE_TIMEOUT) {
        delay(1);
      }

      if (!client.available()) {
        client.stop();
        return false;
      }

      body += (char)client.read();
    }
  } else {
    while (client.connected() || client.available()) {
      if (client.available()) {
        body += (char)client.read();
      } else {
        delay(1);
      }

      if (millis() - start > RESPONSE_TIMEOUT) break;
    }
  }

  client.stop();
  return statusCode >= 200 && statusCode < 300;
}

bool httpRequest(const char *method,
                 const String &path,
                 const String &payload,
                 int &statusCode,
                 String &body) {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  client.stop();

  if (!client.connect(SERVER, SERVER_PORT)) {
    Serial.println("Cannot connect to API");
    statusCode = 0;
    body = "";
    return false;
  }

  client.print(method);
  client.print(" ");
  client.print(path);
  client.println(" HTTP/1.1");

  client.print("Host: ");
  client.println(SERVER);
  client.println("Connection: close");
  client.println("Accept: application/json");

  if (String(method) == "POST" || String(method) == "PUT") {
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(payload.length());
    client.println();
    client.print(payload);
  } else {
    client.println();
  }

  bool ok = readHttpResponse(statusCode, body);

  Serial.print(method);
  Serial.print(" ");
  Serial.print(path);
  Serial.print(" -> ");
  Serial.println(statusCode);

  return ok;
}

// --------------------------------------------------
// Create player
// --------------------------------------------------

bool createPlayer() {
  JSONVar request;
  request["name"] = "Player " + String(playerNumber);

  String body;
  int statusCode;

  if (!httpRequest("POST", CREATE_PLAYER_PATH,
                   JSON.stringify(request), statusCode, body)) {
    Serial.println("Create player failed");
    Serial.println(body);
    return false;
  }

  body.trim();
  playerId = body.toInt();

  if (playerId <= 0 && body.startsWith("{")) {
    JSONVar response = JSON.parse(body);

    if (JSON.typeof(response) != "undefined" &&
        response.hasOwnProperty("id")) {
      playerId = (int)response["id"];
    }
  }

  if (playerId <= 0) {
    Serial.print("Invalid player response: ");
    Serial.println(body);
    return false;
  }

  playerNumber++;

  Serial.print("Player ID: ");
  Serial.println(playerId);
  return true;
}

// --------------------------------------------------
// Read one byte from flag response
// --------------------------------------------------

int readFlagByte() {
  unsigned long start = millis();

  while (!client.available() && client.connected() &&
         millis() - start < RESPONSE_TIMEOUT) {
    delay(1);
  }

  if (client.available()) {
    return client.read();
  }

  return -1;
}

// --------------------------------------------------
// Download and draw flag
// Server sends 160 x 107 RGB565 pixels, big-endian
// --------------------------------------------------

bool fetchAndDrawFlag(const String &code) {
  String path = "/api/Quiz/GetFlag/" + code;

  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  client.stop();

  if (!client.connect(SERVER, SERVER_PORT)) {
    Serial.println("Cannot connect to flag endpoint");
    return false;
  }

  client.print("GET ");
  client.print(path);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(SERVER);
  client.println("Connection: close");
  client.println("Accept: application/octet-stream");
  client.println();

  unsigned long start = millis();

  while (!client.available() && client.connected() &&
         millis() - start < RESPONSE_TIMEOUT) {
    delay(10);
  }

  if (!client.available()) {
    Serial.println("No flag HTTP response");
    client.stop();
    return false;
  }

  String statusLine = client.readStringUntil('\n');
  statusLine.trim();

  Serial.print("Flag response: ");
  Serial.println(statusLine);

  if (statusLine.indexOf(" 200 ") < 0) {
    Serial.println("Flag HTTP error");
    client.stop();
    return false;
  }

  int contentLength = -1;
  bool chunked = false;

  while (client.connected() || client.available()) {
    String header = client.readStringUntil('\n');
    header.trim();

    if (header.length() == 0) break;

    String lower = header;
    lower.toLowerCase();

    if (lower.startsWith("content-length:")) {
      contentLength = header.substring(15).toInt();
    }

    if (lower.startsWith("transfer-encoding:") &&
        lower.indexOf("chunked") >= 0) {
      chunked = true;
    }
  }

  const size_t expectedBytes =
      (size_t)FLAG_WIDTH * FLAG_HEIGHT * 2;

  Serial.print("Flag Content-Length: ");
  Serial.println(contentLength);

  if (chunked || contentLength != (int)expectedBytes) {
    Serial.println("Unexpected flag response format");
    client.stop();
    return false;
  }

  uint16_t row[FLAG_WIDTH];

  Serial.println("Starting flag pixel read...");
  carrier.display.fillScreen(0x0000);

  for (int y = 0; y < FLAG_HEIGHT; y++) {
    for (int x = 0; x < FLAG_WIDTH; x++) {
      int highByte = readFlagByte();

      if (highByte < 0) {
        Serial.print("Flag data ended at row ");
        Serial.print(y);
        Serial.print(", column ");
        Serial.println(x);
        client.stop();
        return false;
      }

      int lowByte = readFlagByte();

      if (lowByte < 0) {
        Serial.print("Flag data ended at row ");
        Serial.print(y);
        Serial.print(", column ");
        Serial.println(x);
        client.stop();
        return false;
      }

      row[x] = ((uint16_t)highByte << 8) |
               (uint16_t)lowByte;
    }

    carrier.display.drawRGBBitmap(
      FLAG_X, FLAG_Y + y, row, FLAG_WIDTH, 1
    );

    if (y % 10 == 0) {
      Serial.print("Drawn row: ");
      Serial.println(y);
    }
  }

  client.stop();
  Serial.println("Flag downloaded and drawn");
  return true;
}

// --------------------------------------------------
// Fetch next question
// --------------------------------------------------

String getJsonString(JSONVar obj, const char *key) {
  if (!obj.hasOwnProperty(key)) return "";
  return String((const char *)obj[key]);
}

bool fetchQuestion() {
  showLoadingScreen();

  String body;
  int statusCode;

  if (!httpRequest("GET", QUESTION_PATH, "",
                   statusCode, body)) {
    Serial.println("Question request failed");
    Serial.println(body);
    return false;
  }

  Serial.print("Question JSON: ");
  Serial.println(body);

  JSONVar data = JSON.parse(body);

  if (JSON.typeof(data) == "undefined") {
    Serial.println("Question JSON parsing failed");
    return false;
  }

  flagCode = getJsonString(data, "flagCode");

  if (flagCode.length() == 0) {
    flagCode = getJsonString(data, "FlagCode");
  }

  JSONVar options;

  if (data.hasOwnProperty("choices")) {
    options = data["choices"];
  } else if (data.hasOwnProperty("Choices")) {
    options = data["Choices"];
  } else {
    Serial.println("Missing choices");
    return false;
  }

  if (flagCode.length() == 0 ||
      JSON.typeof(options) != "array" ||
      options.length() < 4) {
    Serial.println("Invalid question data");
    return false;
  }

  for (int i = 0; i < 4; i++) {
    choices[i] = String((const char *)options[i]);
  }

  if (data.hasOwnProperty("correctChoiceIndex")) {
    correctChoiceIndex = (int)data["correctChoiceIndex"];
  } else if (data.hasOwnProperty("CorrectChoiceIndex")) {
    correctChoiceIndex = (int)data["CorrectChoiceIndex"];
  } else {
    Serial.println("Missing correctChoiceIndex");
    return false;
  }

  if (correctChoiceIndex < 0 || correctChoiceIndex > 3) {
    Serial.println("Invalid correctChoiceIndex");
    return false;
  }

  Serial.print("Flag code: ");
  Serial.println(flagCode);

  if (!fetchAndDrawFlag(flagCode)) {
    Serial.println("Flag download failed");
    showMessage("Flag download failed", "Check Serial Monitor");
    return false;
  }

  selectedChoice = 0;
  drawChoices();
  questionReady = true;

  return true;
}

// --------------------------------------------------
// Draw answer choices
// --------------------------------------------------

void drawChoices() {
  carrier.display.fillRect(0, 125, 320, 115, 0x0000);

  carrier.display.setTextSize(1);
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setCursor(5, 127);
  carrier.display.print("Score: ");
  carrier.display.println(score);

  for (int i = 0; i < 4; i++) {
    int y = 145 + i * 23;

    if (i == selectedChoice) {
      carrier.display.fillRect(3, y - 2, 314, 20, 0x07E0);
      carrier.display.setTextColor(0x0000);
    } else {
      carrier.display.fillRect(3, y - 2, 314, 20, 0x0000);
      carrier.display.setTextColor(0xFFFF);
    }

    carrier.display.setCursor(8, y);
    carrier.display.print(i + 1);
    carrier.display.print(". ");
    carrier.display.println(choices[i]);
  }

  carrier.display.setTextColor(0xFFFF);
}

// --------------------------------------------------
// Save score
// --------------------------------------------------

void saveScore() {
  if (playerId <= 0) return;

  JSONVar request;
  request["score"] = score;

  String path = "/api/Player/" + String(playerId) + "/Score";
  String body;
  int statusCode;

  if (!httpRequest("PUT", path, JSON.stringify(request),
                   statusCode, body)) {
    Serial.println("Could not save score");
    Serial.println(body);
  }
}

// --------------------------------------------------
// Game controls
// --------------------------------------------------

void startGame() {
  if (requestBusy || gameActive) return;

  requestBusy = true;
  gameActive = false;
  questionReady = false;
  score = 0;
  playerId = 0;

  showLoadingScreen();

  if (createPlayer() && fetchQuestion()) {
    gameActive = true;
  } else {
    gameActive = false;
    questionReady = false;
    showMessage("Could not start", "Check Serial Monitor");
  }

  requestBusy = false;
}

void confirmAnswer() {
  if (!gameActive || !questionReady || requestBusy) return;

  requestBusy = true;
  questionReady = false;

  if (selectedChoice == correctChoiceIndex) {
    score++;

    if (fetchQuestion()) {
      questionReady = true;
    } else {
      gameActive = false;
      showMessage("Question error", "Could not load next");
    }
  } else {
    saveScore();
    showGameOver();
  }

  requestBusy = false;
}

// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1500);

  carrier.begin();
  carrier.display.fillScreen(0x0000);

  connectWifi();
  showStartScreen();
}

// --------------------------------------------------
// Main loop
// Button 02 = start/restart
// Button 00 = select answer
// Button 04 = confirm answer
// --------------------------------------------------

void loop() {
  carrier.Buttons.update();

  // Start/restart only when no game is active.
  if (carrier.Buttons.onTouchDown(TOUCH2)) {
    if (!requestBusy && !gameActive) {
      startGame();
    }
  }

  // Cycle through the four answers.
  if (carrier.Buttons.onTouchDown(TOUCH0)) {
    if (gameActive && questionReady && !requestBusy) {
      selectedChoice = (selectedChoice + 1) % 4;
      drawChoices();
    }
  }

  // Confirm the selected answer. Never starts a game.
  if (carrier.Buttons.onTouchDown(TOUCH4)) {
    if (gameActive && questionReady && !requestBusy) {
      confirmAnswer();
    }
  }

  delay(30);
}

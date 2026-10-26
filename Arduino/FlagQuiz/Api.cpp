#include "Api.h"
#include "Config.h"
#include "Globals.h"

#include <WiFiNINA.h>
#include <ArduinoJson.h>

bool connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return true;

    Serial.println("Forbinder til WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > 15000) return false;
        delay(100);
    }

    Serial.println("WiFi forbundet");
    return true;
}

// Alle API-kald bruger denne funktion til at sende HTTP.
// Efter funktionen er kaldt, kan man laese svar-data fra client.
bool sendRequest(WiFiClient& client, const char* method, const String& path,
                 const String& body, int& status, long& contentLength) {
    status = 0;
    contentLength = -1;

    if (!connectWiFi()) return false;

    client.setTimeout(5000);

    if (!client.connect(API_HOST, API_PORT)) {
        Serial.println("Kunne ikke forbinde til API");
        return false;
    }

    // Vi bruger HTTP/1.0, saa vi kan modtage billedet som raa bytes.
    client.print(method);
    client.print(" ");
    client.print(path);
    client.println(" HTTP/1.0");
    client.print("Host: ");
    client.println(API_HOST);
    client.println("Connection: close");
    client.println("Accept-Encoding: identity");

    if (strcmp(method, "GET") != 0) {
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(body.length());
    }

    client.println();

    if (body.length() > 0) client.print(body);

    String statusLine = client.readStringUntil('\n');
    statusLine.trim();

    if (!statusLine.startsWith("HTTP/")) {
        Serial.println("Ugyldigt HTTP svar");
        client.stop();
        return false;
    }

    int space = statusLine.indexOf(' ');
    status = statusLine.substring(space + 1, space + 4).toInt();

    bool chunked = false;

    // Spring HTTP-headers over, indtil vi naar selve svaret.
    while (true) {
        String header = client.readStringUntil('\n');
        header.trim();

        if (header.length() == 0) break;

        String lowerHeader = header;
        lowerHeader.toLowerCase();

        if (lowerHeader.startsWith("content-length:")) {
            contentLength = header.substring(15).toInt();
        }

        if (lowerHeader.startsWith("transfer-encoding:") && lowerHeader.indexOf("chunked") >= 0) {
            chunked = true;
        }
    }

    Serial.print(method);
    Serial.print(" ");
    Serial.print(path);
    Serial.print(" -> ");
    Serial.println(status);

    if (chunked) {
        Serial.println("Chunked HTTP understottes ikke");
        client.stop();
        return false;
    }

    return true;
}

bool requestWorked(int status) {
    return status >= 200 && status < 300;
}

void printApiError(WiFiClient& client, int status) {
    Serial.print("API fejl: ");
    Serial.println(status);
    client.setTimeout(1000);
    Serial.println(client.readString().substring(0, 400));
}

bool createPlayer() {
    WiFiClient client;
    int status;
    long length;

    String name = String(initials);
    String body = "{\"name\":\"" + name + "\"}";

    if (!sendRequest(client, "POST", "/api/Player/Create", body, status, length)) return false;

    if (!requestWorked(status)) {
        printApiError(client, status);
        client.stop();
        return false;
    }

    // API'et svarer med et tal, f.eks. 123.
    String id = client.readString();
    client.stop();
    id.trim();

    if (id.startsWith("\"") && id.endsWith("\"")) {
        id = id.substring(1, id.length() - 1);
    }

    if (id.length() == 0 || id == "0") return false;

    for (unsigned int i = 0; i < id.length(); i++) {
        if (!isDigit(id[i])) return false;
    }

    playerId = id;

    Serial.print("Spiller ");
    Serial.print(name);
    Serial.print(" har ID ");
    Serial.println(playerId);
    return true;
}

bool saveScore() {
    if (playerId.length() == 0) return false;

    WiFiClient client;
    int status;
    long length;

    String path = "/api/Player/" + playerId + "/Score";
    String body = "{\"score\":" + String(score) + "}";

    if (!sendRequest(client, "PUT", path, body, status, length)) return false;

    if (!requestWorked(status)) {
        printApiError(client, status);
        client.stop();
        return false;
    }

    client.stop();
    Serial.println("Point gemt");
    return true;
}

bool getQuestion() {
    WiFiClient client;
    int status;
    long length;

    if (!sendRequest(client, "GET", "/api/Quiz/GetRandomCountry", "", status, length)) return false;

    if (!requestWorked(status)) {
        printApiError(client, status);
        client.stop();
        return false;
    }

    JsonDocument json;
    DeserializationError error = deserializeJson(json, client);
    client.stop();

    if (error) {
        Serial.print("JSON fejl: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonArray answers = json["choices"].as<JsonArray>();
    int answer = json["correctChoiceIndex"] | -1;
    const char* code = json["flagCode"] | "";

    if (answers.isNull() || answers.size() != 4) return false;
    if (answer < 0 || answer > 3 || strlen(code) == 0) return false;

    // Check at der er fire forskellige svar.
    String newChoices[4];

    for (int i = 0; i < 4; i++) {
        newChoices[i] = answers[i].as<String>();
        newChoices[i].trim();

        if (newChoices[i].length() == 0) return false;

        for (int j = 0; j < i; j++) {
            if (newChoices[i].equalsIgnoreCase(newChoices[j])) return false;
        }
    }

    flagCode = String(code);
    correctChoiceIndex = answer;
    selectedChoice = 0;

    for (int i = 0; i < 4; i++) {
        choices[i] = newChoices[i];
    }

    return true;
}

// Flaget kommer som RGB565 og vises en raekke ad gangen.
// Det sparer RAM paa Arduinoen.
bool drawFlag() {
    WiFiClient client;
    int status;
    long length;

    String path = "/api/Quiz/GetFlag/" + flagCode;

    if (!sendRequest(client, "GET", path, "", status, length)) return false;

    if (!requestWorked(status)) {
        printApiError(client, status);
        client.stop();
        return false;
    }

    int sourceWidth = 0;
    int sourceHeight = 0;
    int cropTop = 0;
    int contentHeight = 0;

    if (length == 34240) {          // 160 x 107
        sourceWidth = 160;
        sourceHeight = 107;
        contentHeight = 107;
    } else if (length == 76800) {   // 240 x 160
        sourceWidth = 240;
        sourceHeight = 160;
        contentHeight = 160;
    } else if (length == 115200) {  // 240 x 240 med sorte kanter
        sourceWidth = 240;
        sourceHeight = 240;
        cropTop = 40;
        contentHeight = 160;
    } else {
        Serial.print("Ukendt flag-storrelse: ");
        Serial.println(length);
        client.stop();
        return false;
    }

    uint8_t buffer[480];
    uint16_t pixels[FLAG_WIDTH];
    int lastRow = -1;

    for (int y = 0; y < sourceHeight; y++) {
        int rowSize = sourceWidth * 2;

        if (client.readBytes(buffer, rowSize) != (size_t)rowSize) {
            Serial.println("Kunne ikke hente hele flaget");
            client.stop();
            return false;
        }

        int contentY = y - cropTop;

        if (contentY < 0 || contentY >= contentHeight) continue;

        int outputY = contentY * FLAG_HEIGHT / contentHeight;

        if (outputY == lastRow) continue;
        lastRow = outputY;

        for (int x = 0; x < FLAG_WIDTH; x++) {
            int sourceX = x * sourceWidth / FLAG_WIDTH;
            int offset = sourceX * 2;

            pixels[x] = ((uint16_t)buffer[offset] << 8) | buffer[offset + 1];
        }

        carrier.display.drawRGBBitmap(FLAG_X, FLAG_Y + outputY, pixels, FLAG_WIDTH, 1);
    }

    client.stop();
    return true;
}

#include <Arduino_MKRIoTCarrier.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>

#include "Config.h"
#include "Globals.h"
#include "Ui.h"
#include "Api.h"
#include "Game.h"
#include "Input.h"

void setup() {
    Serial.begin(115200);

    // Vi bruger ikke cover, og knapperne skal vaere nederst.
    carrier.noCase();
    carrier.begin();
    carrier.display.setRotation(0);

    setupButtons();

    showLoading("FORBINDER", "TIL WIFI...");

    if (!connectWiFi()) {
        showError("WiFi fejl");
        return;
    }

    showHome();
}

void loop() {
    checkButtons();
    delay(2);
}

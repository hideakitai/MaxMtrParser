// include file system library before MTR Parser

// if you want to use standard SD library
#include <SD.h>
#define fs SD

// If you want use SPIFFS (ESP32) or other FileSystems
// #include <SPIFFS.h>
// #define fs SPIFFS

// #define MAXMTRPARSER_DEBUGLOG_ENABLE
#include <MaxMtrParser.h>

File file;
String path = "/single.txt";
// String path = "/multi.txt";
MaxMtrParser mtr;
size_t curr_track = 0;
uint32_t start_ms = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    fs.begin();
    if (fs.exists(path)) {
        file = fs.open(path, FILE_READ);
        if (!file) {
            Serial.print("File open failed: ");
            Serial.println(path);
            return;
        }
    } else {
        Serial.print("File not found: ");
        Serial.println(path);
        return;
    }

    mtr.attach(file);

    curr_track = 0;
    start_ms = millis();
}

void loop() {
    if (mtr.hasNextLine()) {
        if (millis() >= start_ms + mtr.nextTimeMs()) {
            printArgs();
            mtr.pop();
        }
    } else {
        if (++curr_track >= mtr.numTracks()) {
            curr_track = 0;
            Serial.println("Restart mtr file");
        }
        Serial.print("Next track = ");
        Serial.println(curr_track);

        mtr.seek(curr_track, 0);
        start_ms = millis();
    }
}

void printArgs() {
    Serial.print("Elapsed = ");
    Serial.print(millis() - start_ms);
    Serial.print(", next time ms = ");
    Serial.print(mtr.nextTimeMs());
    Serial.print(", arg size = ");
    Serial.print(mtr.numArgs());
    Serial.print(", args : ");
    for (size_t i = 0; i < mtr.numArgs(); ++i) {
        Serial.print(mtr.argAsString(i));
        Serial.print(", ");
    }
    Serial.println();
}

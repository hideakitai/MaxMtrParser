# MaxMtrParser

Max/MSP MTR (Multi-Track Recorder) Protocol Parser for Arduino

## Usage

See `parser` example for the details.

```C++
// include file system library before MTR Parser
#include <SD.h>
#define fs SD

// #define MAXMTRPARSER_DEBUGLOG_ENABLE
#include <MaxMtrParser.h>

File file;
String path = "/single.txt";
MaxMtrParser mtr;
uint32_t start_ms = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    fs.begin();
    file = fs.open(path, FILE_READ);

    mtr.attach(file);
    start_ms = millis();
}

void loop() {
    if (mtr.hasNextLine()) {
        if (millis() >= start_ms + mtr.nextTimeMs()) {
            for (size_t i = 0; i < mtr.numArgs(); ++i) {
                Serial.print(mtr.argAsString(i));
                Serial.print(", ");
            }
            Serial.println();

            mtr.pop();
        }
    }
}
```

## Options

### Enable Debug Logger

```C++
// define this preprocessor
#define MAXMTRPARSER_DEBUGLOG_ENABLE

// then include library
#include <MaxMtrParser.h>
```

### Change the size limit of Track / Arguments

```C++
// define this preprocessor (default: 16)
#define MAXMTRPARSER_MAX_ARGS 16
#define MAXMTRPARSER_MAX_TRACKS 16

// then include library
#include <MaxMtrParser.h>
```

## Embedded Libraries

- [ArxContainer v0.4.0](https://github.com/hideakitai/ArxContainer)
- [DebugLog v0.6.6](https://github.com/hideakitai/DebugLog)

## APIs

```C++
bool attach(File& f, const size_t track_idx = 0);
bool hasNextLine();
uint32_t nextTimeMs() const;
bool seek(const size_t track, const uint32_t time_ms);
bool seek(const uint32_t time_ms);

size_t numTracks() const;
uint8_t numArgs() const;

const String& argAsString(const uint8_t i) const;
int32_t argAsInt(const uint8_t i) const;
float argAsFloat(const uint8_t i) const;
double argAsDouble(const uint8_t i) const;
void pop();
```

## License

MIT

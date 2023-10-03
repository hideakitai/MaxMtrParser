#pragma once
#ifndef ARDUINO_MAXMTRP_PARSER_H
#define ARDUINO_MAXMTRP_PARSER_H

#include <Arduino.h>

#include <DebugLog.h>
#ifdef MAXMTRPARSER_DEBUGLOG_ENABLE
#include <DebugLogEnable.h>
#else
#include <DebugLogDisable.h>
#endif

#include <ArxContainer.h>

namespace arduino {
namespace maxmtr {

#ifndef MAXMTRPARSER_MAX_ARGS
#define MAXMTRPARSER_MAX_ARGS 16
#endif  // MAXMTRPARSER_MAX_ARGS

#ifndef MAXMTRPARSER_MAX_TRACKS
#define MAXMTRPARSER_MAX_TRACKS 16
#endif  // MAXMTRPARSER_MAX_TRACKS

#if ARX_HAVE_LIBSTDCPLUSPLUS >= 201103L  // Have libstdc++11
    using namespace std;
#else
    using namespace arx;
#endif

    class Parser {
        File* file {nullptr};
        uint32_t next_time_ms {0xFFFFFFFF};
#if ARX_HAVE_LIBSTDCPLUSPLUS >= 201103L  // Have libstdc++11
        vector<String> args;
        vector<size_t> track_positions;
#else
        vector<String, MAXMTRPARSER_MAX_ARGS> args;
        vector<size_t, MAXMTRPARSER_MAX_TRACKS> track_positions;
#endif

    public:
        bool attach(File& f, const size_t track_idx = 0) {
            file = &f;
            clear();

            if (parse_tracks()) {
                if (track_idx < track_positions.size()) {
                    return seek(track_idx, 0);
                } else {
                    LOG_ERROR("Track index", track_idx, "out of range");
                }
            } else {
                LOG_ERROR("No tracks found");
            }
            return false;
        }

        bool hasNextLine() {
            if (has_next_line_unchecked()) {
                return true;
            } else {
                return parse_next_line();
            }
        }

        uint32_t nextTimeMs() const {
            return next_time_ms;
        }

        bool seek(const size_t track, const uint32_t time_ms) {
            next_time_ms = 0;
            if (file->seek(track_positions[track])) {
                while (next_time_ms <= time_ms) {
                    if (parse_next_line()) {
                        return true;
                    } else {
                        LOG_ERROR("Seek time", time_ms, "[ms] failed");
                        break;
                    }
                }
            } else {
                LOG_ERROR("Seek track index", track, "failed");
            }
            return false;
        }

        bool seek(const uint32_t time_ms) {
            return seek(0, time_ms);
        }

        size_t numTracks() const {
            return track_positions.size();
        }

        uint8_t numArgs() const {
            return args.size();
        }

        const String& argAsString(const uint8_t i) const {
            return args[i];
        }
        int32_t argAsInt(const uint8_t i) const {
            return args[i].toInt();
        }
        float argAsFloat(const uint8_t i) const {
            return args[i].toFloat();
        }
        double argAsDouble(const uint8_t i) const {
#ifdef TEENSYDUINO
            return args[i].toFloat();
#else
            return args[i].toDouble();
#endif
        }

        void pop() {
            args.clear();
            args.reserve(MAXMTRPARSER_MAX_ARGS);
        }

    private:
        void clear() {
            next_time_ms = 0xFFFFFFFF;
            args.clear();
            args.reserve(MAXMTRPARSER_MAX_ARGS);
        }

        bool has_next_line_unchecked() {
            return (next_time_ms != 0xFFFFFFFF) && !args.empty();
        }

        bool has_next_line_checked() {
            return (next_time_ms != 0xFFFFFFFF) && args.empty();
        }

        bool is_next_line_track_start(const String& next_line) const {
            return next_line.substring(0, 5) == "track";
        }

        bool is_next_line_track_end(const String& next_line) const {
            return next_line.substring(0, 4) == "end;";
        }

        bool parse_tracks() {
            while (file->available()) {
                String next_line = file->readStringUntil('\n');
                if (is_next_line_track_start(next_line)) {
                    track_positions.emplace_back(file->position());
                }
            }
            return track_positions.empty() == false;
        }

        bool parse_next_line() {
            if (file->available()) {
                String next_line = file->readStringUntil('\n');

                if (is_next_line_track_start(next_line)) {
                    LOG_INFO("Track header");
                    next_line = file->readStringUntil('\n');
                }

                if (is_next_line_track_end(next_line)) {
                    LOG_INFO("Track footer");
                    clear();
                    return false;
                } else {
                    size_t token_begin = 0;
                    size_t token_end = 0;
                    parse_time(next_line, token_begin, token_end);
                    parse_args(next_line, token_begin, token_end);
                    return true;
                }
            } else {
                LOG_INFO("MTR file reached EOF");
                clear();
                return false;
            }
        }

        void parse_time(const String& next_line, size_t& token_begin, size_t& token_end) {
            token_end = next_line.indexOf(' ', token_begin);
            next_time_ms += next_line.substring(token_begin, token_end).toInt();
            fetch_next_token_index(next_line, token_begin, token_end);
        }

        void parse_args(const String& next_line, size_t& token_begin, size_t& token_end) {
            args.clear();

            while (token_end != -1) {
                args.emplace_back(next_line.substring(token_begin, token_end));
                fetch_next_token_index(next_line, token_begin, token_end);
            }
            args.emplace_back(next_line.substring(token_begin, next_line.lastIndexOf(';')));
        }

        void fetch_next_token_index(const String& next_line, size_t& token_begin, size_t& token_end) const {
            token_begin = token_end + 1;
            token_end = next_line.indexOf(' ', token_begin);
        }
    };

}  // namespace maxmtr
}  // namespace arduino

using MaxMtrParser = arduino::maxmtr::Parser;

#include <DebugLogRestoreState.h>

#endif  // ARDUINO_MAXMTRP_PARSER_H

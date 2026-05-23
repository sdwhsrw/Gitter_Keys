#include "debug.h"
#include "tusb.h"

void raw_cdc_print(const char* str) {
    if (tud_cdc_connected()) {
        tud_cdc_write(str, strlen(str));
        tud_cdc_write_flush();
    }
}

void debug(int level, const char *message) {
    if (level < DEBUG_LEVEL) return;
    raw_cdc_print(message);
}

void debug(int level, String message) {
    debug(level, message.c_str());
}

void debugln(int level, const char *message) {
    if (level < DEBUG_LEVEL) return;
    raw_cdc_print(message);
    raw_cdc_print("\r\n");
}

void debugln(int level, String message) {
    debugln(level, message.c_str());
}

void debugln(int level) {
    if (level < DEBUG_LEVEL) return;
    raw_cdc_print("\r\n");
}

void debugf(int level, const char *message, ...) {
    if (level < DEBUG_LEVEL) return;

    va_list ap;
    va_start(ap, message);
    char buffer[200]= {0};
    vsnprintf(buffer, 200, message, ap);
    va_end(ap);
    raw_cdc_print(buffer);
}

void debugf(int level, String message, ...) {
    debugf(level, message.c_str());
}

void debugfln(int level, const char *message, ...) {
    if (level < DEBUG_LEVEL) return;

    va_list ap;
    va_start(ap, message);
    char buffer[200]= {0};
    vsnprintf(buffer, 200, message, ap);
    va_end(ap);

    raw_cdc_print(buffer);
    raw_cdc_print("\r\n");
}

void debugfln(int level, String message, ...) {
    debugfln(level, message.c_str());
}

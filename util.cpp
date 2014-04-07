/* util.cpp
 * Tested with mbed board: FRDM-KL46Z
 * Author: Mark Gottscho
 * mgottscho@ucla.edu
 */
 
#include "mbed.h"
#include "util.h"

uint32_t bytes_to_unsigned(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0) {
    uint32_t var;
    
    var = b3 << 24;
    var |= b2 << 16;
    var |= b1 << 8;
    var |= b0;
    
    return var;
}

void unsigned_to_bytes(const uint32_t var, uint8_t &b3, uint8_t &b2, uint8_t &b1, uint8_t &b0) {
    b3 = (var >> 24) & 0xFF;
    b2 = (var >> 16) & 0xFF;
    b1 = (var >> 8) & 0xFF;
    b0 = (var) & 0xFF;
}

void panic(Serial *serial, const char *errorMessage, int errorCode, DigitalOut *green_led, DigitalOut *red_led) {
    //We're dead. This is the point of no return! Permanently ignore interrupts.
    __disable_irq();
    
    if (green_led != NULL)
        *green_led = 1; //Turn off green LED
    if (red_led != NULL)
        *red_led = 0; //Turn on red LED
    
    if (serial != NULL) {
        if (errorMessage != NULL)
            serial->printf(">>> PANIC, CODE # %d: %s <<<\r\n", errorCode, errorMessage);
        else
            serial->printf(">>> PANIC <<<\r\n");
    }
    
    while(1); //Spinloop for eternity
}

void warn(Serial *serial, const char *errorMessage, int errorCode) {
    if (serial != NULL) {
        if (errorMessage != NULL)
            serial->printf(">>> WARN, CODE # %d: %s <<<\r\n", errorCode, errorMessage);
        else
            serial->printf(">>> WARN <<<\r\n");
    }
}


#ifdef NDEBUG
void myAssert(Serial *serial, bool condition, const char *file, unsigned long line, DigitalOut *green_led, DigitalOut *red_led) { }
#else
void myAssert(Serial *serial, bool condition, const char *file, unsigned long line, DigitalOut *green_led, DigitalOut *red_led) {
    if (!condition)
    {
        char msg[256];
        if (file != NULL)
            sprintf(msg, "Assertion failed at file %s, line %d", file, line);
        else
            sprintf(msg, "Assertion failed at file UNKNOWN, line %d", line);
        panic(serial, msg, -1, green_led, red_led);
    }   
}
#endif

void toggleLED(DigitalOut *led) {
    if (led != NULL) {
        if (*led == 1)
            *led = 0;
        else
            *led = 1;   
    }
}
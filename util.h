/* util.h
 * Tested with mbed board: FRDM-KL46Z
 * Author: Mark Gottscho
 * mgottscho@ucla.edu
 */
 
#include "mbed.h"

/**
 * Convert four bytes to an unsigned 32-bit int, MSB first.
 */
uint32_t bytes_to_unsigned(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0);

/**
 * Convert an unsigned 32-bit int to 4 bytes, MSB first.
 */
void unsigned_to_bytes(const uint32_t var, uint8_t &b3, uint8_t &b2, uint8_t &b1, uint8_t &b0);

/**
 * Infinitely loop in a panic condition, disabling the green user LED, enabling the red user LED and printing an error message.
 * This method never returns!
 * @param serial the serial interface to use if any
 * @param errorMessage the C string to print
 * @param errorCode the accompanying error code to print
 * @param green_led the green LED if any
 * @param red_led the red LED if any
 */ 
void panic(Serial *serial, const char *errorMessage, int errorCode, DigitalOut *green_led, DigitalOut *red_led);

/**
 * Print a warning message to a serial console.
 * @param serial the serial interface to use if any
 * @param errorMessage the C string to print
 * @param errorCode the accompanying error code to print
 */
void warn(Serial *serial, const char *errorMessage, int errorCode);

/**
 * Assert statement.
 * @param serial the serial interface to use if any
 * @param condition if false, calls panic() with results of the assertion, and never returns.
 * @param file string representing the source file where the assertion is called
 * @param line line number of the source file where the assertion is called
 * @param green_led the green LED, if any
 * @param red_led the red LED, if any
 */
void myAssert(Serial *serial, bool condition, const char *file, unsigned long line, DigitalOut *green_led, DigitalOut *red_led);

/**
 * Toggles an LED, if any.
 * @param led the LED.
 */
void toggleLED(DigitalOut *led);
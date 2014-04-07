/* SerialManager.h
 * Tested with mbed board: FRDM-KL46Z
 * Author: Mark Gottscho
 * mgottscho@ucla.edu
 */
 
#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H
 
#include "mbed.h"

class SerialManager {
    public:
        /**
         * Constructs a SerialManager object, which manages serial communication and interrupts for a specific port.
         * @param serial_tx_pin pin for the UART TX
         * @param serial_rx_pin pin for the UART RX
         * @param enableSerialInterrupts if true, allows interrupt handlers on the serial port
         */
        SerialManager(PinName serial_tx_pin, PinName serial_rx_pin, int baudrate, bool enableSerialInterrupts);
        
        /**
         * Destructor.
         */
        ~SerialManager();
        
        /**
         * Attaches a user-specified callback function that is called each time serial data is received.
         * @param fptr the user callback function
         */
        void attach_rx(void (*fptr)(void));
        
        /**
         * Attaches a user-specified callback member function that is called each time serial data is received.
         * @param tptr the object 
         * @param mptr method to call on the object
         */
        template<typename T> void attach_rx(T *tptr, void (T::*mptr)(void));
        
        /**
         * Detaches the user-specified RX callback function, if any.
         */
        void detach_rx();
        
        /**
         * Receives a line of data from the serial console, terminated by a carriage return character.
         * @param line a pointer to the buffer in which to store the incoming data
         * @param len the maximum number of bytes to receive
         * @returns number of bytes received
         */
        uint32_t receiveLine(char *line, const uint32_t len);
        
        /**
         * Sends a line of data to the serial port.
         * @param line a pointer to the beginning of the data to send
         * @param len the number of bytes to send
         * @returns number of bytes sent
         */
        //uint32_t sendLine(const char *line, const uint32_t len);
        
        /**
         * @returns true if there is data received from serial port ready to use.
         */
        bool haveRxSerialData();
        
        /**
         * Flushes the serial RX buffer.
         */
        void flush();
        
        /**
         * Prints a string to the serial device, terminated by CRLF. Interrupt-safe.
         * @param line the C string to print
         * @param len the maximum length of the line
         */
        void print_line(const char *line, int len);
        
        /**
         * In case the user needs direct Serial access.
         */
        Serial serial;
                
    private:        
        /**
         * Interrupt service routine for serial RX
         */
        void __serial_rx_ISR();
        
        /**
         * Interrupt service routine for serial TX
         */
        //void __serial_tx_ISR();
        
        inline void __disable_uart_irq();
        inline void __enable_uart_irq();
        
        bool __interrupts_en;
        FunctionPointer *__user_fptr; //User callback function that is invoked in __serial_rx_ISR()
        
        //Buffers for working with the serial interface
        const static uint32_t BUFFER_SIZE = 512; //For serial buffer
        volatile uint8_t __rx_buf[BUFFER_SIZE];
        volatile uint8_t __tx_buf[BUFFER_SIZE];
        volatile uint32_t __rx_head; //Head always points to the first item to read (oldest)
        volatile uint32_t __rx_tail; //Tail always points to the last item written (newest)
        volatile uint32_t __tx_head;
        volatile uint32_t __tx_tail;
        volatile bool __have_rx_serial; //Flag for the RX data
};

#endif
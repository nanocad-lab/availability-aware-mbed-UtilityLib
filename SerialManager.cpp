/* SerialManager.cpp
 * Tested with mbed board: FRDM-KL46Z
 * Author: Mark Gottscho
 * mgottscho@ucla.edu
 */
 
#include "mbed.h"
#include "SerialManager.h"

using namespace std;

SerialManager::SerialManager(PinName serial_tx_pin, PinName serial_rx_pin, int baudrate, bool enableSerialInterrupts) :
                                            serial(serial_tx_pin, serial_rx_pin),
                                            __interrupts_en(enableSerialInterrupts),
                                            __user_fptr(NULL),
                                            __rx_head(0),
                                            __rx_tail(0),
                                            __tx_head(0),
                                            __tx_tail(0),
                                            __have_rx_serial(false)
                                             {
    
    serial.baud(baudrate);
    
    //Set up serial interrupt handlers
    if (__interrupts_en) {
        serial.attach(this, &SerialManager::__serial_rx_ISR, Serial::RxIrq);
        //serial.attach(this, &SerialManager::__serial_tx_ISR, Serial::TxIrq);
    }
}
                                            
SerialManager::~SerialManager() {
    detach_rx();
}

void SerialManager::attach_rx(void (*fptr)(void)) {
    //set user function pointer
    detach_rx();
    if (fptr != NULL)
        __user_fptr = new FunctionPointer(fptr);
}

template<typename T> void SerialManager::attach_rx(T *tptr, void (T::*mptr)(void)) {
    //set user function pointer
    detach();
    if (tptr != NULL && mptr != NULL)
        __user_fptr = new FunctionPointer(tptr, mptr);
}

void SerialManager::detach_rx() {
    if (__user_fptr != NULL)
        delete __user_fptr;
    __user_fptr = NULL;
}

//THIS DOES NOT WORK YET
/*uint32_t SerialManager::sendLine(const char *line, const uint32_t len) {
    int i = 0;
    char temp_byte;
    bool buf_empty;
    
    if (line == NULL) //check input
        return 0;

    G_red_led = 0;
    
    // Start critical section - don't interrupt while changing global buffer variables
    __disable_irq();
    buf_empty = (G_tx_head == G_tx_tail);
    
    while (i < len && line[i] != '\r') { //Loop until we have sent the maximum number of characters or we hit a carriage return
        // Wait if tx buffer full
        if ((G_tx_head + 1) % BUFFER_SIZE == G_tx_tail) { //If TX buffer is full, wait.
            // End critical section - need to let interrupt routine empty buffer by sending
            __enable_irq();
            while ((G_tx_head + 1) % BUFFER_SIZE == G_tx_tail) { } //Spinloop until TX buffer is not full
            // Start critical section - don't interrupt while changing global buffer variables
            __disable_irq();
        }
        G_tx_head = (G_tx_head + 1) % BUFFER_SIZE;
        G_tx_buf[G_tx_head] = line[i++];
    }
    
    //Now we have buffered all characters in the line. Trigger the TX serial interrupt
    if (G_console.writeable() && buf_empty) {
        //Write the first byte to get it started
        temp_byte = G_tx_buf[G_tx_tail];
        G_tx_tail = (G_tx_tail + 1) % BUFFER_SIZE;
        
        // Send first character to start tx interrupts, if stopped
        G_console.putc(temp_byte);
    }
    
    // End critical section
    __enable_irq();
    
    G_red_led = 1;
    
    return i;
}*/


uint32_t SerialManager::receiveLine(char *line, const uint32_t len) {
    int i = 0;
    char lastChar = '\0';
    
    if (line == NULL) //check input
        return 0;

    // Start critical section - don't interrupt while changing global buffer variables
    __disable_uart_irq();
    
    while (i < len && lastChar != '\r') { //Loop until maximum number of characters or a newline symbol
        //Wait for more characters if the rx buffer is empty
        if (__rx_tail == __rx_head) {
            // End critical section - need to allow rx interrupt to get new characters for buffer
            __enable_uart_irq();
            while (__rx_tail == __rx_head) { } //Spinloop until there are some characters
            // Start critical section - don't interrupt while changing global buffer variables
            __disable_uart_irq();
        }
        
        lastChar = __rx_buf[__rx_tail];
        if (lastChar == '\r') //newline symbol
            line[i] = '\0';
        else
            line[i] = lastChar;
        i++;
        __rx_tail = (__rx_tail + 1) % BUFFER_SIZE;
    }
    
    //Clear flag
    __have_rx_serial = false;
    
    // End critical section 
    __enable_uart_irq();
    
    return i;
}

bool SerialManager::haveRxSerialData() {
    return __have_rx_serial;
}

void SerialManager::flush() {
    while (serial.readable()) {
        serial.getc();
    }   
}

void SerialManager::print_line(const char *line, int len) {
    if (line == NULL || len < 1)
        return;
        
    const char *curr = line;
    while (curr < line+len && *curr != '\0') {
        serial.putc(*curr);
        curr++;
    }
}

void SerialManager::__serial_rx_ISR() {
    char tmp;
    
    if (__user_fptr != NULL) //user callback
        __user_fptr->call();
    
    //Loop while the UART inbound FIFO is not empty and the receiving buffer is not full
    while (serial.readable() && (__rx_head != (__rx_tail - 1) % BUFFER_SIZE)) {
        tmp = serial.getc(); //read a byte into the buffer from the serial port
        __rx_buf[__rx_head] = tmp;
        __rx_head = (__rx_head + 1) % BUFFER_SIZE;
        if (tmp == '\r')
            __have_rx_serial = true;
    }
}

//THIS DOES NOT WORK YET
/*void SerialManager::__serial_tx_ISR() {
    //Loop while the UART outbound FIFO is not full and the transmitting buffer is not empty
    while (G_console.writeable() && (G_tx_tail != G_tx_head)) {
        G_console.putc(G_tx_buf[G_tx_tail]); //write a byte to the serial port from the buffer
        G_tx_tail = (G_tx_tail + 1) % BUFFER_SIZE;  
    }
}*/

inline void SerialManager::__disable_uart_irq() {
    // Start critical section - don't interrupt with serial I/O
    // Since user specifies UART TX/RX pins, we don't know which we are using, so disable all 3
    NVIC_DisableIRQ(UART0_IRQn);
    NVIC_DisableIRQ(UART1_IRQn);
    NVIC_DisableIRQ(UART2_IRQn);   
}

inline void SerialManager::__enable_uart_irq() {
    // End critical section - can now interrupt with serial I/O
    // Since user specifies UART TX/RX pins, we don't know which we are using, so enable all 3
    NVIC_EnableIRQ(UART0_IRQn);
    NVIC_EnableIRQ(UART1_IRQn);
    NVIC_EnableIRQ(UART2_IRQn);   
}
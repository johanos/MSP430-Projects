/*
 * uart_out.h
 *
 * Defines several functions to do output through the UART
 * Works only when interrupts are enabled!
 */

void init_USCI_UART(void); // sets up the UART
int tx_buffer_count(void); // counter of characters to transmit
int tx_start(char *, int );// starts transmission of a buffer
int tx_start_string(char *);// starts transmission of a string
void u_print_string(char *);// prints a string with busy waits before/after


#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define UBRR(BAUD) (F_CPU/16/BAUD-1)

void uart_init        (int baudrate);
void uart_write       (const void *data, int cnt);
int  uart_available   ();
char uart_read        ();
void uart_rx_handler  (void (*handler)());

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __UART_H__ */

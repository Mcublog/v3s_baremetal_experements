#ifndef V3S_UART_H
#define V3S_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

    void uart_init(void);
    void uart_putchar(char c);
    void uart_puts(const char *s);

#ifdef __cplusplus
}
#endif

#endif
#include "tremo_uart.h"
#include "stdio.h"


#ifdef __cplusplus
extern "C"
{
#endif
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

    PUTCHAR_PROTOTYPE
    {
        uart_send_data(UART0, ch);
        return ch;
    }

    int _write(int fd, char *ptr, int len)
    {
        (void)fd, (void)ptr, (void)len;
        for (size_t i = 0; i < len; i++)
        {
            uart_send_data(UART0, ptr[i]);
        }
        return len;
    }
#ifdef __cplusplus
}
#endif
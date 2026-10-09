#include <stdint.h>

volatile uint32_t app_counter = 100;
volatile uint32_t app_status;

int main(void)
{
    app_status = app_counter + 50;

    while (1)
    {
        app_counter++;
    }

    return 0;
}
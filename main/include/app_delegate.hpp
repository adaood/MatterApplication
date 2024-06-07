#ifndef APP_DELEGATE_HPP
#define APP_DELEGATE_HPP

#include <hal/gpio_types.h>
#include <cstdint>

namespace app_delegate
{
    typedef struct config
    {
        gpio_num_t factory_pin;
        uint16_t reset_hold_time_s;
        gpio_num_t restart_pin;
    } config_t;

    void init(config_t *config = nullptr, bool bridge = false);

    void start();
}
#endif // APP_DELEGATE_HPP
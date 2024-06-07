#include <app_delegate.hpp>
#include <app_device_light.hpp>
#include <app_device_fan.hpp>

extern "C" void app_main()
{
    // app_delegate::config_t config = {
    //     .factory_pin = GPIO_NUM_5,
    //     .reset_hold_time_s = 5,
    //     .restart_pin = GPIO_NUM_5,
    // };
    app_delegate::init(nullptr, true);

    app_delegate::device::on_off_light::config_t on_off_light_config = {
        .led_pin = GPIO_NUM_2,
        .button_pin = GPIO_NUM_5,
        .device_name = "Light",
    };
    app_delegate::device::on_off_light::create(&on_off_light_config);

    app_delegate::device::fan::config_t fan_config = {
        .fan_pin = GPIO_NUM_5,
        .button_pin = GPIO_NUM_5,
        .device_name = "Fan",
    };
    app_delegate::device::fan::create(&fan_config);

    app_delegate::start();
}
#ifndef APP_CALLBACK_HPP
#define APP_CALLBACK_HPP

#include <mh_matter.hpp>
#include <hal/gpio_types.h>

namespace app_delegate
{
    void app_event_cb(mh_matter::event::event_t event, void *priv_data);

    esp_err_t app_identification_cb(uint32_t device_type, uint16_t endpoint_id, void *priv_data);

    esp_err_t app_attribute_update_cb(uint32_t device_type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id,
                                      void *val, void *priv_data);

    void app_factory_button_enable(gpio_num_t factory_pin, uint16_t factory_hold_time_s);

    void app_restart_button_enable(gpio_num_t restart_pin);
} // namespace app_delegate

#endif // APP_CALLBACK_HPP
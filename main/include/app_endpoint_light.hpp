#ifndef APP_ENDPOINT_LIGHT_HPP
#define APP_ENDPOINT_LIGHT_HPP

#include <hal/gpio_types.h>
#include <cstdint>
#include <esp_matter_endpoint.h>

namespace app_delegate
{
    namespace endpoint
    {
        namespace on_off_light
        {
            const uint32_t DEVICE_TYPE_ID = ESP_MATTER_ON_OFF_LIGHT_DEVICE_TYPE_ID;

            typedef struct config
            {
                gpio_num_t led_pin;
                gpio_num_t button_pin;
                char *device_name;
            } config_t;

            void create(config_t *config = nullptr, esp_matter::endpoint_t *aggregator = nullptr);

            void updateAttribute_cb(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data);

            void reportAttribute_cb(void *endpoint_id);
        } // namespace on_off_light
    } // namespace endpoint
} // namespace app_delegate
#endif // APP_ENDPOINT_LIGHT_HPP
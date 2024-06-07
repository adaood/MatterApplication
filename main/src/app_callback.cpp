#include "app_callback.hpp"
#include "app_device_light.hpp"

#include <mh_matter.hpp>
#include <endpoint/on_off_light.hpp>

#include <on_error.hpp>
#include <iot_button.h>

void app_delegate::app_event_cb(mh_matter::event::event_t event, void *priv_data)
{
}

esp_err_t app_delegate::app_identification_cb(uint32_t device_type, uint16_t endpoint_id, void *priv_data)
{
    ESP_LOGI(__FILENAME__, "call %s", __FUNCTION__);
    if (priv_data == nullptr)
    {
        return ESP_OK;
    }
    return ESP_OK;
}

esp_err_t app_delegate::app_attribute_update_cb(uint32_t device_type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id,
                                                void *val, void *priv_data)
{
    if (val == nullptr || priv_data == nullptr)
    {
        return ESP_OK;
    }
    if (device_type == mh_matter::endpoint::on_off_light::DEVICE_TYPE_ID)
    {
        app_delegate::device::on_off_light::updateAttribute_cb(endpoint_id, cluster_id, attribute_id, static_cast<esp_matter_attr_val_t *>(val), priv_data);
    }
    return ESP_OK;
}

void app_delegate::app_factory_button_enable(gpio_num_t factory_pin, uint16_t factory_hold_time_s)
{
    factory_hold_time_s = (factory_hold_time_s * 1000);
    gpio_set_direction(factory_pin, GPIO_MODE_INPUT);
    button_config_t config = {.type = BUTTON_TYPE_GPIO,
                              .long_press_time = factory_hold_time_s,
                              .gpio_button_config = {
                                  .gpio_num = factory_pin,
                                  .active_level = 1,
                              }};
    button_handle_t handle = iot_button_create(&config);
    iot_button_register_cb(
        handle, BUTTON_LONG_PRESS_START,
        [](void *button_handle, void *usr_data)
        {
            ESP_LOGI(__FILENAME__, "Factory reset triggered");

            // TODOL: Implement the factory reset function with visual(led) feedback
            // esp_matter::factory_reset();
        },
        nullptr);
}

void app_delegate::app_restart_button_enable(gpio_num_t restart_pin)
{
    gpio_set_direction(restart_pin, GPIO_MODE_INPUT);
    button_config_t config = {.type = BUTTON_TYPE_GPIO,
                              .gpio_button_config = {
                                  .gpio_num = restart_pin,
                                  .active_level = 1,
                              }};
    button_handle_t handle = iot_button_create(&config);
    iot_button_register_cb(
        handle, BUTTON_PRESS_DOWN,
        [](void *button_handle, void *usr_data)
        {
            ESP_LOGI(__FILENAME__, "Restart device triggered");

            // TODO: Implement the restart device function.
        },
        nullptr);
}
#include "app_delegate.hpp"
#include "app_callback.hpp"

#include <nvs_flash.h>
#include <esp_log.h>
#include <endpoint/root_node.hpp>
#include <endpoint/aggregator.hpp>
#include <mh_matter.hpp>

void app_delegate::init(config_t *config, bool bridge)
{
    nvs_flash_init();

    mh_matter::set_attribute_cb(app_attribute_update_cb);
    mh_matter::set_identify_cb(app_identification_cb);

    if (config == nullptr)
    {
        ESP_LOGW(__FILENAME__, "Configuration is not provided");
    }
    else
    {
        if (!(config->factory_pin == GPIO_NUM_NC || config->reset_hold_time_s == 0))
        {
            app_factory_button_enable(config->factory_pin, config->reset_hold_time_s);
        }
        if (config->restart_pin != GPIO_NUM_NC)
        {
            app_restart_button_enable(config->restart_pin);
        }
    }

    mh_matter::endpoint::root_node::config_t node_config;
    esp_matter::node_t *node = mh_matter::endpoint::root_node::create(&node_config);

    if (bridge)
    {
        mh_matter::endpoint::aggregator::config_t aggregator_config;
        esp_matter::endpoint_t *aggregator = mh_matter::endpoint::aggregator::create(node, &aggregator_config);
    }
}

void app_delegate::start()
{
    mh_matter::start(app_delegate::app_event_cb, nullptr);
}
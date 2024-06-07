#include "app_device_fan.hpp"

#include <endpoint/fan.hpp>
#include <fanAccessory.hpp>

void app_delegate::device::fan::updateAttribute_cb(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    ESP_LOGV(__FILENAME__, "updateAttribute_cb: endpoint_id=%d, cluster_id=%d, attribute_id=%d", (int)endpoint_id, (int)cluster_id, (int)attribute_id);
    if (priv_data != nullptr && val != nullptr &&
        cluster_id == mh_matter::endpoint::fan::_CLUSTER_ID && attribute_id == mh_matter::endpoint::fan::_ATTRIBUTE_PERCENT_SETTING_ID)
    {
        FanAccessory *fanAccessory = static_cast<FanAccessory *>(priv_data);
        if (val->val.i > 0)
        {
            fanAccessory->setPower(true);
        }
        else
        {
            fanAccessory->setPower(false);
        }
    }
}

void app_delegate::device::fan::reportAttribute_cb(void *endpoint_id)
{
    if (endpoint_id == nullptr)
    {
        return;
    }
    uint16_t *id = static_cast<uint16_t *>(endpoint_id);
    FanAccessory *fanAccessory = static_cast<FanAccessory *>(esp_matter::endpoint::get_priv_data(*id));
    esp_matter_attr_val_t val_PERCENT_SETTING;
    esp_matter_attr_val_t val_PERCENT_CURRENT;
    esp_matter_attr_val_t val_FAN_MODE;

    if (fanAccessory->getPower())
    {
        val_PERCENT_SETTING = esp_matter_nullable_uint8(100);
        val_PERCENT_CURRENT = esp_matter_uint8(100);
        val_FAN_MODE = esp_matter_enum8(3);
    }
    else
    {
        val_PERCENT_SETTING = esp_matter_nullable_uint8(0);
        val_PERCENT_CURRENT = esp_matter_uint8(0);
        val_FAN_MODE = esp_matter_enum8(0);
    }

    esp_matter::attribute::report(*id, mh_matter::endpoint::fan::_CLUSTER_ID, mh_matter::endpoint::fan::_ATTRIBUTE_PERCENT_SETTING_ID, &val_PERCENT_SETTING);
    esp_matter::attribute::report(*id, mh_matter::endpoint::fan::_CLUSTER_ID, mh_matter::endpoint::fan::_ATTRIBUTE_PERCENT_CURRENT_ID, &val_PERCENT_CURRENT);
    esp_matter::attribute::report(*id, mh_matter::endpoint::fan::_CLUSTER_ID, mh_matter::endpoint::fan::_ATTRIBUTE_FAN_MODE_ID, &val_FAN_MODE);
}

void app_delegate::device::fan::create(config_t *config, esp_matter::endpoint_t *aggregator)
{
    void *priv_data = nullptr;
    if (config != nullptr)
    {
        priv_data = new FanAccessory(config->button_pin, config->fan_pin);
    }
    mh_matter::endpoint::fan::config_t fan_config;
    // fan_config.bridged_device_basic_information.node_label = config->device_name;
    esp_matter::endpoint_t *endpoint =
        mh_matter::endpoint::fan::create(esp_matter::node::get(), &fan_config, aggregator, priv_data);

    if (priv_data != nullptr)
    {
        uint16_t *endpoint_id = new uint16_t(esp_matter::endpoint::get_id(endpoint));
        static_cast<FanAccessory *>(priv_data)->setReportAttributesCallback(reportAttribute_cb, endpoint_id);

        esp_matter_attr_val_t val;
        esp_matter::cluster_t *cluster = esp_matter::cluster::get(endpoint, mh_matter::endpoint::fan::_CLUSTER_ID);
        esp_matter::attribute_t *attr = esp_matter::attribute::get(cluster, mh_matter::endpoint::fan::_ATTRIBUTE_PERCENT_CURRENT_ID);
        esp_matter::attribute::get_val(attr, &val);
        if (val.val.i > 0)
        {
            static_cast<FanAccessory *>(priv_data)->setPower(true);
        }
        else
        {
            static_cast<FanAccessory *>(priv_data)->setPower(false);
        }
    }
}

// Path: main/src/app_device_light.cpp
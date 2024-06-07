#include "app_endpoint_light.hpp"

#include <endpoint/on_off_light.hpp>
#include <lightAccessory.hpp>

void app_delegate::endpoint::on_off_light::updateAttribute_cb(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    if (priv_data != nullptr && val != nullptr &&
        cluster_id == chip::app::Clusters::OnOff::Id && attribute_id == chip::app::Clusters::OnOff::Attributes::OnOff::Id) // TODO: change id to metahouse endpoint cluster id
    {
        LightAccessory *lightAccessory = static_cast<LightAccessory *>(priv_data);
        lightAccessory->setPower(val->val.b);
    }
}

void app_delegate::endpoint::on_off_light::reportAttribute_cb(void *endpoint_id)
{
    if (endpoint_id == nullptr)
    {
        return;
    }
    uint16_t *id = static_cast<uint16_t *>(endpoint_id);
    LightAccessory *lightAccessory = static_cast<LightAccessory *>(esp_matter::endpoint::get_priv_data(*id));
    esp_matter_attr_val_t val = esp_matter_bool(lightAccessory->getPower());
    esp_matter::attribute::report(*id, chip::app::Clusters::OnOff::Id, chip::app::Clusters::OnOff::Attributes::OnOff::Id, &val);
}

void app_delegate::endpoint::on_off_light::create(config_t *config, esp_matter::endpoint_t *aggregator)
{
    void *priv_data = nullptr;
    if (config != nullptr)
    {
        priv_data = new LightAccessory(config->button_pin, config->led_pin);
    }
    mh_matter::endpoint::on_off_light::config_t on_off_light_config;
    on_off_light_config.on_off.lighting.start_up_on_off = nullptr;
    // on_off_light_config.bridged_device_basic_information.node_label = config->device_name;
    esp_matter::endpoint_t *endpoint =
        mh_matter::endpoint::on_off_light::create(esp_matter::node::get(), &on_off_light_config, aggregator, priv_data);

    if (priv_data != nullptr)
    {
        uint16_t *endpoint_id = new uint16_t(esp_matter::endpoint::get_id(endpoint));
        static_cast<LightAccessory *>(priv_data)->setReportAttributesCallback(reportAttribute_cb, endpoint_id);

        esp_matter_attr_val_t val;
        esp_matter::cluster_t *cluster = esp_matter::cluster::get(endpoint, mh_matter::endpoint::on_off_light::_CLUSTER_ID);
        esp_matter::attribute_t *attr = esp_matter::attribute::get(cluster, mh_matter::endpoint::on_off_light::_ATTRIBUTE_ID);
        esp_matter::attribute::get_val(attr, &val);

        static_cast<LightAccessory *>(priv_data)
            ->setPower(val.val.b);
    }
}

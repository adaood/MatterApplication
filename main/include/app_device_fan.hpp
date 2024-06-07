// #ifndef APP_DEVICE_FAN_HPP
// #define APP_DEVICE_FAN_HPP

// namespace app_delegate
// {
//     namespace  device
//     {
//         namespace fan
//         {
//             const uint32_t DEVICE_TYPE_ID = ESP_MATTER_FAN_DEVICE_TYPE_ID;

//             typedef struct config
//             {
//                 gpio_num_t fan_pin;
//                 gpio_num_t button_pin;
//                 char *device_name;
//             } config_t;

//             void create(config_t *config = nullptr, esp_matter::endpoint_t *aggregator = nullptr);

//             void updateAttribute_cb(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data);

//             void reportAttribute_cb(void *endpoint_id);
//         } // namespace fan
//     } // namespace  device
// } // namespace app_delegate
// #endif // APP_DEVICE_FAN_HPP
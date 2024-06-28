#include "AccessPoint.hpp"

#include <esp_err.h>
#include <esp_event.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_spiffs.h>
#include <esp_wifi.h>

#include "HelperHandler.hpp"

static const char *TAG = "AccessPoint";

AccessPoint::AccessPoint(StorageManagerInterface *storageManager) : storageManager(storageManager) {
  ESP_LOGI(TAG, "AccessPoint instance created");
  esp_err_t err = esp_event_loop_create_default();
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "Failed to create event loop: %s", esp_err_to_name(err));
    return;
  }

  err = esp_netif_init();

  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  err = esp_wifi_init(&cfg);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize wifi: %s", esp_err_to_name(err));
    return;
  }

  wifi_config_t wifi_config = {
      .ap =
          {
              .ssid = CONFIG_AP_SSID,
              .password = CONFIG_AP_PASSWORD,
              .ssid_len = strlen(CONFIG_AP_SSID),
              .channel = CONFIG_AP_CHANNEL,
              .authmode = WIFI_AUTH_WPA2_PSK,
              .max_connection = CONFIG_AP_MAX_CONNECTIONS,
              .pmf_cfg =
                  {
                      .required = true,
                  },
          },
  };

  err = esp_wifi_set_mode(WIFI_MODE_AP);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set wifi mode: %s", esp_err_to_name(err));
    return;
  }

  err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set wifi config: %s", esp_err_to_name(err));
    return;
  }

  err = esp_wifi_start();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start wifi: %s", esp_err_to_name(err));
    return;
  }

  // register event task handler for wifi events
  err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, this);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register wifi event handler: %s", esp_err_to_name(err));
  }

  ESP_LOGI(TAG, "Access point started");
}

AccessPoint::~AccessPoint() { ESP_LOGI(TAG, "AccessPoint instance destroyed"); }

esp_err_t AccessPoint::startWebServer() {
  ESP_LOGI(TAG, "Starting web server");

  httpd_uri_t uri_routes[] = {
      {.uri = "/command/unpair", .method = HTTP_GET, .handler = command_handler, .user_ctx = this},
      {.uri = "/command/factory", .method = HTTP_GET, .handler = command_handler, .user_ctx = this},
      {.uri = "/command/updatefirmware", .method = HTTP_GET, .handler = command_handler, .user_ctx = this},
      {.uri = "/command/restart", .method = HTTP_GET, .handler = command_handler, .user_ctx = this},
      {.uri = "/accessories/stored", .method = HTTP_GET, .handler = accessories_handler, .user_ctx = this},
      {.uri = "/accessories/save", .method = HTTP_POST, .handler = accessories_handler, .user_ctx = this},
      {.uri = "/wifi/stored", .method = HTTP_GET, .handler = wifi_handler, .user_ctx = this},
      {.uri = "/wifi/save", .method = HTTP_POST, .handler = wifi_handler, .user_ctx = this},
      {.uri = "/wifi/connect", .method = HTTP_PUT, .handler = wifi_handler, .user_ctx = this},
      {.uri = "/\*", .method = HTTP_GET, .handler = file_read_handler, .user_ctx = this},
  };

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  httpd_handle_t server = nullptr;

  /* Mount the filesystem */
  esp_err_t err = mount_fs("", "frontend");
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to mount filesystem: %s", esp_err_to_name(err));
    return err;
  }

  config.uri_match_fn = httpd_uri_match_wildcard;
  config.max_uri_handlers = sizeof(uri_routes) / sizeof(uri_routes[0]);
  config.stack_size = CONFIG_AP_WEB_STACK_SIZE;

  err = httpd_start(&server, &config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start server: %s", esp_err_to_name(err));
    return err;
  }

  for (uint8_t i = 0; i < config.max_uri_handlers; i++) {
    err = httpd_register_uri_handler(server, &uri_routes[i]);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register URI handler: %s", esp_err_to_name(err));
      // return err;
    }
  }

  ESP_LOGI(TAG, "Web server started");

  return ESP_OK;
}

esp_err_t AccessPoint::file_read_handler(httpd_req_t *req) {
  /* Get the URI from the request */
  const char *uri = req->uri;

  /* Check if the URI is NULL */
  if (!uri || strlen(uri) == 0) {
    ESP_LOGE(TAG, "URI is NULL");
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "URI is NULL");
    return ESP_OK;
  }

  /* Check if the URI is a file */
  if (uri[strlen(uri) - 1] == '.') {
    ESP_LOGE(TAG, "URI is INVALID");
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "URI is INVALID");
    return ESP_OK;
  }

  char filename[CONFIG_AP_MAX_PATH_SIZE];
  /* Check if the URI is a directory*/
  if (uri[0] == '/' && uri[1] == '\0') {
    strncpy(filename, "/index.html", CONFIG_AP_MAX_PATH_SIZE);
  } else {
    if (uri[strlen(uri) - 1] == '/') {
      strncpy(filename, uri, strlen(uri) - 1);
    } else {
      strncpy(filename, uri, CONFIG_AP_MAX_PATH_SIZE);
    }
  }

  ESP_LOGI(TAG, "Filename: %s", filename);

  /* Check if the file exists */
  esp_err_t err = check_file_exist(filename);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "File does not exist");
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
    return ESP_OK;
  }

  /* Set the content type of the response */
  err = set_content_type(req, filename);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set content type");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to set content type");
    return ESP_OK;
  }

  /* Stream the file to the response */
  err = stream_file(req, filename);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to stream file");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to stream file");
    return ESP_OK;
  }

  return ESP_OK;
}

esp_err_t AccessPoint::command_handler(httpd_req_t *req) {
  AccessPoint *self = (AccessPoint *)req->user_ctx;

  /* Check the method of the request */
  if (req->method == HTTP_GET) {
    /* Get the URI from the request */
    char *uri = (char *)req->uri;
    if (uri == NULL) {
      ESP_LOGE(TAG, "Failed to get the URI from the request");
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }

    /* Check the URI */
    if (strcmp(uri, "/command/unpair") == 0) {
      /* Erase all data but not the accessories, restart the device */
      if (unpair_device(self->storageManager) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase all data but not the accessories");
        httpd_resp_send_500(req);
        return ESP_FAIL;
      }

    } else if (strcmp(uri, "/command/factory") == 0) {
      /* Erase all data including accessories, restart the device */
      if (factory_reset(self->storageManager) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase all data including accessories");
        httpd_resp_send_500(req);
        return ESP_FAIL;
      }

    } else if (strcmp(uri, "/command/updatefirmware") == 0) {
      /* Update the firmware, restart the device */
      if (update_firmware(self->storageManager) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update the firmware");
        httpd_resp_send_500(req);
        return ESP_FAIL;
      }

    } else if (strcmp(uri, "/command/restart") == 0) {
      /* Restart the device */
      existProgramMode(self->storageManager);
      if (restart_device() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to restart the device");
        httpd_resp_send_500(req);
        return ESP_FAIL;
      }

    } else {
      /* Invalid URI */
      ESP_LOGE(TAG, "Invalid URI: %s", uri);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }

    /* Send the response */
    httpd_resp_send(req, "success", HTTPD_RESP_USE_STRLEN);

    return ESP_OK;

  } else {
    /* Invalid method */
    ESP_LOGE(TAG, "Invalid method: %d", req->method);
    httpd_resp_send_err(req, HTTPD_405_METHOD_NOT_ALLOWED, "Invalid method");
    return ESP_FAIL;
  }
}

esp_err_t AccessPoint::accessories_handler(httpd_req_t *req) {
  AccessPoint *self = (AccessPoint *)req->user_ctx;

  /* Check the method of the request */
  if (req->method == HTTP_GET) {
    /* Get the accessory database as a JSON string */
    char response[CONFIG_AP_ACCESSORY_JSON_SIZE];
    if (get_accessory_DB_JSON(self->storageManager, response, sizeof(response)) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to get the accessory database");
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }

    /* Send the response in format :
        {"data": <accessory database JSON>, "message": "success"}
    */
    httpd_resp_set_type(req, "application/json");

    char response_buffer[CONFIG_AP_ACCESSORY_JSON_SIZE + 64];
    snprintf(response_buffer, sizeof(response_buffer), "{\"data\": %s, \"message\": \"success\"}", response);
    httpd_resp_send(req, response_buffer, HTTPD_RESP_USE_STRLEN);

  } else if (req->method == HTTP_POST) {
    /* Get the content length of the request */
    size_t content_length = req->content_len;
    if (content_length == 0) {
      ESP_LOGE(TAG, "No content in the request");
      httpd_resp_send_408(req);
      return ESP_FAIL;
    }

    /* Allocate a buffer to store the request */
    char *buffer = (char *)malloc(content_length + 1);
    if (buffer == NULL) {
      ESP_LOGE(TAG, "Failed to allocate memory for the request");
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }

    /* Read the request */
    if (httpd_req_recv(req, buffer, content_length) <= 0) {
      ESP_LOGE(TAG, "Failed to read the request");
      httpd_resp_send_500(req);
      free(buffer);
      return ESP_FAIL;
    }

    /* Null-terminate the buffer */
    buffer[content_length] = '\0';

    /* Set the accessory database from the request */
    if (set_accessory_DB_JSON(self->storageManager, buffer) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to set the accessory database");
      httpd_resp_send_500(req);
      free(buffer);
      return ESP_FAIL;
    }

    /* Send the response
    {"data": <accessory database JSON>, "message": "success"}
    */
    httpd_resp_set_type(req, "application/json");

    char response_buffer[CONFIG_AP_ACCESSORY_JSON_SIZE + 64];
    snprintf(response_buffer, sizeof(response_buffer), "{\"data\": %s, \"message\": \"success\"}", buffer);
    httpd_resp_send(req, response_buffer, HTTPD_RESP_USE_STRLEN);
    free(buffer);
  } else {
    /* Send the response */
    httpd_resp_send_404(req);
  }
  return ESP_OK;
}

esp_err_t AccessPoint::wifi_handler(httpd_req_t *req) {
  /* Check the method of the request */
  if (req->method == HTTP_GET) {
    /* Get the stored WiFi credentials */
    char wifi_stored_SSID[] = "No Need For Matter";
    char wifi_stored_password[] = "No Need For Matter";

    /* Send the stored WiFi credentials as JSON */
    httpd_resp_set_type(req, "application/json");
    char response[100];
    // {data: {SSID: "MetaHouse", Password: "MetaHouse123"}}
    sprintf(response, "{\"data\": {\"SSID\": \"%s\", \"PASSWORD\": \"%s\"}}", wifi_stored_SSID,
            wifi_stored_password);

    httpd_resp_send(req, response, strlen(response));

    return ESP_OK;
  } else if (req->method == HTTP_POST) {
    /* Save the WiFi credentials */
    /* Respond success as JSON*/
    httpd_resp_set_type(req, "application/json");
    char response[100];
    sprintf(response, "{\"message\": \"success\"}");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
  } else if (req->method == HTTP_PUT) {
    /* Try to connect to WiFi */
    /* Respond success as JSON data "" and message success*/
    httpd_resp_set_type(req, "application/json");
    char response[100];
    sprintf(response, "{\"data\": \"\", \"message\": \"success\"}");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
  }
  return ESP_OK;
}

void AccessPoint::change_led_status(int32_t event_id) {
  // change the led status
}

void AccessPoint::wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                                     void *event_data) {
  if (event_base == WIFI_EVENT) {
    AccessPoint *self = (AccessPoint *)arg;

    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
      ESP_LOGI(TAG, "station connected");
      self->change_led_status(WIFI_EVENT_AP_STACONNECTED);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
      ESP_LOGI(TAG, "station disconnected");
      self->change_led_status(WIFI_EVENT_AP_STADISCONNECTED);
    }
  }
}
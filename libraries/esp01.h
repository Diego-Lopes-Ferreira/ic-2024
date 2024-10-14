#include <stdio.h>
#include <string.h>

#include "dd_usart.h"
#include "stm32g4xx.h"  // Register names, NVIC macros and IRQ declarations

typedef enum Esp_State_t {
  ESP_STATE_SETUP_AT_CMD,              // 01
  ESP_STATE_SETUP_AT_OK,               // 02
  ESP_STATE_SETUP_CWMODE_SET_CMD,      // 03
  ESP_STATE_SETUP_CWMODE_SET_OK,       // 04
  ESP_STATE_SETUP_CWMODE_CHECK_CMD,    // 05
  ESP_STATE_SETUP_CWMODE_CHECK_VALUE,  // 06
  ESP_STATE_SETUP_CWMODE_CHECK_OK,     // 07

  ESP_STATE_WIFI_AT_CMD,           // 08
  ESP_STATE_WIFI_AT_OK,            // 09
  ESP_STATE_WIFI_CWJAP_CMD,        // 10
  ESP_STATE_WIFI_CWJAP_OK,         // 11
  ESP_STATE_WIFI_CIPSTATUS_CMD,    // 12
  ESP_STATE_WIFI_CIPSTATUS_VALUE,  // 13
  ESP_STATE_WIFI_CIPSTATUS_OK,     // 14

  ESP_STATE_HTTP_wait_for_timer,   // 15
  ESP_STATE_HTTP_AT_CMD,           // 16
  ESP_STATE_HTTP_AT_OK,            // 17
  ESP_STATE_HTTP_CIPSTATUS_CMD,    // 18
  ESP_STATE_HTTP_CIPSTATUS_VALUE,  // 19
  ESP_STATE_HTTP_CIPSTATUS_OK,     // 20
  ESP_STATE_HTTP_CIPSTART_CMD,     // 21
  ESP_STATE_HTTP_CIPSTART_OK,      // 22
  ESP_STATE_HTTP_copy_adc,         // 23
  ESP_STATE_HTTP_CIPSEND_CMD,      // 24
  ESP_STATE_HTTP_CIPSEND_ARROW,    // 25
  ESP_STATE_HTTP_CIPSEND_MESSAGE,  // 26
  ESP_STATE_HTTP_CIPSEND_SEND_OK,  // 27
  ESP_STATE_HTTP_CIPSEND_OK        // 28
} Esp_State_t;

typedef struct {
  float V;        // Voltage (rms)
  float I;        // Current (rms)
  float S;        // Aparent power (VA)
  float P;        // Active power (W)
  float Q;        // Reactive power (VAr)
  float PF;       // Power factor
  float freq_hz;  // Frequency (hz)
} CycleInformation_t;

void ESP01_increment_state(Esp_State_t *state);
void ESP01_handle_line(char line[], Esp_State_t *state);
void ESP01_handle_state(Esp_State_t *state);
void ESP01_handle_send_http(CycleInformation_t cycle_info, float *voltage_list, float *current_list);

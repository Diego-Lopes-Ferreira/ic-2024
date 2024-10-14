#include "esp01.h"
void ESP01_increment_state(Esp_State_t *state) {
  if (*state >= ESP_STATE_HTTP_CIPSEND_OK) {
    *state = ESP_STATE_HTTP_wait_for_timer;
  } else {
    *state += 1;
  }
}

void ESP01_handle_line(char line[], Esp_State_t *state) {
  if (*state == ESP_STATE_SETUP_AT_OK ||            //
      *state == ESP_STATE_SETUP_CWMODE_SET_OK ||    //
      *state == ESP_STATE_SETUP_CWMODE_CHECK_OK ||  //
      *state == ESP_STATE_WIFI_AT_OK ||             //
      *state == ESP_STATE_WIFI_CWJAP_OK ||          //
      *state == ESP_STATE_WIFI_CIPSTATUS_OK ||      //
      *state == ESP_STATE_HTTP_AT_OK ||             //
      *state == ESP_STATE_HTTP_CIPSTATUS_OK ||      //
      *state == ESP_STATE_HTTP_CIPSTART_OK          //

  ) {
    if (strcmp(line, "OK\r\n") == 0) { ESP01_increment_state(state); };
  }

  if (*state == ESP_STATE_HTTP_CIPSEND_OK) {
    if (strcmp(line, "OK\r\n") == 0) { *state = ESP_STATE_HTTP_wait_for_timer; };
  }

  if (*state == ESP_STATE_HTTP_CIPSEND_SEND_OK) {  //
    if (strcmp(line, "SEND OK\r\n") == 0) { ESP01_increment_state(state); };
  }

  if (*state == ESP_STATE_HTTP_CIPSEND_ARROW) {  //
    if (strcmp(line, ">") == 0) { ESP01_increment_state(state); };
  }

  if (*state == ESP_STATE_SETUP_CWMODE_CHECK_VALUE) {  //
    if (strcmp(line, "+CWMODE:1\r\n") == 0) { ESP01_increment_state(state); };
  }

  if (*state == ESP_STATE_WIFI_CIPSTATUS_VALUE || *state == ESP_STATE_HTTP_CIPSTATUS_VALUE) {  //
    if (strcmp(line, "STATUS:2\r\n") == 0) { ESP01_increment_state(state); };
  }

  // NOTE(dd): Sugestion
  if (*state == ESP_STATE_HTTP_CIPSTATUS_VALUE) {  // Check status first
    if (strncmp(line, "STATUS:", 7) == 0) {         // If message is the status
      if (strlen(line) >= 9 && line[8] == '2') {   // Check for number 2
        ESP01_increment_state(state);              // Go to the next step
      } else {                                     // Otherwise
        *state = ESP_STATE_HTTP_CIPSTATUS_CMD;     // Go back to the command
      }
    }
  }
}

void ESP01_handle_state(Esp_State_t *state) {
  char msg_buffer[256];
  memset(msg_buffer, 0, sizeof(msg_buffer));  // Reset buffer

  if (*state == ESP_STATE_SETUP_AT_CMD ||  //
      *state == ESP_STATE_WIFI_AT_CMD ||   //
      *state == ESP_STATE_HTTP_AT_CMD      //
  ) {
    sprintf(msg_buffer, "AT\r\n");                                      // Write command
    dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer
    ESP01_increment_state(state);
  }

  else if (*state == ESP_STATE_SETUP_CWMODE_SET_CMD) {
    sprintf(msg_buffer, "AT+CWMODE_CUR=1\r\n");                         // Write command
    dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer
    ESP01_increment_state(state);
  }

  else if (*state == ESP_STATE_SETUP_CWMODE_CHECK_CMD) {
    sprintf(msg_buffer, "AT+CWMODE?\r\n");                              // Write command
    dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer
    ESP01_increment_state(state);
  }

  else if (*state == ESP_STATE_WIFI_CWJAP_CMD) {
    sprintf(msg_buffer, "AT+CWJAP=\"dnotebook\",\"senhasenha\"\r\n");   // Write command
    dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer
    ESP01_increment_state(state);
  }

  else if (*state == ESP_STATE_WIFI_CIPSTATUS_CMD || *state == ESP_STATE_HTTP_CIPSTATUS_CMD) {
    sprintf(msg_buffer, "AT+CIPSTATUS\r\n");                            // Write command
    dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer
    ESP01_increment_state(state);
  }

  else if (*state == ESP_STATE_HTTP_CIPSTART_CMD) {
    sprintf(msg_buffer, "AT+CIPSTART=\"TCP\",\"192.168.137.1\",3000\r\n");  // Write command
    dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));      // Send buffer
    ESP01_increment_state(state);
  }

  else if (*state == ESP_STATE_HTTP_CIPSEND_CMD) {
    // sprintf(msg_buffer, "AT+CIPSEND=%d\r\n", 159);                      // Write command
    sprintf(msg_buffer, "AT+CIPSEND=%d\r\n", 318);                      // Write command
    dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer
    ESP01_increment_state(state);
  }
}

// void ESP01_handle_send_http(CycleInformation_t cycle_info, float *voltage_list, float *current_list) {
//   char msg_buffer[256];
//   memset(msg_buffer, 0, sizeof(msg_buffer));  // Reset buffer
//   sprintf(msg_buffer,
//           "POST / HTTP/1.1\r\nHost: 192.168.137.1:3000\r\nUser-Agent: ESP01/2024.8.19\r\nContent-Type: application/json\r\nAccept: "
//           "*/*\r\nContent-Length: 16\r\n\r\n{\"msg\": \"teste\"}\r\n\r\n");  // Write message
//   dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));         // Send buffer
//   memset(msg_buffer, 0, sizeof(msg_buffer));                                 // Reset buffer
//   sprintf(msg_buffer, "{\"msg\": \"hello from esp01\"}\r\n\r\n");            // Write message
//   dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));         // Send buffer
// }

void ESP01_handle_send_http(CycleInformation_t cycle_info, float *voltage_list, float *current_list) {
  // Sending: {"device_id": 2,"voltage_list": [],"current_list": []}
  char msg_buffer[256];
  uint16_t i;

  memset(msg_buffer, 0, sizeof(msg_buffer));  // Reset buffer
  sprintf(msg_buffer,
          "POST / HTTP/1.1\r\nHost: 192.168.137.1:3000\r\nUser-Agent: ESP01/2024.8.19\r\nContent-Type: application/json\r\nAccept: "
          "*/*\r\nContent-Length: 176 \r\n\r\n");
  dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer

  // ===== TO THE CLOUD
  memset(msg_buffer, 0, sizeof(msg_buffer));                          // Reset buffer
  sprintf(msg_buffer, "{\"device_id\":2,");                           // Write message
  dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer

  memset(msg_buffer, 0, sizeof(msg_buffer));                          // Reset buffer
  sprintf(msg_buffer, "\"voltage_rms\":%10.6f,", cycle_info.V);       // Write message
  dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer

  memset(msg_buffer, 0, sizeof(msg_buffer));                          // Reset buffer
  sprintf(msg_buffer, "\"voltage_hz\":%10.6f,", cycle_info.freq_hz);  // Write message
  dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer

  memset(msg_buffer, 0, sizeof(msg_buffer));                          // Reset buffer
  sprintf(msg_buffer, "\"current_rms\":%10.6f,", cycle_info.I);       // Write message
  dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer

  memset(msg_buffer, 0, sizeof(msg_buffer));                          // Reset buffer
  sprintf(msg_buffer, "\"power_s\":%11.6f,", cycle_info.S);           // Write message
  dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer

  memset(msg_buffer, 0, sizeof(msg_buffer));                          // Reset buffer
  sprintf(msg_buffer, "\"power_p\":%11.6f,", cycle_info.P);           // Write message
  dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer

  memset(msg_buffer, 0, sizeof(msg_buffer));                          // Reset buffer
  sprintf(msg_buffer, "\"power_q\":%11.6f,", cycle_info.Q);           // Write message
  dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer

  memset(msg_buffer, 0, sizeof(msg_buffer));                           // Reset buffer
  sprintf(msg_buffer, "\"power_pf\":% 9.6f}\r\n\r\n", cycle_info.PF);  // Write message
  dd_usart_write(USART1, (uint8_t *)msg_buffer, strlen(msg_buffer));   // Send buffer

  // ===== TO THE COMPUTER VIA SERIAL
  // memset(msg_buffer, 0, sizeof(msg_buffer));                           // Reset buffer
  // sprintf(msg_buffer, "{\"voltage_list\": [");                         // Write message
  // dd_usart_write(LPUART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer

  // // The numbers (voltage)
  // for (i = 0; i < 512; i++) {
  //   memset(msg_buffer, 0, sizeof(msg_buffer));  // Reset buffer
  //   if (voltage_list[i] > 999.0 || voltage_list[i] < -999.0) { voltage_list[i] = 0.0; }
  //   (i != 512) ? sprintf(msg_buffer, "% 11.6f,", voltage_list[i]) : sprintf(msg_buffer, "%.6f", voltage_list[i]);
  //   dd_usart_write(LPUART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer
  // }

  // memset(msg_buffer, 0, sizeof(msg_buffer));                           // Reset buffer
  // sprintf(msg_buffer, "],\"current_list\": [");                        // Write message
  // dd_usart_write(LPUART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer

  // for (i = 0; i < 512; i++) {
  //   memset(msg_buffer, 0, sizeof(msg_buffer));  // Reset buffer
  //   if (current_list[i] > 999.0 || current_list[i] < -999.0) { current_list[i] = 0.0; }
  //   (i != 512) ? sprintf(msg_buffer, "% 11.6f,", current_list[i]) : sprintf(msg_buffer, "%.6f", current_list[i]);
  //   dd_usart_write(LPUART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer
  // }

  // memset(msg_buffer, 0, sizeof(msg_buffer));                           // Reset buffer
  // sprintf(msg_buffer, "]}\r\n\r\n");                                   // Write message
  // dd_usart_write(LPUART1, (uint8_t *)msg_buffer, strlen(msg_buffer));  // Send buffer
}

/*
# SETUP
ESP_STATE_SETUP_AT_CMD
ESP_STATE_SETUP_AT_OK
ESP_STATE_SETUP_CWMODE_SET_CMD
ESP_STATE_SETUP_CWMODE_SET_OK
ESP_STATE_SETUP_CWMODE_CHECK_CMD
ESP_STATE_SETUP_CWMODE_CHECK_VALUE
ESP_STATE_SETUP_CWMODE_CHECK_OK

# WIFI CONNECT
ESP_STATE_WIFI_AT_CMD
ESP_STATE_WIFI_AT_OK
ESP_STATE_WIFI_CWJAP_CMD
ESP_STATE_WIFI_CWJAP_OK
ESP_STATE_WIFI_CIPSTATUS_CMD
ESP_STATE_WIFI_CIPSTATUS_VALUE
ESP_STATE_WIFI_CIPSTATUS_OK

# SEND HTTP
ESP_STATE_HTTP_AT_CMD
ESP_STATE_HTTP_AT_OK
ESP_STATE_HTTP_CIPSTATUS_CMD
ESP_STATE_HTTP_CIPSTATUS_VALUE
ESP_STATE_HTTP_CIPSTATUS_OK
ESP_STATE_HTTP_CIPSTART_CMD
ESP_STATE_HTTP_CIPSTART_OK
ESP_STATE_HTTP_CIPSEND_CMD
ESP_STATE_HTTP_CIPSEND_ARROW
ESP_STATE_HTTP_CIPSEND_MESSAGE
ESP_STATE_HTTP_CIPSEND_SEND_OK
ESP_STATE_HTTP_CIPSEND_OK
*/

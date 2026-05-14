#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>
#include <esp_event.h>

void wifi_start_sta(const char *ssid, const char *passwd);
void wifi_start_ap(const char *ap_ssid, const char *ap_passwd);
void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

#endif

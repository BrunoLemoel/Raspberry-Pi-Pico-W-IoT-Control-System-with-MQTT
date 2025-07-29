#ifndef MAIN_H
#define MAIN_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/dns.h"
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "cyw43.h"
#include "cyw43_country.h"
#include "lwip/apps/mqtt.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/mqtt_priv.h"
#include "hivemq_config.h"

// Definições WiFi usando valores corretos do CYW43
#ifndef CYW43_AUTH_WPA2_AES_PSK
#define CYW43_AUTH_WPA2_AES_PSK CYW43_AUTH_WPA2_MIXED_PSK
#endif

#ifndef CYW43_AUTH_WPA2_MIXED_PSK  
#define CYW43_AUTH_WPA2_MIXED_PSK 0x00400004
#endif

/* MACROS MQTT */
#define DEBUG_printf printf
#define MQTT_SERVER_HOST HIVEMQ_HOST
#define MQTT_SERVER_PORT HIVEMQ_PORT
#define MQTT_TLS 1  // Habilitar TLS para validação do certificado
#define MQTT_USERNAME HIVEMQ_USERNAME
#define MQTT_PASSWORD HIVEMQ_PASSWORD
#define BUFFER_SIZE 256
#define CLIENT_ID HIVEMQ_CLIENT_ID

/* ESTRUTURAS */
typedef struct MQTT_CLIENT_T_ {
    ip_addr_t remote_addr;
    mqtt_client_t *mqtt_client;
    u32_t received;
    u32_t counter;
    u32_t reconnect;
} MQTT_CLIENT_T;

/* FUNÇÕES */
void run_dns_lookup(void);
void mqtt_run_test(void);

#endif // MAIN_H


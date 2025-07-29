#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "cyw43.h"
#include "cyw43_country.h"
#include "lwip/dns.h"
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/apps/mqtt.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/mqtt_priv.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "hivemq_config.h"
#include "ca_cert.h"

// Definições dos pinos
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define JOYSTICK_PIN 22

// Definições dos LEDs RGB
#define LED_RED_PIN 13
#define LED_GREEN_PIN 11
#define LED_BLUE_PIN 12

// Variáveis globais para DNS e MQTT
ip_addr_t mqtt_ip;
volatile bool mqtt_connected = false;
volatile bool mqtt_connection_failed = false;
mqtt_client_t *global_client = NULL;

// Credenciais WiFi (já definidas pelo CMakeLists.txt)
// WIFI_SSID e WIFI_PASSWORD são definidos automaticamente

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("Conexão MQTT aceita\n");
        mqtt_connected = true;
    } else {
        printf("Falha na conexão MQTT, status: %d\n", status);
        mqtt_connection_failed = true;
    }
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("Mensagem recebida no tópico %s com tamanho total %u\n", topic, (unsigned int)tot_len);
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("Dados da mensagem recebida com tamanho %d, flags %u\n", len, (unsigned int)flags);
    
    // Buffer para armazenar a mensagem recebida
    char message[64];
    if (len >= sizeof(message)) {
        len = sizeof(message) - 1;
    }
    memcpy(message, data, len);
    message[len] = '\0';
    
    printf("Mensagem recebida: %s\n", message);
    
    // Processar comandos de LED
    if (strncmp(message, "LED_RED:", 8) == 0) {
        if (strcmp(message + 8, "ON") == 0) {
            gpio_put(LED_RED_PIN, 1);
            printf("LED Vermelho LIGADO\n");
        } else if (strcmp(message + 8, "OFF") == 0) {
            gpio_put(LED_RED_PIN, 0);
            printf("LED Vermelho DESLIGADO\n");
        }
    }
    else if (strncmp(message, "LED_GREEN:", 10) == 0) {
        if (strcmp(message + 10, "ON") == 0) {
            gpio_put(LED_GREEN_PIN, 1);
            printf("LED Verde LIGADO\n");
        } else if (strcmp(message + 10, "OFF") == 0) {
            gpio_put(LED_GREEN_PIN, 0);
            printf("LED Verde DESLIGADO\n");
        }
    }
    else if (strncmp(message, "LED_BLUE:", 9) == 0) {
        if (strcmp(message + 9, "ON") == 0) {
            gpio_put(LED_BLUE_PIN, 1);
            printf("LED Azul LIGADO\n");
        } else if (strcmp(message + 9, "OFF") == 0) {
            gpio_put(LED_BLUE_PIN, 0);
            printf("LED Azul DESLIGADO\n");
        }
    }
    else if (strcmp(message, "LED_ALL_ON") == 0) {
        gpio_put(LED_RED_PIN, 1);
        gpio_put(LED_GREEN_PIN, 1);
        gpio_put(LED_BLUE_PIN, 1);
        printf("Todos os LEDs LIGADOS\n");
    }
    else if (strcmp(message, "LED_ALL_OFF") == 0) {
        gpio_put(LED_RED_PIN, 0);
        gpio_put(LED_GREEN_PIN, 0);
        gpio_put(LED_BLUE_PIN, 0);
        printf("Todos os LEDs DESLIGADOS\n");
    }
}

// Função para inicializar os pinos dos botões e joystick
void init_buttons_and_joystick(void) {
    // Configurar botões como entrada com pull-up
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
    
    // Configurar joystick como entrada com pull-up
    gpio_init(JOYSTICK_PIN);
    gpio_set_dir(JOYSTICK_PIN, GPIO_IN);
    gpio_pull_up(JOYSTICK_PIN);
    
    printf("Botões e joystick inicializados\n");
}

// Função para inicializar os LEDs RGB
void init_rgb_leds(void) {
    // Configurar LEDs como saída
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_put(LED_RED_PIN, 0);  // Iniciar desligado
    
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, 0);  // Iniciar desligado
    
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, 0);  // Iniciar desligado
    
    printf("LEDs RGB inicializados (Vermelho: Pino %d, Verde: Pino %d, Azul: Pino %d)\n", 
           LED_RED_PIN, LED_GREEN_PIN, LED_BLUE_PIN);
}

// Função para ler o estado dos botões e joystick
void read_button_states(bool *button_a, bool *button_b, bool *joystick) {
    // Ler estados (invertido porque usamos pull-up, então pressionado = LOW = 0)
    *button_a = !gpio_get(BUTTON_A_PIN);
    *button_b = !gpio_get(BUTTON_B_PIN);
    *joystick = !gpio_get(JOYSTICK_PIN);
}

// Função para publicar estado no MQTT
void publish_button_state(const char* topic, bool state) {
    if (!global_client || !mqtt_connected) {
        return;
    }
    
    const char* message = state ? "TRUE" : "FALSE";
    err_t pub_err = mqtt_publish(global_client, topic, message, strlen(message), 0, 0, NULL, NULL);
    
    if (pub_err == ERR_OK) {
        printf("Publicado %s: %s\n", topic, message);
    } else {
        printf("Falha ao publicar %s, erro: %d\n", topic, pub_err);
    }
}

void run_dns_lookup(void) {
    printf("Procurando endereço IP para %s...\n", HIVEMQ_HOST);
    err_t dns_err = dns_gethostbyname(HIVEMQ_HOST, &mqtt_ip, NULL, NULL);
    
    if (dns_err == ERR_OK) {
        printf("DNS resolvido com sucesso\n");
    } else if (dns_err == ERR_INPROGRESS) {
        printf("Resolução DNS em andamento, aguardando...\n");
        for (int i = 0; i < 100; i++) {
            sleep_ms(100);
            cyw43_arch_poll();
            dns_err = dns_gethostbyname(HIVEMQ_HOST, &mqtt_ip, NULL, NULL);
            if (dns_err == ERR_OK) {
                printf("DNS resolvido após aguardar\n");
                break;
            }
        }
    }
    
    if (dns_err != ERR_OK) {
        printf("Falha na resolução DNS: %d\n", dns_err);
        return;
    }
    
    printf("IP do servidor: %s\n", ip4addr_ntoa(ip_2_ip4(&mqtt_ip)));
}

void mqtt_run_test(void) {
    printf("Iniciando Teste de Conexão MQTT Simples\n");
    printf("Conectando em: %s:%d\n", HIVEMQ_HOST, HIVEMQ_PORT);
    printf("ID do Cliente: %s\n", HIVEMQ_CLIENT_ID);
    
    mqtt_client_t *client = mqtt_client_new();
    if (!client) {
        printf("Falha ao criar cliente MQTT\n");
        return;
    }

    // Salvar referência global do cliente
    global_client = client;

    // Configuração básica do cliente MQTT
    struct mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci));
    ci.client_id = HIVEMQ_CLIENT_ID;
    ci.client_user = NULL;  // Sem autenticação
    ci.client_pass = NULL;  // Sem autenticação
    ci.keep_alive = 60;
    ci.will_topic = NULL;
    ci.will_msg = NULL;
    ci.will_retain = 0;
    ci.will_qos = 0;

    // Teste sem TLS primeiro para isolar o problema
    printf("Testando sem TLS para resolução de problemas...\n");

    // Resolver hostname primeiro
    printf("Resolvendo hostname %s...\n", HIVEMQ_HOST);
    err_t dns_err = dns_gethostbyname(HIVEMQ_HOST, &mqtt_ip, NULL, NULL);
    
    if (dns_err == ERR_INPROGRESS) {
        printf("Resolução DNS em andamento, aguardando...\n");
        for (int i = 0; i < 50; i++) {
            sleep_ms(100);
            cyw43_arch_poll();
            dns_err = dns_gethostbyname(HIVEMQ_HOST, &mqtt_ip, NULL, NULL);
            if (dns_err == ERR_OK) {
                printf("DNS resolvido!\n");
                break;
            }
        }
    }
    
    if (dns_err != ERR_OK) {
        printf("Falha na resolução DNS: %d\n", dns_err);
        mqtt_client_free(client);
        global_client = NULL;
        return;
    }
    
    printf("IP do Servidor: %s\n", ip4addr_ntoa(ip_2_ip4(&mqtt_ip)));

    // Conectar usando IP resolvido SEM TLS
    printf("Tentando conexão MQTT sem TLS...\n");
    err_t err = mqtt_client_connect(client, &mqtt_ip, HIVEMQ_PORT, 
                                   mqtt_connection_cb, NULL, &ci);

    if (err != ERR_OK) {
        printf("Falha na conexão MQTT com erro: %d\n", err);
        mqtt_client_free(client);
        global_client = NULL;
        return;
    }

    // Aguardar conexão
    int timeout = 0;
    while (!mqtt_connected && !mqtt_connection_failed && timeout < 100) {
        sleep_ms(100);
        cyw43_arch_poll();
        timeout++;
        if (timeout % 10 == 0) {
            printf("Aguardando conexão... %d segundos\n", timeout / 10);
        }
    }

    if (mqtt_connection_failed) {
        printf("Falha na conexão MQTT!\n");
        mqtt_client_free(client);
        global_client = NULL;
        return;
    }

    if (!mqtt_connected) {
        printf("Timeout na conexão MQTT!\n");
        mqtt_client_free(client);
        global_client = NULL;
        return;
    }

    printf("MQTT conectado com sucesso!\n");

    // Configurar callbacks para publish
    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);

    // Inscrever-se no tópico de comandos de LED
    printf("Inscrevendo-se no tópico de comandos LED...\n");
    err_t sub_err = mqtt_subscribe(client, "pico/led_command", 0, NULL, NULL);
    if (sub_err == ERR_OK) {
        printf("Inscrito com sucesso em pico/led_command\n");
    } else {
        printf("Falha ao se inscrever em pico/led_command, erro: %d\n", sub_err);
    }

    // Publicar mensagem inicial
    const char* message = "Monitor de Botões e Controlador de LED do Pico W Iniciado!";
    printf("Publicando mensagem de inicialização...\n");
    mqtt_publish(client, "pico/status", message, strlen(message), 0, 0, NULL, NULL);

    printf("Iniciando loop de monitoramento de botões...\n");
    
    // Loop principal de monitoramento
    int loop_count = 0;
    while (mqtt_connected) {
        // Ler estados atuais
        bool button_a, button_b, joystick;
        read_button_states(&button_a, &button_b, &joystick);
        
        // Publicar status a cada 1 segundo (10 * 100ms)
        if (loop_count % 10 == 0) {
            publish_button_state("pico/button_a", button_a);
            publish_button_state("pico/button_b", button_b);
            publish_button_state("pico/joystick", joystick);
        }
        
        // Enviar heartbeat a cada 30 segundos
        loop_count++;
        if (loop_count % 300 == 0) {  // 300 * 100ms = 30 segundos
            mqtt_publish(client, "pico/heartbeat", "ALIVE", 5, 0, 0, NULL, NULL);
            printf("Heartbeat enviado (loop %d)\n", loop_count / 300);
        }
        
        // Aguardar e processar rede
        sleep_ms(100);
        cyw43_arch_poll();
        
        // Verificar se ainda está conectado
        if (mqtt_connection_failed) {
            printf("Conexão MQTT perdida!\n");
            break;
        }
    }

    printf("Desconectando do broker MQTT...\n");
    mqtt_disconnect(client);
    mqtt_client_free(client);
    global_client = NULL;
    printf("Monitoramento MQTT finalizado!\n");
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Falha ao inicializar WiFi\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    printf("Conectando ao WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Falha ao conectar ao WiFi\n");
        return 1;
    } else {
        printf("WiFi conectado com sucesso!\n");
    }

    // Inicializar botões e joystick
    printf("Inicializando botões e joystick...\n");
    init_buttons_and_joystick();

    // Inicializar LEDs RGB
    printf("Inicializando LEDs RGB...\n");
    init_rgb_leds();

    // Conectar ao MQTT e iniciar monitoramento
    mqtt_run_test();

    cyw43_arch_deinit();
    return 0;
}

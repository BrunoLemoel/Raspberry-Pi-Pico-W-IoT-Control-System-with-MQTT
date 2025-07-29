#ifndef HIVEMQ_CONFIG_H
#define HIVEMQ_CONFIG_H

// Configurações para mqtt-dashboard.com
// Broker MQTT público que suporta TLS na porta 8884

// Host do broker MQTT
#ifndef HIVEMQ_HOST
#define HIVEMQ_HOST "mqtt-dashboard.com"
#endif

// Porta não-TLS para teste inicial
#ifndef HIVEMQ_PORT
#define HIVEMQ_PORT 1883
#endif

// Credenciais de autenticação (mqtt-dashboard.com é público, não requer auth)
#ifndef HIVEMQ_USERNAME
#define HIVEMQ_USERNAME ""
#endif

#ifndef HIVEMQ_PASSWORD
#define HIVEMQ_PASSWORD ""
#endif

// Cliente ID único fornecido
#ifndef HIVEMQ_CLIENT_ID
#define HIVEMQ_CLIENT_ID "clientId-RALBqlP6QP"
#endif

// Configurações de tópicos MQTT
#define TOPIC_PUBLISH "pico/data"
#define TOPIC_SUBSCRIBE "pico/commands"
#define TOPIC_STATUS "pico/status"

// Configurações de QoS
#define MQTT_QOS_LEVEL 1

// Keep alive em segundos
#define MQTT_KEEP_ALIVE 60

#endif // HIVEMQ_CONFIG_H

// COMM BAUDS
#define BAUDS 115200

#define LOGGER_ENABLED            // Comentar para deshabilitar el logger por consola serie

#define LOG_LEVEL TRACE           // nivells en c_logger: TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NONE

// DEVICE
//#define DEVICE_ESP_ID             "54CE0361421"   // ESP32 ID
#define DEVICE_GIIROB_PR2_ID      "00" //"giirobpr2_00"

// WIFI
#define NET_SSID                  "UPV-PSK"
#define NET_PASSWD                "giirob-pr2-2023"
/*CAMBIAR EL WIFI PARA PROBALO EN CASA*/


// MQTT
#define MQTT_SERVER_IP            "broker.emqx.io"
#define MQTT_SERVER_PORT          1883
#define MQTT_USERNAME             "giirob222"   // Descomentar esta línea (y la siguiente) para que se conecte al broker MQTT usando usuario y contraseña
#define MQTT_PASSWORD             "UPV2024"


#define STATION_COMMAND_TOPIC "giirob/pr2/station/mesa/status" //leer
#define STATION_STATUS_TOPIC "giirob/pr2/station/mesa/commands"  //enviar


//mensajes que se envian

#define MENSAJE_DETECTADO "sensor_detecta"
#define MENSAJE_NO_DETECTADO "sensor_no_detecta"

#define ENVIAR_SENYAL "SensorMqtt"
#define ENVIAR_SENYAL_NO_DETECTADO "NotSensorMqtt"

enum estado_sensor
{
  DESCONOCIDO,
  DETECTADO,
  NO_DETECTADO
};

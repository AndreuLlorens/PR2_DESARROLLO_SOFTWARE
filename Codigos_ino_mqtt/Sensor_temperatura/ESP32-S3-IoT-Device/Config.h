// COMM BAUDS
#include<Arduino.h>
#include<Wire.h>
#include<LiquidCrystal_I2C.h>


#define BAUDS 115200

#define LOGGER_ENABLED            // Comentar para deshabilitar el logger por consola serie

#define LOG_LEVEL TRACE           // nivells en c_logger: TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NONE

// DEVICE
//#define DEVICE_ESP_ID             "54CE0361421"   // ESP32 ID
#define DEVICE_GIIROB_PR2_ID      "01" //"giirobpr2_00"

// WIFI
#define NET_SSID                  "UPV-PSK"
#define NET_PASSWD                "giirob-pr2-2023"

// MQTT
#define MQTT_SERVER_IP            "broker.emqx.io"
#define MQTT_SERVER_PORT          1883
#define MQTT_USERNAME             "giirob222"   // Descomentar esta línea (y la siguiente) para que se conecte al broker MQTT usando usuario y contraseña
#define MQTT_PASSWORD             "UPV2024"

#define STATION_COMMAND_TOPIC "giirob/pr2/station/mesa/status" //leer
#define STATION_STATUS_TOPIC "giirob/pr2/station/mesa/commands"  //enviar

//mensajes que enviamos 
#define ENVIAR_MENSAJE_TEMEPRATURA_ALTA "temperatura_alta"
#define ENVIAR_MENSAJE_TEMPERATURA_ACEPTABLE "temperatura_correcta"

#define MENSAJE_ACTIVAR_SETA "EMERGENCIA"
#define MENSAJE_DESACTIVAR_SETA "LIBERANDO"

#define MENSAJE_STOP_REMOTO "STOP"
#define MENSAJE_START_REMOTO "START"


#define BUFFER_SIZE 10 //tamaño del buffer

#define RANGO_ANALOGICO_MAX 4095
#define RANGO_ANALOGICO_MIN 0

#define TIEMPO_ESPERA_PUT 1000
#define TIEMPO_ESPERA_GET 1000
#define TIEMPO_ESPERA_LISTAR 1500
#define TIEMPO_ESPERA_MEDIR_NTC 1000
#define TIEMPO_PAUSA_DE_TAREAS 1000

#define PIN_NTC 19 //pin NTC
#define PIN_SDA 4 //pines para la lcd
#define PIN_SCL 5
#define PIN_LED_ROJO 20
#define PIN_LED_AZUL 48
#define PIN_BOTON_EMERGENCIA 13

#define PUT_ITEM_TASK_PRIORITY 1
#define GET_ITEM_TASK_PRIORITY 1
#define LISTAR_BUFFER_TASK_PRIORITY 1
#define MEDIR_NTC_TASK_PRIORITY 1
#define MOSTRAR_LCD_TASK_PRIORITY 1

//variables globales para la medicion de la NTC. NOTA: MIRAR ESQUEMATICO
const float Vcc = 3.3f;
const float R0 = 10000.f;
const float beta = 3950.0f;
const float T0 = 298.15f; //Temperatura ambiente que suponemos (25ºC)
const float K = 273.15f; //para pasar a celsius

/*VARIABLES DE LAS TAREAS*/
static TaskHandle_t put_item_task_handle = NULL;
static TaskHandle_t get_item_task_handle = NULL;
static TaskHandle_t Listar_Buffer_task_handle = NULL;
static TaskHandle_t Medir_ntc_task_handle = NULL;
static TaskHandle_t mostar_lcd_task_handle = NULL;

portMUX_TYPE temperature_buffer_access = portMUX_INITIALIZER_UNLOCKED; //mutex para el item, para nosotros Tº
portMUX_TYPE temperatura_medida_NTC_access = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE medicion_extraida_buffer_access = portMUX_INITIALIZER_UNLOCKED;

LiquidCrystal_I2C lcd_i2c(0x3F, 16, 2); //Variable de acceso al lcd

const float Temperatura_minima_permitida = 20.0f;
const float Temperatura_maxima_permitida = 32.0f;
const unsigned long tiempoAcomodo = 5000;  // 5000 milisegundos = 5 segundos extra
unsigned long inicioAcomodo = 0;
bool enAcomodo = false;

struct Buffer_Circ
{
  float buffer[BUFFER_SIZE];
  uint16_t bufIN;
  uint16_t bufOUT;
  uint32_t bufCount;
};
Buffer_Circ buf;

enum Estado_Temperatura
{
  CORRECTA,
  ALTA,
  DESCONOCIDO
};

Estado_Temperatura Estado_temperatura_actual = DESCONOCIDO;

volatile bool Seta_ISR = false;
volatile bool Estado_ISR = false;
volatile float Temperatura_medida_NTC = 0.0f;
volatile float Medicion_extraida_buffer = 0.0f;


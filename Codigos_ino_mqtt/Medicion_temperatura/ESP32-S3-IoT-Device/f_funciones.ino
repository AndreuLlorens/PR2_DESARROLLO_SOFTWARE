#include<Wire.h>
#include<LiquidCrystal_I2C.h>

void IRAM_ATTR isr()
{
  static uint32_t tiempo_final;
  uint32_t tiempo_inicial = millis();
  if((tiempo_inicial - tiempo_final) > 1000)
  {
    Seta_ISR = !Seta_ISR;
    Estado_ISR = true;
  }
  tiempo_final = tiempo_inicial;

}

static float Medir_NTC()
{
  uint16_t ADCvalor = analogRead(PIN_NTC);

  if( (ADCvalor <= RANGO_ANALOGICO_MIN) || (ADCvalor >= RANGO_ANALOGICO_MAX) )
  {
    return (-1.0f);
  }
  //quitar static si no va correctamente la medicion
  float Vout = (ADCvalor * Vcc) / float(RANGO_ANALOGICO_MAX);
  float R_NTC = (Vout * R0) / (Vcc - Vout);
  float tempK = (1.0f) / ((log(R_NTC / R0) / beta) + (1.0f / T0 ));
  float tempC = tempK - K;

  return(tempC);
}

static void Mostrar_LCD(float valor)
{
  if(valor == (-1.0f))
  {
    lcd_i2c.clear();
    lcd_i2c.setCursor(0, 0);
    lcd_i2c.print("Error en ADC");
  }

  else
  {
  lcd_i2c.clear();
  lcd_i2c.setCursor(0, 0);
  lcd_i2c.print("Temp:");
  lcd_i2c.print(valor, 1);
  lcd_i2c.print((char)223);
  lcd_i2c.print("C");
  }  

}

/*funciones del buffer*/
float get_item(Buffer_Circ * b, float * item)
{
  portENTER_CRITICAL(&temperature_buffer_access);

  if(b->bufCount == 0)
  {
    portEXIT_CRITICAL(&temperature_buffer_access);
    return(-1.0f);
  }

  *item = b->buffer[b->bufOUT];
  b->bufOUT = (b->bufOUT + 1) % BUFFER_SIZE;
  b->bufCount--;

  portEXIT_CRITICAL(&temperature_buffer_access);
  return(0.0f);
}

float put_item(Buffer_Circ * b, float item)
{
  portENTER_CRITICAL(&temperature_buffer_access);

  if(b->bufCount == BUFFER_SIZE)
  {
    portEXIT_CRITICAL(&temperature_buffer_access);
    return(-1.0f);
  }
  b->buffer[b->bufIN] = item;
  b->bufIN = (b->bufIN + 1) % BUFFER_SIZE;
  b->bufCount++;

  portEXIT_CRITICAL(&temperature_buffer_access);
  return(0.0f);
}

static void Listar_Buffer(Buffer_Circ * b)
{
  float copia_buffer[BUFFER_SIZE];
  uint32_t cantidad;
  uint16_t index;
  portENTER_CRITICAL(&temperature_buffer_access);
  cantidad = b->bufCount;
  
  for(uint32_t i = 0; i < cantidad; i++)
  {
    index = (b -> bufOUT + i) % BUFFER_SIZE;
    copia_buffer[i] = b->buffer[index];
  }
  portEXIT_CRITICAL(&temperature_buffer_access);

  Serial.print("Buffer: [");
  for(uint32_t i = 0; i < cantidad; i++)
  {
    Serial.print(copia_buffer[i], 2);
    if(i < (cantidad - 1))
    {
      Serial.print("ºC,");
    }
  }
  Serial.println("ºC]");
}

/*tareas*/
static void put_item_task(void * pvParameters)
{
  static float temp_temporal;

  for(;;)
  {
    while(Seta_ISR)
    {
      vTaskDelay(pdMS_TO_TICKS(TIEMPO_PAUSA_DE_TAREAS));
    }
    portENTER_CRITICAL(&temperatura_medida_NTC_access);
    temp_temporal = Temperatura_medida_NTC;
    portEXIT_CRITICAL(&temperatura_medida_NTC_access);

    if(put_item(&buf, temp_temporal) == 0)
    {
      Serial.printf("PRODUCCIDO: %.2f ºC\n", temp_temporal);
    }
    vTaskDelay(pdMS_TO_TICKS(TIEMPO_ESPERA_PUT));
  }
}

static void get_item_task(void * pvParameters)
{
  float item;
  for(;;)
  {
    if(get_item(&buf, &item) == 0)
    {
      Serial.printf("Consumido: %.2f ºC\n", item);
      //Mostrar_LCD(item); //Hacerlo en una tarea

      //AQUI METER EL MQTT ITEM ES LA VARIABLE QUE SALE DEL 
      portENTER_CRITICAL(&medicion_extraida_buffer_access);
      Medicion_extraida_buffer = item;
      portEXIT_CRITICAL(&medicion_extraida_buffer_access);
    }
    vTaskDelay(pdMS_TO_TICKS(TIEMPO_ESPERA_GET));
  }
}

static void Listar_Buffer_task(void * pvParameters)
{
  for(;;)
  {
    while(Seta_ISR)
    {
      vTaskDelay(pdMS_TO_TICKS(TIEMPO_PAUSA_DE_TAREAS));
    }
    Listar_Buffer(&buf);
    vTaskDelay(pdMS_TO_TICKS(TIEMPO_ESPERA_LISTAR));
  }
}

static void Medir_temperatura_NTC_task(void * pvParameters)
{
  for(;;)
  {
    while(Seta_ISR)
    {
      vTaskDelay(pdMS_TO_TICKS(TIEMPO_PAUSA_DE_TAREAS));
    }
    portENTER_CRITICAL(&temperatura_medida_NTC_access);
    Temperatura_medida_NTC = Medir_NTC();
    portEXIT_CRITICAL(&temperatura_medida_NTC_access);
    vTaskDelay(pdMS_TO_TICKS(TIEMPO_ESPERA_MEDIR_NTC));
  }
}


static void Mostrar_LCD_task(void * pvParameters)
{
  float extraccion;
  for(;;)
  {
    while(Seta_ISR)
    {
      vTaskDelay(pdMS_TO_TICKS(TIEMPO_PAUSA_DE_TAREAS));
    }
    portENTER_CRITICAL(&medicion_extraida_buffer_access);
    extraccion = Medicion_extraida_buffer;
    portEXIT_CRITICAL(&medicion_extraida_buffer_access);
    Mostrar_LCD(extraccion);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}




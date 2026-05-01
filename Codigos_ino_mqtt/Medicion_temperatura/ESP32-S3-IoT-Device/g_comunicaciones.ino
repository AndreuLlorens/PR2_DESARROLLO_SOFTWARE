void suscribirseATopics() 
{  
  mqtt_subscribe(STATION_COMMAND_TOPIC); //leer del topic
}
void alRecibirMensajePorTopic(char * topic, String mensaje_mqtt)
{

}

void enviarMensajePorTopic()
{
   Estado_Temperatura nuevo_estado_temperatura;
  if(Estado_ISR)
  {
    Estado_ISR = false;
    if(Seta_ISR)
    {
      Serial.println("SETA ON");
      mqtt_publish(STATION_STATUS_TOPIC, MENSAJE_ACTIVAR_SETA);
    
    }

    else
    {
      Serial.println("SETA OFF");
      mqtt_publish(STATION_STATUS_TOPIC, MENSAJE_DESACTIVAR_SETA);
    }
  }
    
    while(Seta_ISR)
    {
      delay(TIEMPO_PAUSA_DE_TAREAS);
    }
    while(Medicion_extraida_buffer == 0.0f)
    {
      delay(3000);//extraer tiene que sacar
    }

  portENTER_CRITICAL(&medicion_extraida_buffer_access);
  float temperatura_extraida_buffer = Medicion_extraida_buffer;
  portEXIT_CRITICAL(&medicion_extraida_buffer_access);
  if(temperatura_extraida_buffer > Temperatura_maxima_permitida)
  {
    nuevo_estado_temperatura = ALTA;
  }
  else
  {
    nuevo_estado_temperatura = CORRECTA;
  }

  if(nuevo_estado_temperatura != Estado_temperatura_actual)
  {
    switch(nuevo_estado_temperatura)
    {
      case ALTA:
        enAcomodo = false;               // Cancelamos cualquier temporizador previo
        digitalWrite(PIN_LED_AZUL, HIGH);  // Enfriando
        digitalWrite(PIN_LED_ROJO, LOW);
        mqtt_publish(STATION_STATUS_TOPIC, ENVIAR_MENSAJE_TEMEPRATURA_ALTA);
        Serial.println(">>> MQTT: temperatura_alta. Activando LED Azul.");
        break;
        
      case CORRECTA:
        // Acaba de entrar al rango correcto desde Alta o Baja.
        // NO apagamos los LEDs todavía, ni enviamos el mensaje MQTT de "correcta".
        // Empezamos a contar el tiempo de acomodo.
        enAcomodo = true;
        inicioAcomodo = millis();  // Guardamos el momento exacto
        mqtt_publish(STATION_STATUS_TOPIC, ENVIAR_MENSAJE_TEMPERATURA_ACEPTABLE);
        break;
        
      default:   
        Serial.println("SE HA EJECUTADO EL CASO QUE ES DESCONOCIDO");
        break;
      }
      Estado_temperatura_actual = nuevo_estado_temperatura; //sobreescribo
  }
  if((Estado_temperatura_actual == CORRECTA) && enAcomodo)
  {
    if (millis() - inicioAcomodo >= tiempoAcomodo)
    {
      enAcomodo = false;  // Terminamos el periodo de acomodo

      // Apagamos los LEDs
      digitalWrite(PIN_LED_ROJO, LOW);
      digitalWrite(PIN_LED_AZUL, LOW);

      // Ahora sí, avisamos de que todo está estable
      mqtt_publish(STATION_STATUS_TOPIC, ENVIAR_MENSAJE_TEMPERATURA_ACEPTABLE);
      Serial.println(">>> MQTT: temperatura_correcta. Acomodo finalizado, LEDs apagados.");
    }
  }
  delay(2000);
}





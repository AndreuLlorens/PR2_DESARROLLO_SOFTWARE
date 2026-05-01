estado_sensor Estado_lectura_sensor = DESCONOCIDO;
 // Para evitar reenvíos repetidos
void suscribirseATopics()
{
  mqtt_subscribe(STATION_COMMAND_TOPIC);
}

void enviarMensajePorTopic()
{

  if(Estado_lectura_sensor == DETECTADO)
  {
    Serial.println("ARRANCO PROGRAMA ROBODK");
    mqtt_publish(STATION_STATUS_TOPIC,ENVIAR_SENYAL);
  }
  else if(Estado_lectura_sensor == NO_DETECTADO)
  {
    Serial.println("PARO PROGRAMA ROBODK");
    mqtt_publish(STATION_STATUS_TOPIC,ENVIAR_SENYAL_NO_DETECTADO);
  }
  else
  {
    Serial.println("DESCONCODIO NO HAGO NADA");
  }
}

void alRecibirMensajePorTopic(char * topic, String mensaje_mqtt)
{

  mensaje_mqtt.trim();
  Serial.print("Mensaje Recibido: ");
  Serial.print(mensaje_mqtt);
  Serial.println();

  if(mensaje_mqtt ==  MENSAJE_DETECTADO)
  {
    Estado_lectura_sensor = DETECTADO;
  }

  else if(mensaje_mqtt == MENSAJE_NO_DETECTADO)
  {
    Estado_lectura_sensor = NO_DETECTADO;
  }
  else
  {
    Estado_lectura_sensor = DESCONOCIDO;
  }
  enviarMensajePorTopic();
  
}

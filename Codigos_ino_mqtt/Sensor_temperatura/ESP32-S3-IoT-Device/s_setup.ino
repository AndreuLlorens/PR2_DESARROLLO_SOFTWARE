#include<Wire.h>
#include<LiquidCrystal_I2C.h>


void on_setup()
{
  delay(5000);
  Wire.begin(PIN_SDA, PIN_SCL);
  lcd_i2c.init();
  lcd_i2c.backlight();
  lcd_i2c.setCursor(0 ,0);
  lcd_i2c.print("Incio Conf");
  delay(1000);
  lcd_i2c.clear();
  //inicializo el buffer
  buf.bufIN = 0;
  buf.bufOUT = 0;
  buf.bufCount = 0;


  pinMode(PIN_NTC, INPUT);
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_AZUL, OUTPUT);
  //ISR 
  pinMode(PIN_BOTON_EMERGENCIA, INPUT_PULLUP);
  attachInterrupt(PIN_BOTON_EMERGENCIA, isr, FALLING);
  
  xTaskCreate(Medir_temperatura_NTC_task, "medir temperatura", 4096,
  NULL, MEDIR_NTC_TASK_PRIORITY, &Medir_ntc_task_handle);

  xTaskCreate(put_item_task, "Productor", 4096,
  NULL, PUT_ITEM_TASK_PRIORITY, &put_item_task_handle);
  
  xTaskCreate(get_item_task, "Consumidor", 4096,
  NULL, GET_ITEM_TASK_PRIORITY, &get_item_task_handle);

  xTaskCreate(Mostrar_LCD_task, "lcd", 3072, 
  NULL, MOSTRAR_LCD_TASK_PRIORITY, &mostar_lcd_task_handle);

  xTaskCreate(Listar_Buffer_task, "Listar", 3072,
  NULL, LISTAR_BUFFER_TASK_PRIORITY, &Listar_Buffer_task_handle);


}


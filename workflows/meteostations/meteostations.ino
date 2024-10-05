#include "OneWire.h"
#include "DallasTemperature.h"

#define Serial Serial

OneWire oneWire(10); // порт подключения датчика (A1)
OneWire oneWire2(9); // порт подключения датчика (A1)
DallasTemperature ds(&oneWire);
DallasTemperature ds2(&oneWire2);

void outNumber(char *component, uint32_t number);
void outText(char *component, char *text);
void print_string(char *string);
void print_dec(uint32_t data);
void sendFFFFFF(void);

String incStr;    // объявляем переменую типа String не путать со string
String string;
float t1 = 0.00;
float t2 = 0.00;

byte temp = 0;
bool grafik = 0;

void setup(){
  ds.begin(); // инициализация датчика ds18b20
  ds2.begin(); // инициализация датчика ds18b20
  Serial.begin(9600);
 
}

void loop(){

  if (Serial.available()) 
  {
    char inc;
    inc = Serial.read();
    incStr += inc;
    if (inc == 0x23) 
    {
      incStr = "";
    }
    if (inc == 0x0A) 
    {  // настравиваем ожидать данные в конце данных 0x0A
       AnalyseString(incStr);
       incStr = "";
       inc = "";
    }
  }

  
if (!(Serial.available()))
{ 
  if (grafik == 0)
  {
      ds.requestTemperatures();
      t1 =(ds.getTempCByIndex(0));
   
      t1 = t1_averaged_reading();
      
      String t0 = "\"" + String(t1) + "\"";  // выводим температуру и градусы цельсия 
      SendData("t0.txt", t0);
      delay(1000);


      ds2.requestTemperatures();
      t2 =(ds2.getTempCByIndex(0));
   
      t2 = t2_averaged_reading();
      
      String t1 = "\"" + String(t2) + "\"";  // выводим температуру и градусы цельсия 
      SendData("t1.txt", t1);
      delay(1000);

  }else if (grafik == 1)
    {
      ds.requestTemperatures();
      t1 =(ds.getTempCByIndex(0));
   
      t1 = t1_averaged_reading();
       gafick0();
      ds2.requestTemperatures();
      t2 =(ds2.getTempCByIndex(0));
   
      t2 = t2_averaged_reading(); 
       gafick1();
    }
}
  if ((incStr.indexOf("00"))>=0)
  { // когда находимся на странице 0 обновляем компоненты
      grafik = 0;
      cle();
      
      String t0 = "\"" + String(t1) + "\"";  // выводим температуру и градусы цельсия 
      SendData("t0.txt", t0);
      delay(1000);

      String t1 = "\"" + String(t2) + "\"";  // выводим температуру и градусы цельсия 
      SendData("t1.txt", t1);
      delay(1000);
  
  } else if((incStr.indexOf("01"))>=0)
  {
     grafik = 0;
  } else if((incStr.indexOf("02"))>=0)
  {
     grafik = 0;
  } else if((incStr.indexOf("03"))>=0)
  {
     grafik = 0; 
  } else if((incStr.indexOf("04"))>=0) // когда находимся на странице 4 обновляем компоненты
  {
       grafik = 1; 
    
  }

 
  
}
// Автор Максим Селиванов
void outNumber(char *component, uint32_t number){
  print_string(component);  // Отправка имени компонента и атрибута
  print_string("=");      // Открывающая двойная ковычка
  print_dec(number);      // Печать 16-разрядного числа
  sendFFFFFF();       // Три байта 0xFF
}

/*****************************************************
      Изменение текстового атрибута
*****************************************************/
/**
void outText(char *component, char *text)
{
  print_string(component);  // Отправка имени компонента и атрибута
  print_string("=");    // Открывающая двойная ковычка
  print_string("\"");
  //print_string(text);   // Печать текста без конвертации
  print_string_ISO8859(text); // Печать текста с конвертацией в таблицу ISO8859-5
  print_string("\"");     // Закрывающая двойная ковычка
  sendFFFFFF();       // Три байта 0xFF
}
**/
/*****************************************************
  ПЕЧАТЬ СТРОКИ (c конвертацией в таблицу ISO8859-5)
*****************************************************/
/**
void print_string_ISO8859(char *string)
{
  uint8_t temp;
  while(*string)
  {
    temp = *string++;
    if (temp >= 0xC0) temp -= 0x10; // Для конвертации из windows-1251 в ISO8859-5
    Serial.write(temp);
  }
}
**/
/*****************************************************
  ПЕЧАТЬ СТРОКИ (без конвертации в таблицу ISO8859-5)
*****************************************************/
void print_string(char *string)
{
  while(*string) Serial.write(*string++); // Пока разыменованное значение не ноль, отправлять через UART
}

void print_dec(uint32_t data)
{
  uint16_t num;  // вспомогательная переменная
  //for(num=0; data >= 1000000; num++) data -= 1000000; // Выделение десятков тысяч в переменной num
  //nexSerial.write(num + '0'); 
  for(num=0; data >= 100000; num++) data -= 100000; // Выделение десятков тысяч в переменной num
  Serial.write(num + '0'); 
  for(num=0; data >= 10000; num++) data -= 10000; // Выделение десятков тысяч в переменной num
  Serial.write(num + '0');              // Печать десятков тысяч в ASCII
  for(num=0; data >= 1000; num++) data -= 1000; // Выделение тысяч в переменной num
  Serial.write(num + '0');              // Печать тысяч в ASCII
  for(num=0; data >= 100; num++) data -= 100;   // Выделение сотен в переменной num
  Serial.write(num + '0');              // Печать сотен в ASCII
  for(num=0; data >= 10; num++) data -= 10;   // Выделение десятков в переменной num
  Serial.write(num + '0');              // Печать десятков в ASCII
  Serial.write(data + '0');             // Печать единиц в ASCII
}


void sendFFFFFF(void)
{
  Serial.write(0xFF);
  Serial.write(0xFF);
  Serial.write(0xFF);
} // Здесь закачивается код Максима Селиванова 


void AnalyseString(String incStr) 
{

}

void cle(void){
  Serial.print("grafik.cle 1,255");
  Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  Serial.write(0xff);
  Serial.write(0xff);
  delay(8);
} 


void gafick0(void){
  Serial.print("add 1,0,");
  print_dec(t1);
  Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  Serial.write(0xff);
  Serial.write(0xff);
  delay(8); 
}

void gafick1(void){
  Serial.print("add 1,1,");
  print_dec(t2);
  Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  Serial.write(0xff);
  Serial.write(0xff);
  delay(8);
}

float t1_averaged_reading ()
{
  int N = 10;
  float sum = 0.0 ;
  for (byte i = 0 ; i < N ; i++)
    sum += t1 ; // whatever code reads the sensor
  return sum / N ;
}

float t2_averaged_reading ()
{
  int N = 10;
  float sum = 0.0 ;
  for (byte i = 0 ; i < N ; i++)
    sum += t2 ; // whatever code reads the sensor
  return sum / N ;
}

void SendData(String dev, String data)
{
  Serial.print(dev);
  Serial.print("=");
  Serial.print(data);
  Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  Serial.write(0xff);
  Serial.write(0xff);
}

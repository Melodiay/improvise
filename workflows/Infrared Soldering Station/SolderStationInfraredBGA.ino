
#include <GyverMAX6675.h> // Подключаем библиотеку работы с микросхемой MAX6675 автор https://alexgyver.ru/lessons/

#define ZERO_PIN 2  // Для обращения к выводу 2 указываем имя ZERO_PIN, порт для детектора нуля
#define nigniy_1 3  // указываем порты 3 вывода нижнего нагревателя с ШИМ
#define verhniy_1 5 // указываем верхний нагреватель порт 5 вывода с ШИМ
#define coolerv 9   // Кулер встроеный в верхний нагреватель 
#define cooler 10  // Для подключения кулера с ШИМ, для охлаждения всей платы
#define lampa 11   // Для подключения лампы с ШИМ, подстветка при пайке

// объединяем порт 8 и 4 на ардуино для двух термопар, в месте подключаем
// Пины модуля MAX6675K
#define CLK_PIN   8  // Пин SCK  указываем вывводы для программного ICP
#define DATA_PIN  4  // Пин SO
#define CS_PIN    7  // Пин CS

// указываем пины в порядке SCK SO CS
GyverMAX6675<CLK_PIN, DATA_PIN, CS_PIN> sens; // sens, CLK, DATA, CS если больше одного модуюя можно переменовать например в CLK1, DATA1, CS1, sens1
                                              // незабывать обращаться к sens1, а не к sens - это если не одна термопара
// Пины модуля MAX6675K
#define CLK_PIN2  8  // Пин SCK указываем вывводы для программного ICP
#define DATA_PIN2  4  // Пин SO
#define CS_PIN2    13  // Пин CS

// указываем пины в порядке SCK SO CS
GyverMAX6675<CLK_PIN2, DATA_PIN2, CS_PIN2> sens2;

#include <GyverRelay.h>
// установка, гистерезис, направление регулирования автор https://alexgyver.ru/lessons/
GyverRelay regulator(REVERSE); 
GyverRelay regulator2(REVERSE);

void outNumber(char *component, uint16_t number);

String incStr;    // объявляем переменую типа String не путать со string
String string;

float tempt1 = 0;      //  вывод температуры сдатчика на дисплей. облявляем переменную целочисленую для температуры 1, если нужно с дробной частью то объявляем float и переменной присваимваем 0
float tempt2 = 0;      // вывод температуры сдатчика на дисплей. температура 2 целочисленная, для дробной заменить int на float и переменной присваимваем 0
int temp = 0; // temp используется для активации нужной страницы и подсчета наней температуры, если temp = 0, то температуру не подсчитываем 
int temp1 = 225; // температура по умолчанию верхнего нагревателя 
int temp2 = 160; // температура по умолчанию нижнего нагревателя
int tempust1 = 0;     // установленая температура 'C градусов цельсия должна считываться с дисплея Nextion
int tempust2 = 0;     // установленая температура 'C градусов цельсия должна считываться с дисплея Nextion 
int pwmv = 76; // ШИМ верхнего нагревателя по умолчанию 30% == в ШИМ = 76 дробную часть отбрасываем 76,5
int pwmn = 76; // ШИМ нижнего нагревателя по умолчанию 30% == в ШИМ = 76 дробную часть отбрасываем 76,5
int pwmust1 = 0; // Установленый ШИМ
int pwmust2 = 0; // Установленый ШИМ
int kp = 20; // ПИД регулирование порпорциональное   значение по умолчанию
int ki = 60; // ПИД регулирование интегральное       значение по умолчанию 
int kd = 1;  // ПИД регулирование дифферинциальное   значение по умолчанию
int Kp = 0;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
int Ki = 0;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
int Kd = 0;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
int reley_v; // Верхний нагреватель включение отдельно
int reley_n; // Нижний нагреватель переменная для включения паяльной станции верхний нагреватель нижний нагреватель, либо по отдельности Верх нагр. Нижний нагр.
int reley_n1; // Нижний нагреатель включение отдельно
int ph; //  пидрегулирование или гистерезис переключение между ПИД и Гистерезис регулирование
int pidn; // ПИД нижнего нагревателя
int hestn; // Гистерезис нижнего нагревателя
int r100; //  увеличивается или уменьшается сотнями то есть +100 или -100
int r10;  //  увеличивается или уменьшается десятками то есть +10 или -10
int r1;   //  увеличивается или уменьшается единицами то есть +1 или -1
int rtemp; // переменая где хранятся 100, 10, 1
int termoprofily = 0; // Номер термопрофиля по умолчанию выбран 0
String profily="Lead-free"; // Загружает Бессвинцовый термопрофиль по умолчанию
int shag = 0;
int sec = 0;


void setup(void) {
  Serial.begin(9600); // Указваем скорость UART 9600 бод
  
  pinMode(nigniy_1, OUTPUT); // нижний нагреватель номер 1 настраиваем на выход
  digitalWrite(nigniy_1, LOW); // отключаем выход
  pinMode(verhniy_1, OUTPUT); // верхний нагреватель настраиваем на выход
  digitalWrite(verhniy_1, LOW); // отключаем выход, то есть не подаем пять вольт, подовать будем поже
  pinMode(cooler, OUTPUT);    // тоже самое настраиваем вывод на выход
  digitalWrite(cooler, LOW);  // отключаем вывод
  pinMode(lampa, OUTPUT);    // тоже самое настраиваем вывод на выход
  digitalWrite(lampa, LOW);  // отключаем вывод
  pinMode(ZERO_PIN, INPUT); // настраиваем вывод на вход
  tempust1 = temp1;
  tempust2 = temp2;
}

void loop(void) {
  if (Serial.available()) {
    char inc;
    inc = Serial.read();
    incStr += inc;
    if (inc == 0x23) {
      incStr = "";
    }
    if (inc == 0x0A) {  // настравиваем ожидать данные в конце данных 0x0A
       AnalyseString(incStr);
       incStr = "";
       inc = "";
    }
  }
 
  if ((incStr.indexOf("00"))>=0){ // когда находимся на странице 0 обновляем компоненты
      temp = 1;
      outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      outNumber("termoprofily.temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      outNumber("termoprofily.temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      outNumber("termoprofily.n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
      outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
      outNumber("watt.pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
      outNumber("watt.pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
      String t13= "\"" + String(profily) + "\"";  // Отображение 
      SendData("termoprofily.t13.txt", t13);
      String t24 = "\"" + String(kp) + "\"";  // выводим пропорциональное
      SendData("pid.t24.txt", t24);
      String t25 = "\"" + String(ki) + "\"";  // выводим интегральное
      SendData("pid.t25.txt", t25);
      String t26= "\"" + String(kd) + "\"";  // выводим дефференциальное
      SendData("pid.t26.txt", t26);      
  } else if((incStr.indexOf("01"))>=0)
  {
      temp = 0;
  }else if((incStr.indexOf("02"))>=0)
  {
      temp = 0;
  }else if((incStr.indexOf("03"))>=0)
  {
      temp = 0;
  }else if((incStr.indexOf("04"))>=0) // когда находимся на странице 4 обновляем компоненты
  {
      temp = 0;
      outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
      outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      outNumber("main.n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      outNumber("main.n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      String t13= "\"" + String(profily) + "\"";  // Отображение 
      SendData("t13.txt", t13);
      outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
      outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
      
  }else if((incStr.indexOf("05"))>=0) // когда находимся на странице 5 обновляем компоненты
  {
      temp = 0;
      outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
      outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
  }else if((incStr.indexOf("06"))>=0)
  {
      temp = 0;
  }else if((incStr.indexOf("07"))>=0) // когда находимся на странице 7 обновляем компоненты
  {
      temp = 0;
      String t24 = "\"" + String(kp) + "\"";  // выводим пропорциональное
      SendData("t24.txt", t24);
      String t25 = "\"" + String(ki) + "\"";  // выводим интегральное
      SendData("t25.txt", t25);
      String t26= "\"" + String(kd) + "\"";  // выводим дефференциальное
      SendData("t26.txt", t26);
  }
  
  if (temp==1){
     if (!(Serial.available())){   // если не принимаем и не передаем даные, то считываем температуры temp1 и temp2, в противном случае игнорируем temp1 и temp2
          if (tempt1 = (sens.readTemp())){            // Читаем температуру
             // Если чтение прошло успешно - выводим в Serial
            tempt1 = (sens.getTemp());   // Забираем температуру через getTemp - вещественные числа (с float) с дробной частью, не забудьте объявить переменную temp1 нужного типа должна быть float а не int
            //temp1 = (sens.getTempInt());   // или getTempInt - целые числа (без float), можно объявить переменную temp1 типом int
            String t1 = "\"" + String(tempt1) + "'C\"";  // выводим температуру и градусы цельсия 
            SendData("t0.txt", t1);                     // на дисплей Nextion в компонент t0 с параметром txt
          } else {
            String t1 = "\"" + String(tempt1) + "Error\"";  // если произошла ошибка выодим Error
            SendData("t0.txt", t1);
          }
          delay(1000);   // задержка в 1 секунду, так как нужно примерно 750 мс чтобы считать температуру успешно
    
          if (tempt2 = (sens2.readTemp())){            // Читаем температуру
             // Если чтение прошло успешно - выводим в Serial
            tempt2 = (sens2.getTemp());   // Забираем температуру через getTemp - вещественные числа (с float)
            //temp2 = (sens2.getTempInt());   // или getTempInt - целые числа (без float)
            String t1 = "\"" + String(tempt2) + "'C\"";
            SendData("t1.txt", t1);
          } else {
            String t1 = "\"" + String(tempt2) + "Error\"";  // если произошла ошибка выодим Error
            SendData("t1.txt", t1);
          }
          delay(1000); 
          
      }
  }
  
   if (ph==1){
        if(reley_n==1){
          if (reley_n1==1){
            nigniye(); // Пид регулирование
          }
       }
       if(reley_v==1){
         verhniy(); // Пид регулирование
       }
   }else {
       if(reley_n==1){
          if (reley_n1==1){
            reguln();  // Гистерезис
          }
      }
      if(reley_v==1){
        regul();   // Гистерезис
      }
  }
  
  regulator.setpoint = tempust1;    // установка (температуры)
  regulator.hysteresis = 0.25;   // ширина гистерезиса
  regulator.k = 0.5;          // коэффициент обратной связи (подбирается по факту)
  regulator.dT = 500;       // установить время итерации для getResultTimer
  regulator2.setpoint = tempust2;    // установка (температуры)
  regulator2.hysteresis = 0.25;   // ширина гистерезиса
  regulator2.k = 0.5;          // коэффициент обратной связи (подбирается по факту)
  regulator2.dT = 500;       // установить время итерации для getResultTimer
  
  
}
// Автор Максим Селиванов
void outNumber(char *component, uint16_t number){
  print_string(component);  // Отправка имени компонента и атрибута
  print_string("=");      // Открывающая двойная ковычка
  print_dec(number);      // Печать 16-разрядного числа
  sendFFFFFF();       // Три байта 0xFF
}
/*****************************************************
      Изменение текстового атрибута
*****************************************************/
void outText(char *component, char *text){
  print_string(component);  // Отправка имени компонента и атрибута
  print_string("=""\"");    // Открывающая двойная ковычка
  //print_string(text);   // Печать текста без конвертации
  print_string_ISO8859(text); // Печать текста с конвертацией в таблицу ISO8859-5
  print_string("\"");     // Закрывающая двойная ковычка
  sendFFFFFF();       // Три байта 0xFF
}

/*****************************************************
  ПЕЧАТЬ СТРОКИ (c конвертацией в таблицу ISO8859-5)
*****************************************************/
void print_string_ISO8859(char *string){
  uint8_t temp;
  while(*string){
    temp = *string++;
    if (temp >= 0xC0) temp -= 0x10; // Для конвертации из windows-1251 в ISO8859-5
    Serial.write(temp);
  }
}
/*****************************************************
  ПЕЧАТЬ СТРОКИ (без конвертации в таблицу ISO8859-5)
*****************************************************/
void print_string(char *string){
  while(*string) Serial.write(*string++); // Пока разыменованное значение не ноль, отправлять через UART
}


void print_dec(uint16_t data)
{
  uint8_t num;  // вспомогательная переменная
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


void sendFFFFFF(void){
  Serial.write(0xFF);
  Serial.write(0xFF);
  Serial.write(0xFF);
} // Здесь закачивается код Максима Селиванова 
// Пид регулирование нижний нагреватель
void nigniye(){
    // (вход, установка, п, и, д, период в секундах, мин.выход, макс. выход)
  analogWrite(nigniy_1, NizPID(tempt2, tempust2, Kp, Ki, Kd, 0.02, 0, pwmust2)); 
  delay(20); 
}
// Пид регулирование верхний нагреватель
void verhniy(){
     // (вход, установка, п, и, д, период в секундах, мин.выход, макс. выход)
     analogWrite(verhniy_1, VerhPID(tempt1, tempust1, Kp, Ki, Kd, 0.02, 0, pwmust1));
     delay(20);
}
// Гистерезис нижний нагреватель
void reguln(){
    regulator2.input =  tempt2; 
    digitalWrite(nigniy_1, regulator2.getResultTimer());   // отправляем на реле (ОС работает по своему таймеру)
   /* 
    if (reley_n1==1){digitalWrite(nigniy_1, regulator2.getResultTimer());}   // отправляем на реле (ОС работает по своему таймеру)
    */
}
// Гистерезис верхний нагреватель
void regul() {
  
    regulator.input =  tempt1;
    digitalWrite(verhniy_1, regulator.getResultTimer());   // отправляем на реле (ОС работает по своему таймеру)
  
}

void AnalyseString(String incStr) {
  if (incStr.indexOf("bt0-on") >= 0) {     // слушаем UART на передачу команды bt0-on подаем 5 вольт на вывод
    reley_n=1;
    //digitalWrite(nigniy_1, HIGH);
  } else if (incStr.indexOf("bt0-off") >= 0) { //слушаем UART на команду bt0-off и снимаем 5 вольт с вывода
    reley_n=0;
    //digitalWrite(nigniy_1, LOW);
  }
  if (incStr.indexOf("c0-on") >= 0) {      // тоже самое что и bt0
    reley_n1=1;
  } else if (incStr.indexOf("c0-off") >= 0) {
    reley_n1=0;
    digitalWrite(nigniy_1, LOW);
  }
  if ((incStr.indexOf("c3-on")) >= 0) { 
    reley_v=1;
    //digitalWrite(verhniy_1, HIGH);// тоже самое что и bt0
  } else if ((incStr.indexOf("c3-off")) >= 0) {
    reley_v=0;
    digitalWrite(verhniy_1, LOW);
  }
  if (incStr.indexOf("coolerv-on") >= 0) {      // тоже самое что и bt0
     digitalWrite(coolerv, HIGH);
  }  else if (incStr.indexOf("coolerv-off") >= 0) {
     digitalWrite(coolerv, LOW);
  }
  if (incStr.indexOf("cooler-on") >= 0) {      // тоже самое что и bt0
     digitalWrite(cooler, HIGH);
  }  else if (incStr.indexOf("cooler-off") >= 0) {
     digitalWrite(cooler, LOW);
  }
  if (incStr.indexOf("bt1-on") >= 0) {      // тоже самое что и bt0
     digitalWrite(lampa, HIGH);
  }  else if (incStr.indexOf("bt1-off") >= 0) {
     digitalWrite(lampa, LOW);
  }
   if (incStr.indexOf("pid") >= 0) {     // выбиран пид регулирование
     ph=1;
    
  } else if (incStr.indexOf("hesterezis") >= 0) { // выбран гистерезис
     ph=0;
  }

  if (incStr.indexOf("b4") >= 0) {
    if (shag < 10){
      shag++;
      outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
      if(shag == 0){
       sec=0;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 50; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       } else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 50; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 0; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 0; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
    } else if(shag == 1){
       sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 50; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 50; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 80; // Нижний нагреватель Бессвинецовый выбрано 80 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
        } else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 80; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
        }else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 30; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 50; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
    } else if(shag == 2){
       sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 80; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 80; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
        }else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 50; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 80; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
    } else if(shag == 3){
       sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 100; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 100; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 120; // Нижний нагреватель Бессвинецовый выбрано 120 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
        }else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 120; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 80; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
    } else if(shag == 4){
       sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 120; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 120; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 140; // Нижний нагреватель Бессвинецовый выбрано 140 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
        }else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 140; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 120; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
    } else if(shag == 5){
       sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 140; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 140; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       } else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 150; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 120; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 140; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
    } else if(shag == 6){
       sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 150; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 150; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 160; // Верхний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
        } else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 140; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 150; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 150; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
       
    } else if(shag == 7){
       sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 180; // Верхний нагреватель Бессвинцовый выбрано 180 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       } else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 165; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 150; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 180; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 150; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
       
    } else if(shag == 8){
       sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 160; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 160; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
        profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       } else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 170; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 150; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 150; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
       
    } else if(shag == 9){
       sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 180; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 180; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       } else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 175; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 150; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 150; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
    } else if(shag == 10){
       sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 0; // Нижний нагреватель Бессвинецовый выбрано 0 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 180; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 0; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 0; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
       }
    } else {
       shag = 0;
       sec=0;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      } else if (termoprofily == 1){
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
      } else if (termoprofily == 2){
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 0; // Нижний нагреватель Бессвинецовый выбрано 0 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 3){
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 0; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       } else if (termoprofily == 4){
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 0; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }else if (termoprofily == 5){
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 0; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
       }
    }  
  }
  if (incStr.indexOf("bs20") >= 0) {
    if (sec < 600){
      sec++;
      outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
    } else if(sec == 600){
        sec = 0;
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
    }
    
  }
    if (incStr.indexOf("bs21") >= 0) {
    if (sec > 0 ){
      sec--;
      outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
    } else if (sec == 0){
        sec = 600;
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
    }
    
  }
  if (incStr.indexOf("b6") >= 0) {
    if (temp1 < 300){
      temp1=temp1+rtemp;
      outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      tempust1 = temp1;
    } else if(temp1 == 300){
        temp1 = 0;
       outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
       tempust1 = temp1;
    }
    
  }
    if (incStr.indexOf("b7") >= 0) {
    if (temp1 > 0 ){
      temp1=temp1-rtemp;
      outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      tempust1 = temp1;
    } else if (temp1 == 0){
        temp1 = 300;
       outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
       tempust1 = temp1;
    }
    
  }
    if (incStr.indexOf("b8") >= 0) {
    if (temp2 < 300){
      temp2=temp2+rtemp;
      outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      tempust2 = temp2;
    } else if (temp2 == 300){
        temp2 = 0;
       outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
       tempust2 = temp2;
    }
    
  }
    if (incStr.indexOf("b9") >= 0) {
    if (temp2 > 0){
      temp2=temp2-rtemp;
      outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      tempust2 = temp2;
    } else if (temp2 == 0){
       temp2 = 300;
       outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
       tempust2 = temp2;
    }
    
  }
    if (incStr.indexOf("b10") >= 0) {
    if (pwmv < 255){
      pwmv=pwmv+rtemp;
      outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
      pwmust1 = pwmv;
    } else if(pwmv == 255){
        pwmv = 0;
       outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
       pwmust1 = pwmv;
    }
    
  }
    if (incStr.indexOf("b11") >= 0) {
    if (pwmv > 0 ){
      pwmv=pwmv-rtemp;
      outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
      pwmust1 = pwmv;
    } else if (pwmv == 0){
        pwmv = 255;
       outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
       pwmust1 = pwmv;
    }
    
  }
    if (incStr.indexOf("b12") >= 0) {
    if (pwmn < 255){
      pwmn=pwmn+rtemp;
      outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
      pwmust2 = pwmn;
    } else if(pwmn == 255){
        pwmn = 0;
       outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
       pwmust2 = pwmn;
    }
    
  }
    if (incStr.indexOf("b13") >= 0) {
    if (pwmn > 0 ){
      pwmn=pwmn-rtemp;
      outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
      pwmust2 = pwmn;
    } else if (pwmn == 0){
        pwmn = 255;
       outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
       pwmust2 = pwmn;
    }
    
  }  
    if (incStr.indexOf("b14") >= 0) {
    if (kp < 10000){
      kp=kp+rtemp;
      String t24 = "\"" + String(kp) + "\"";  // Отображение kp
      SendData("t24.txt", t24);
      Kp = kp;
    } else if(kp == 10000){
        kp = 0;
      String t24 = "\"" + String(kp) + "\"";  // Отображение kp
      SendData("t24.txt", t24);
       Kp = kp;
    }
    
  }
    if (incStr.indexOf("b15") >= 0) {
    if (kp > 0 ){
      kp=kp-rtemp;  
      String t24 = "\"" + String(kp) + "\"";  // Отображение kp
      SendData("t24.txt", t24);
      Kp = kp;
    } else if (kp == 0){
        kp = 10000;
       String t24 = "\"" + String(kp) + "\"";  // Отображение kp
      SendData("t24.txt", t24);
       Kp = kp;
    }
    
  }   
    if (incStr.indexOf("b16") >= 0) {
    if (ki < 10000){
      ki=ki+rtemp;
      String t25 = "\"" + String(ki) + "\"";  // Отображение ki
      SendData("t25.txt", t25);
      Ki = ki;
    } else if(ki == 10000){
        ki = 0;
        String t25 = "\"" + String(ki) + "\"";  // Отображение ki
        SendData("t25.txt", t25);
        Ki = ki;
    }
    
  }
    if (incStr.indexOf("b17") >= 0) {
    if (ki > 0 ){
      ki=ki-rtemp;
      String t25 = "\"" + String(ki) + "\"";  // Отображение ki
      SendData("t25.txt", t25);
      Ki = ki;
    } else if (ki == 0){
       ki = 10000;
       String t25 = "\"" + String(ki) + "\"";  // Отображение ki
       SendData("t25.txt", t25);
       Ki = ki;
    }
    
  }    
    if (incStr.indexOf("b18") >= 0) {
    if (kd < 10000){
      kd=kd+rtemp;
      String t26= "\"" + String(kd) + "\"";  // Отображение kd
      SendData("t26.txt", t26);
      Kd = kd;
    } else if(kd == 10000){
        kd = 0;
      String t26= "\"" + String(kd) + "\"";  // Отображение kd
      SendData("t26.txt", t26);
       Kd = kd;
    }
    
  }
    if (incStr.indexOf("b19") >= 0) {
    if (kd > 0 ){
      kd=kd-rtemp;
      String t26= "\"" + String(kd) + "\"";  // Отображение kd
      SendData("t26.txt", t26);
      Kd = kd;
    } else if (kd == 0){
        kd = 10000;
       String t26= "\"" + String(kd) + "\"";  // Отображение kd
      SendData("t26.txt", t26);
       Kd = kd;
    }
    
  }     
  
   if (incStr.indexOf("r1") >= 0) {
    r1=1;
    rtemp=r1;
  }   
    if (incStr.indexOf("r10") >= 0) {
    r10=10;
    rtemp=r10;
  }  
    if (incStr.indexOf("r100") >= 0) {
    r100=100;
    rtemp=r100;
  }    
  if (incStr.indexOf("b2") >= 0) {
    if (termoprofily < 5){
      termoprofily++;
      outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
      if (termoprofily == 0){
        profily="Lead-free"; 
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      } else if (termoprofily == 1){
        profily="Lead"; // Термопрофиль Свинец
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }else if (termoprofily == 2){
        profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }else if (termoprofily == 3){
        profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }else if (termoprofily == 4){
        profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }else if (termoprofily == 5){
        profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }
    } else if(termoprofily == 5){
       termoprofily = 0;
       outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
       if (termoprofily == 0){
        profily="Lead-free"; // Термопрофиль Бессвинец
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      } else if (termoprofily == 1){
        profily="Lead"; // Термопрофиль Свинец
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }else if (termoprofily == 2){
        profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }else if (termoprofily == 3){
        profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }else if (termoprofily == 4){
        profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }else if (termoprofily == 5){
        profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }
    }
    
  }
   if (incStr.indexOf("b3") >= 0) {
    if (termoprofily > 0){
      termoprofily--;
      outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
      if (termoprofily == 0){
        profily="Lead-free"; // Термопрофиль Бессвинец
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      } else if (termoprofily == 1){
        profily="Lead"; // Термопрофиль Свинец
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          
      }else if (termoprofily == 2){
        profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          
      }else if (termoprofily == 3){
        profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }else if (termoprofily == 4){
        profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          
      }else if (termoprofily == 5){
        profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          
      }
    } else if(termoprofily == 0){
       termoprofily = 5;
       outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
       if (termoprofily == 0){
        profily="Lead-free"; // Термопрофиль Бессвинец
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          
      } else if (termoprofily == 1){
        profily="Lead";  // Термопрофиль Свинец
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          
      }else if (termoprofily == 2){
        profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          
      }else if (termoprofily == 3){
        profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          
      }else if (termoprofily == 4){
        profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }else if (termoprofily == 5){
        profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          
      }
    }
    
  }
}

// функция пид автор https://alexgyver.ru/lessons/ 
int NizPID(float input, float setpoint, float kp, float ki, float kd, float dt, int minOut, int maxOut) {
  float err = setpoint - input;
  static float integral = 0, prevErr = 0;
  integral = constrain(integral + (float)err * dt * ki, minOut, maxOut);
  float D = (err - prevErr) / dt;
  prevErr = err;
  return constrain(err * kp + integral + D * kd, minOut, maxOut);
}

// функция пид автор https://alexgyver.ru/lessons/
int VerhPID(float input, float setpoint, float kp, float ki, float kd, float dt, int minOut, int maxOut) {
  float err = setpoint - input;
  static float integral = 0, prevErr = 0;
  integral = constrain(integral + (float)err * dt * ki, minOut, maxOut);
  float D = (err - prevErr) / dt;
  prevErr = err;
  return constrain(err * kp + integral + D * kd, minOut, maxOut);
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

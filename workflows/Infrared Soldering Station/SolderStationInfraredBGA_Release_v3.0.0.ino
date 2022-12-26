#include <EEPROM.h>
#include <GyverMAX6675.h> // Подключаем библиотеку работы с микросхемой MAX6675   автор https://alexgyver.ru/lessons/

#define INIT_ADDR 1023  // номер резервной ячейки
#define INIT_KEY 100     // ключ первого запуска. 0-254, на выбор
//#define ZERO_PIN 2  // Для обращения к выводу 2 указываем имя ZERO_PIN, порт для детектора нуля
#define INT_NUM 0     // соответствующий ему номер прерывания
#define nigniy_1 3  // указываем порты 3 вывода нижнего нагревателя с ШИМ
#define verhniy_1 5 // указываем верхний нагреватель порт 5 вывода с ШИМ
#define coolerv 9   // Кулер встроеный в верхний нагреватель 
#define cooler 10  // Для подключения кулера с ШИМ, для охлаждения всей платы
#define lampa 11   // Для подключения лампы с ШИМ, подстветка при пайке
//#define DIM_AMOUNT 2  // количество диммеров для детектора нуля
//const byte dimPins[] = {3, 5}; // их пины для детектора нуля
#include <GyverTimers.h>    // библиотека таймера

//int dimmer[DIM_AMOUNT];     // переменная диммера
//volatile int counter = 0;   // счётчик цикла
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
#define DATA_PIN2 4  // Пин SO
#define CS_PIN2   12  // Пин CS

// указываем пины в порядке SCK SO CS
GyverMAX6675<CLK_PIN2, DATA_PIN2, CS_PIN2> sens2;

#include <GyverRelay.h>
// установка, гистерезис, направление регулирования автор https://alexgyver.ru/lessons/
GyverRelay regulator(REVERSE); 
GyverRelay regulator2(REVERSE);


void outNumber(char *component, uint16_t number);
void outText(char *component, char *text);
void print_string(char *string);
void print_dec(uint16_t data);
void sendFFFFFF(void);

uint32_t myTimer0 = 0, myTimer1 = 0, myTimer2 = 0; // автор https://alexgyver.ru/lessons/

String incStr;    // объявляем переменую типа String не путать со string
String string;

float tempt1 = 0;      //  вывод температуры сдатчика на дисплей. облявляем переменную целочисленую для температуры 1, если нужно с дробной частью то объявляем float и переменной присваимваем 0
float tempt2 = 0;      // вывод температуры сдатчика на дисплей. температура 2 целочисленная, для дробной заменить int на float и переменной присваимваем 0
int temp = 0; // temp используется для активации нужной страницы и подсчета наней температуры, если temp = 0, то температуру не подсчитываем 
unsigned int temp1 = 225; // температура по умолчанию верхнего нагревателя 
unsigned int temp2 = 160; // температура по умолчанию нижнего нагревателя
unsigned int tempust1 = 0;     // установленая температура 'C градусов цельсия должна считываться с дисплея Nextion
unsigned int tempust2 = 0;     // установленая температура 'C градусов цельсия должна считываться с дисплея Nextion 
byte pwmv = 76; // ШИМ верхнего нагревателя по умолчанию 30% == в ШИМ = 76 дробную часть отбрасываем 76,5
byte pwmn = 76; // ШИМ нижнего нагревателя по умолчанию 30% == в ШИМ = 76 дробную часть отбрасываем 76,5
byte pwmust1 = 0; // Установленый ШИМ
byte pwmust2 = 0; // Установленый ШИМ
byte coolervh = 76;
byte coolerp = 76;
byte coolvust1 = 0;
byte coolpust2 = 0;
float kp = 5.00; // ПИД регулирование порпорциональное   значение по умолчанию
float ki = 3.00; // ПИД регулирование интегральное       значение по умолчанию 
float kd = 3.00;  // ПИД регулирование дифферинциальное   значение по умолчанию
float Kp = 0.00;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
float Ki = 0.00;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
float Kd = 0.00;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
bool reley_v; // Верхний нагреватель включение отдельно
bool reley_n; // Нижний нагреватель переменная для включения паяльной станции верхний нагреватель нижний нагреватель, либо по отдельности Верх нагр. Нижний нагр.
bool reley_n1; // Нижний нагреатель включение отдельно
bool ph; //  пидрегулирование или гистерезис переключение между ПИД и Гистерезис регулирование
int pidn; // ПИД нижнего нагревателя
int hestn; // Гистерезис нижнего нагревателя
float r100; //  увеличивается или уменьшается сотнями то есть +100 или -100
float r10;  //  увеличивается или уменьшается десятками то есть +10 или -10
float r1;   //  увеличивается или уменьшается единицами то есть +1 или -1
float r01;
float r001;
float rtemp; // переменая где хранятся 100, 10, 1, 0.1, 0.01
int termoprofily = 0; // Номер термопрофиля по умолчанию выбран 0
String profily="Lead-free"; // Загружает Бессвинцовый термопрофиль по умолчанию
int shag = 0;
uint32_t sec = 0;
bool termoprofily0_1 = 0;
bool termoprofily1_9 = 0;
bool termoprofily10 = 0;
bool bt0 = 0;

void setup(void) {
  /**
  детектор нуля
  pinMode(ZERO_PIN, INPUT_PULLUP);
  for (byte i = 0; i < DIM_AMOUNT; i++) pinMode(dimPins[i], OUTPUT);
  attachInterrupt(INT_NUM, isr, FALLING); // для самодельной схемы ставь RISING
  Timer2.disableISR();
  // 37 мкс - период прерываний для 255 шагов и 50 Гц
  // для 60 Гц ставь число 31
  Timer2.setPeriod(37); 
  **/
  Serial.begin(9600); // Указваем скорость UART 9600 бод

  pinMode(nigniy_1, OUTPUT); // нижний нагреватель номер 1 настраиваем на выход
  analogWrite(nigniy_1, 0); // отключаем выход
  pinMode(verhniy_1, OUTPUT); // верхний нагреватель настраиваем на выход
  analogWrite(verhniy_1, 0); // отключаем выход, то есть не подаем пять вольт, подовать будем поже
  pinMode(coolerv, OUTPUT);    // тоже самое настраиваем вывод на выход
  analogWrite(coolerv, 0);  // отключаем вывод
  pinMode(cooler, OUTPUT);    // тоже самое настраиваем вывод на выход
  analogWrite(cooler, 0);  // отключаем вывод
  pinMode(lampa, OUTPUT);    // тоже самое настраиваем вывод на выход
  digitalWrite(lampa, LOW);  // отключаем вывод

  if (EEPROM.read(INIT_ADDR) != INIT_KEY) { // первый запуск
    EEPROM.write(INIT_ADDR, INIT_KEY);    // записали ключ
    // записали стандартное значение пид
    // в данном случае это значение переменной, объявленное выше
    temp1 = 0; // температура по умолчанию верхнего нагревателя 
    temp2 = 0; // температура по умолчанию нижнего нагревателя
    EEPROM.put(187, pwmv);
    EEPROM.put(192, pwmn);
    EEPROM.put(7, kp);
    EEPROM.put(12, ki);
    EEPROM.put(17, kd);
    
    EEPROM.put(22, sec);
    EEPROM.put(27, temp1);
    EEPROM.put(32, temp2);
    
    EEPROM.put(37, sec);
    EEPROM.put(42, temp1);
    EEPROM.put(47, temp2);
    
    EEPROM.put(52, sec);
    EEPROM.put(57, temp1);
    EEPROM.put(62, temp2);
    
    EEPROM.put(67, sec);
    EEPROM.put(72, temp1);
    EEPROM.put(77, temp2);
    
    EEPROM.put(82, sec);
    EEPROM.put(87, temp1);
    EEPROM.put(92, temp2);
    
    EEPROM.put(97, sec);
    EEPROM.put(102, temp1);
    EEPROM.put(107, temp2);
    
    EEPROM.put(112, sec);
    EEPROM.put(117, temp1);
    EEPROM.put(122, temp2);
    
    EEPROM.put(127, sec);
    EEPROM.put(132, temp1);
    EEPROM.put(137, temp2);
    
    EEPROM.put(142, sec);
    EEPROM.put(147, temp1);
    EEPROM.put(152, temp2);
    
    EEPROM.put(157, sec);
    EEPROM.put(162, temp1);
    EEPROM.put(167, temp2);
    
    EEPROM.put(172, sec);
    EEPROM.put(177, temp1);
    EEPROM.put(182, temp2);

    EEPROM.put(197, coolervh);
    EEPROM.put(202, coolerp);
    EEPROM.put(207, termoprofily);
  }

  EEPROM.get(187, pwmv);
  EEPROM.get(192, pwmn);
  EEPROM.get(7, kp);
  EEPROM.get(12, ki);
  EEPROM.get(17, kd);
  EEPROM.get(197, coolervh);
  EEPROM.get(202, coolerp);
  //EEPROM.get(207, termoprofily);
  if (EEPROM.get(207, termoprofily) == 0){
         shag = 0;
         sec = 3;    
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
  } else if (EEPROM.get(207, termoprofily) == 1){
         shag = 0;
         sec = 3;
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;        
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
  }else if (EEPROM.get(207, termoprofily) == 2){
         EEPROM.get(22, sec);
         EEPROM.get(27, temp1);
         EEPROM.get(32, temp2);
         shag = 0;
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
  }
  //temp1 = 225; // температура по умолчанию верхнего нагревателя 
  //temp2 = 160; // температура по умолчанию нижнего нагревателя
  tempust1 = temp1;
  tempust2 = temp2;
  pwmust1 = pwmv;
  pwmust2 = pwmn;
  coolvust1 = coolervh;
  coolpust2 = coolerp;
  Kp = kp;
  Ki = ki;
  Kd = kd;
  shag = 0;
  sec = 0;
  
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
      
      /**
      //outNumber("termoprofily.temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      //outNumber("termoprofily.temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      outNumber("termoprofily.n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
      outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
      outNumber("watt.pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
      outNumber("watt.pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
      String t13= "\"" + String(profily) + "\"";  // Отображение 
      SendData("termoprofily.t13.txt", t13);
      String t24 = "\"" + String(Kp) + "\"";  // выводим пропорциональное
      SendData("pid.t24.txt", t24);
      String t25 = "\"" + String(Ki) + "\"";  // выводим интегральное
      SendData("pid.t25.txt", t25);
      String t26= "\"" + String(Kd) + "\"";  // выводим дефференциальное
      SendData("pid.t26.txt", t26);  
      **/   
  } else if((incStr.indexOf("01"))>=0)
  {
    temp = 0;
    
  } else if((incStr.indexOf("02"))>=0)
  {
      temp = 0;
  }else if((incStr.indexOf("03"))>=0)
  {
      temp = 0;
  }else if((incStr.indexOf("04"))>=0) // когда находимся на странице 4 обновляем компоненты
  {
      temp = 0;
      
      outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
      String t13= "\"" + String(profily) + "\"";  // Отображение 
      SendData("t13.txt", t13);
      outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
      //outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
      /**
      if(reley_n==1){
          tsw_termoprofily_off();
       }else{
          tsw_termoprofily_on();
       } 
      **/
      
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
       String t24 = "\"" + String(Kp) + "\"";  // выводим пропорциональное
       SendData("t24.txt", t24);
       String t25 = "\"" + String(Ki) + "\"";  // выводим интегральное
       SendData("t25.txt", t25);
       String t26= "\"" + String(Kd) + "\"";  // выводим дефференциальное
       SendData("t26.txt", t26);
      
  }else if((incStr.indexOf("08"))>=0) // когда находимся на странице 7 обновляем компоненты
  { 
       temp = 0;
       outNumber("coolervh.val", coolervh);  // Отображение числа в числовом компоненте coolervh
       outNumber("coolerp.val", coolerp);  // Отображение числа в числовом компоненте coolerp
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
            if(bt0==1) // Кнопка bt0 равна 1
            {
              page_main();
              bt0_click();
              bt0 = 0;
            }
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
            if(bt0==1) // Кнопка bt0 равна 1
            {
              page_main();
              bt0_click();
              bt0 = 0;
            }
          }
          delay(1000); 
          
      }
  }

  // задаём значение
  //dimmer[0] = map(analogRead(A0), 0, 1023, 0, 9500);
  //dimmer[0] = 50;
  //dimmer[1] = 120;
  //delay(100); // в реальном коде задержек быть не должно
  
  
  
  regulator.setpoint = tempust1;    // установка (температуры)
  regulator.hysteresis = 0.25;   // ширина гистерезиса
  regulator.k = 0.5;          // коэффициент обратной связи (подбирается по факту)
  regulator.dT = 500;       // установить время итерации для getResultTimer
  regulator2.setpoint = tempust2;    // установка (температуры)
  regulator2.hysteresis = 0.25;   // ширина гистерезиса
  regulator2.k = 0.5;          // коэффициент обратной связи (подбирается по факту)
  regulator2.dT = 500;       // установить время итерации для getResultTimer
  
  if (millis() >= myTimer0 + 1*1000) {   // таймер на 80000 мс (1 раза в 80 сек) автор таймера https://alexgyver.ru/lessons/
      //myTimer2 = millis();
      if (ph==1){
        if(reley_n==1){
          //Timer2.enableISR();
          if (reley_n1==1){
            nigniye(); // Пид регулирование
          }
          if(reley_v==1){
            verhniy(); // Пид регулирование
          } 
        }
  }else {
      if(reley_n==1){
          //Timer2.enableISR();
        if (reley_n1==1){
          reguln();  // Гистерезис
        }          
        if(reley_v==1){
          regul();   // Гистерезис      
        }
      }
  }
      myTimer0 = millis();
    }
    
  if (termoprofily1_9 == 1){ 
    if (millis() >= myTimer1 + sec*1000) {   // таймер на 80000 мс (1 раза в 80 сек) автор таймера https://alexgyver.ru/lessons/
      //myTimer1 = millis();
      termoprofily0_1_9();
      termoprofily1_9 = 0;   // выполнить действие 1
      myTimer1 = millis();
    }
  }
  if (termoprofily10 == 1){ 
    if (millis() >= myTimer2 + sec*1000) {   // таймер на 80000 мс (1 раза в 80 сек) автор таймера https://alexgyver.ru/lessons/
      //myTimer2 = millis();
      termoprofily_10();
      termoprofily10 = 0;   // выполнить действие 1
      myTimer2 = millis();
    }
  }
  // при достижении температуры 190 и 220 градусов ордуино кликает кнопку sound после чего начинает работать либо светодиод либо пищалка
  if ((190 <= tempt1) && (193 >= tempt1)){  // пока находится в диапазоне 190-193 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();       
  }else if ((220 <= tempt1) && (223 >= tempt1)){  // пока находится в диапазоне 220-223 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();   
  }

  if (tempust1 == 0){
      reley_v=0; analogWrite(verhniy_1, 0);
    } else if(incStr.indexOf("c3-on") >= 0) { reley_v=1; }  
  if (tempust2 == 0){
      reley_n1=0; analogWrite(nigniy_1, 0);
    } else if(incStr.indexOf("c0-on") >= 0) { reley_n1=1; }  
  if(reley_n==1){
    if ((tempust1 == 0) && (tempust2 == 0)){
      termoprofily10 = 1;
      shag = 0;
      reley_n=0;
      reley_n1=0;
      reley_v=0;
      analogWrite(nigniy_1, 0);
      analogWrite(verhniy_1, 0);
    }
  } 
  /**
  if (!(Serial.available() && sens.readTemp() && sens2.readTemp())){
     
     int Value1 = tempt1;
     print_string("add 3,0,");
     print_dec(Value1);
     sendFFFFFF();  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
     delay(8);
        
     int Value2 = tempt2;
     print_string("add 3,1,");
     print_dec(Value2);
     sendFFFFFF();  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
     delay(8);
  }
  **/
}
/**
// прерывание детектора нуля
void isr() {
  counter = 255;
  Timer2.restart();
}
// прерывание таймера
ISR(TIMER2_A) {
  for (byte i = 0; i < DIM_AMOUNT; i++) {
    if (counter == dimmer[i]) digitalWrite(dimPins[i], 1);  // на текущем тике включаем
    else if (counter == dimmer[i] - 1) digitalWrite(dimPins[i], 0);  // на следующем выключаем
  }
  counter--;
}
**/

void yield() {
  // а тут можно опрашивать кнопку
  // и не пропустить нажатия из за delay!
   
}


// этот код и остальной нужен

void termoprofily0_1_9(void){
  page_termoprofily();
  delay(10);
  b4_click();
  delay(10);
  page_main();
  delay(10);
  
}
void termoprofily_10(void){
  page_termoprofily();
  delay(10);
  b4_click();
  delay(10);
  page_main();
  delay(10);
  bt0_click();  
}

void sound_click(void){
  Serial.print("click sound,1");
  Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  Serial.write(0xff);
  Serial.write(0xff);  
}

void bt0_click(void){
  Serial.print("click bt0,1");
  Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  Serial.write(0xff);
  Serial.write(0xff); 
  Serial.print("click bt0,0");
  Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  Serial.write(0xff);
  Serial.write(0xff);  
}
void b4_click(void){
  Serial.print("click b4,1");
  Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  Serial.write(0xff);
  Serial.write(0xff);
}
void page_main(void){
  Serial.print("page main");
  Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  Serial.write(0xff);
  Serial.write(0xff);
}
void page_termoprofily(void){
  Serial.print("page termoprofily");
  Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  Serial.write(0xff);
  Serial.write(0xff);
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
  print_string("=");    // Открывающая двойная ковычка
  print_string("\"");
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
  //Serial.write(num + '0');              // Печать десятков тысяч в ASCII
  for(num=0; data >= 1000; num++) data -= 1000; // Выделение тысяч в переменной num
  //Serial.write(num + '0');              // Печать тысяч в ASCII
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
  //delay(20); 
}
// Пид регулирование верхний нагреватель
void verhniy(){
     // (вход, установка, п, и, д, период в секундах, мин.выход, макс. выход)
     analogWrite(verhniy_1, VerhPID(tempt1, tempust1, Kp, Ki, Kd, 0.02, 0, pwmust1)); 
     //delay(20);
}
// Гистерезис нижний нагреватель
void reguln(){
    regulator2.input =  tempt2; 
    digitalWrite(nigniy_1, regulator2.getResultTimer());   // отправляем на реле (ОС работает по своему таймеру)
 
}
// Гистерезис верхний нагреватель
void regul() {
  
    regulator.input =  tempt1;
    digitalWrite(verhniy_1, regulator.getResultTimer());   // отправляем на реле (ОС работает по своему таймеру)
  
}

void AnalyseString(String incStr) {
  if (incStr.indexOf("bt0-on") >= 0) {     // слушаем UART на передачу команды bt0-on подаем 5 вольт на вывод
    reley_n=1;
    sec = 3;
    termoprofily1_9 = 1;
    bt0 = 1;
    //digitalWrite(nigniy_1, HIGH);
  } else if (incStr.indexOf("bt0-off") >= 0) { //слушаем UART на команду bt0-off и снимаем 5 вольт с вывода
    reley_n=0;
    bt0 = 0;
    Timer2.disableISR();
    shag = 0;
    //sec=0;
    termoprofily1_9 = 0;
    termoprofily10 = 0;
    if (termoprofily == 0){
         shag = 0;
         sec=3;
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
    } else if (termoprofily == 1){
         shag = 0;      
         sec=3;
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2; 
         outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
    } else if (termoprofily == 2){
         EEPROM.get(22, sec);
         EEPROM.get(27, temp1);
         EEPROM.get(32, temp2);
         shag = 0;         
         //sec=0;
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
        // temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
        // temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
    } 
    //digitalWrite(nigniy_1, LOW);
  }
  if (incStr.indexOf("c0-on") >= 0) {      // тоже самое что и bt0
    reley_n1=1;
  } else if (incStr.indexOf("c0-off") >= 0) {
    reley_n1=0;
    analogWrite(nigniy_1, 0);
  }
  if (incStr.indexOf("c3-on") >= 0) { 
    reley_v=1;
  } else if ((incStr.indexOf("c3-off")) >= 0) {
    reley_v=0;
    analogWrite(verhniy_1, 0);
  }
  if (incStr.indexOf("coolerv-on") >= 0) {      // тоже самое что и bt0
      analogWrite(coolerv, coolvust1);
  }  else if (incStr.indexOf("coolerv-off") >= 0) {
      analogWrite(coolerv, 0);
  }
  if (incStr.indexOf("cooler-on") >= 0) {      // тоже самое что и bt0
     analogWrite(cooler, coolpust2);
  }  else if (incStr.indexOf("cooler-off") >= 0) {
     analogWrite(cooler, 0);
  }
  if (incStr.indexOf("bt1-on") >= 0) {      // тоже самое что и bt0
     digitalWrite(lampa, HIGH);
  }  else if (incStr.indexOf("bt1-off") >= 0) {
     digitalWrite(lampa, LOW);
  }
   if (incStr.indexOf("pid") >= 0) {     // выбран пид регулирование
     ph=1;
    
  } else if (incStr.indexOf("hesterezis") >= 0) { // выбран гистерезис
     ph=0;
  }
  if (incStr.indexOf("ntermo") >= 0) {     // выбран пид регулирование
     EEPROM.put(207, termoprofily);
  }
  if (incStr.indexOf("wattsave") >= 0) {     // Сохранение Мощности в eeprom память по умолчанию
       EEPROM.put(187, pwmv);
       EEPROM.put(192, pwmn);
    
  } 
  if (incStr.indexOf("coolerssave") >= 0) {     // Сохранение Мощности в eeprom память по умолчанию
       EEPROM.put(197, coolervh);
       EEPROM.put(202, coolerp);
    
  } 
  if (incStr.indexOf("pidsave") >= 0) {     // Сохранение ПИД регулирование, П И Д составляющих в eeprom память по умолчанию
     EEPROM.put(7, kp);
     EEPROM.put(12, ki);
     EEPROM.put(17, kd);
  } 
  if (incStr.indexOf("termosave") >= 0) {     // Сохранение ПИД регулирование, П И Д составляющих в eeprom память по умолчанию
     if(shag == 0){
       if (termoprofily == 2){
         EEPROM.put(22, sec);
         EEPROM.put(27, temp1);
         EEPROM.put(32, temp2);
       } 
    } else if(shag == 1){
       if (termoprofily == 2){
         EEPROM.put(37, sec);
         EEPROM.put(42, temp1);
         EEPROM.put(47, temp2);
      
        } 
    } else if(shag == 2){
       if (termoprofily == 2){
         EEPROM.put(52, sec);
         EEPROM.put(57, temp1);
         EEPROM.put(62, temp2);
        }
    } else if(shag == 3){
       if (termoprofily == 2){
         EEPROM.put(67, sec);
         EEPROM.put(72, temp1);
         EEPROM.put(77, temp2);
        }
    } else if(shag == 4){
       if (termoprofily == 2){
         EEPROM.put(82, sec);
         EEPROM.put(87, temp1);
         EEPROM.put(92, temp2);
        }
    } else if(shag == 5){
       if (termoprofily == 2){
         EEPROM.put(97, sec);
         EEPROM.put(102, temp1);
         EEPROM.put(107, temp2);
       } 
    } else if(shag == 6){
       if (termoprofily == 2){
         EEPROM.put(112, sec);
         EEPROM.put(117, temp1);
         EEPROM.put(122, temp2);
        } 
       
    } else if(shag == 7){
       if (termoprofily == 2){
         EEPROM.put(127, sec);
         EEPROM.put(132, temp1);
         EEPROM.put(137, temp2);
       } 
       
    } else if(shag == 8){
       if (termoprofily == 2){
         EEPROM.put(142, sec);
         EEPROM.put(147, temp1);
         EEPROM.put(152, temp2);
       } 
       
    } else if(shag == 9){
       if (termoprofily == 2){
          EEPROM.put(157, sec);
          EEPROM.put(162, temp1);
          EEPROM.put(167, temp2);
       } 
    } else if(shag == 10){
        if (termoprofily == 2){
          EEPROM.put(172, sec);
          EEPROM.put(177, temp1);
          EEPROM.put(182, temp2);
       }
    } 
     
  } 
  
  if (incStr.indexOf("b4") >= 0) {
    if (shag < 10){
      shag++;
      outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
      if(shag == 0){
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         //shag = 0;
         sec=3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
      } else if (termoprofily == 1){
         //shag = 0;
         sec=3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2; 
         if (reley_n==1){
           termoprofily1_9 = 1;
         }       
      } else if (termoprofily == 2){
          EEPROM.get(22, sec);
          EEPROM.get(27, temp1);
          EEPROM.get(32, temp2);
         // shag = 0;
         //sec=0;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
       } 
     } else if(shag == 1){
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         sec=60; 
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 50; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
      } else if (termoprofily == 1){
         sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 50; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }        
      } else if (termoprofily == 2){
         EEPROM.get(37, sec);
         EEPROM.get(42, temp1);
         EEPROM.get(47, temp2);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
        }
    } else if(shag == 2){
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         sec=60; 
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 80; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
      } else if (termoprofily == 1){
         sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 80; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }        
      } else if (termoprofily == 2){
         EEPROM.get(52, sec);
         EEPROM.get(57, temp1);
         EEPROM.get(62, temp2);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
        }
    } else if(shag == 3){
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         sec=60;  
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 100; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1){
         sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 100; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;  
         if (reley_n==1){
           termoprofily1_9 = 1;
         }      
      } else if (termoprofily == 2){
        EEPROM.get(67, sec);
        EEPROM.get(72, temp1);
        EEPROM.get(77, temp2);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 120; // Нижний нагреватель Бессвинецовый выбрано 120 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
        }
    } else if(shag == 4){
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         sec=60;  
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 120; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1){
         sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 120; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2; 
         if (reley_n==1){
           termoprofily1_9 = 1;
         }       
      } else if (termoprofily == 2){
         EEPROM.get(82, sec);
         EEPROM.get(87, temp1);
         EEPROM.get(92, temp2);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 140; // Нижний нагреватель Бессвинецовый выбрано 140 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
        }
    } else if(shag == 5){
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         sec=60;  
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 140; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1){
         sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 140; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;   
         if (reley_n==1){
           termoprofily1_9 = 1;
         }     
      } else if (termoprofily == 2){
          EEPROM.get(97, sec);
          EEPROM.get(102, temp1);
          EEPROM.get(107, temp2);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
       } 
    } else if(shag == 6){
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         sec=60; 
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1){
         sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 150; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }        
      } else if (termoprofily == 2){
          EEPROM.get(112, sec);
          EEPROM.get(117, temp1);
          EEPROM.get(122, temp2);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
       } 
       
    } else if(shag == 7){
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         sec=60; 
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 160; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1){
         sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2; 
         if (reley_n==1){
           termoprofily1_9 = 1;
         }       
      } else if (termoprofily == 2){
         EEPROM.get(127, sec);
         EEPROM.get(132, temp1);
         EEPROM.get(137, temp2);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
       } 
       
    } else if(shag == 8){
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         sec=60; 
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 180; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1){
         sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 160; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;   
         if (reley_n==1){
           termoprofily1_9 = 1;
         }     
      } else if (termoprofily == 2){
         EEPROM.get(142, sec);
         EEPROM.get(147, temp1);
         EEPROM.get(152, temp2);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
       } 
       
    } else if(shag == 9){
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         sec=60; 
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1){
         sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 180; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2; 
         if (reley_n==1){
           termoprofily1_9 = 1;
         }       
      } else if (termoprofily == 2){
         EEPROM.get(157, sec);
         EEPROM.get(162, temp1);
         EEPROM.get(167, temp2);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
       } 
    } else if(shag == 10){
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         sec=60;  
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
      } else if (termoprofily == 1){
         sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2; 
         if (reley_n==1){
           termoprofily1_9 = 1;
         }    
      } else if (termoprofily == 2){
         EEPROM.get(172, sec);
         EEPROM.get(177, temp1);
         EEPROM.get(182, temp2);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily1_9 = 1;
         }
       } 
       }
    } else {
         shag = 0;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0){
         shag = 0;
         sec=3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead-free"; 
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily10 = 1;
           reley_n=0;
           reley_n1=0;
           reley_v=0;
           analogWrite(nigniy_1, 0);
           analogWrite(verhniy_1, 0);
         }
         
      } else if (termoprofily == 1){
         shag = 0;
         sec=3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="Lead"; // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2; 
         if (reley_n==1){
           termoprofily10 = 1;
           reley_n=0;
           reley_n1=0;
           reley_v=0;
           analogWrite(nigniy_1, 0);
           analogWrite(verhniy_1, 0);
         }       
      } else if (termoprofily == 2){
         EEPROM.get(22, sec);
         EEPROM.get(27, temp1);
         EEPROM.get(32, temp2);
         shag = 0;
         //sec=0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
        // temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 0 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1){
           termoprofily10 = 1;
           reley_n=0;
           reley_n1=0;
           reley_v=0;
           analogWrite(nigniy_1, 0);
           analogWrite(verhniy_1, 0);
         }
       }
    }  
  }
    if (incStr.indexOf("bs20") >= 0) {
    if (sec < 480){
       sec++;
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
    } else if(sec == 480){
        sec = 0;
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
    }
    
  }
    if (incStr.indexOf("bs21") >= 0) {
    if (sec > 0 ){
      sec--;
      outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
    } else if (sec == 0){
        sec = 480;
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
    }
    
  }
  if (incStr.indexOf("b6") >= 0) {
    if (temp1 <= 300){
      temp1=temp1+rtemp;
      outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      tempust1 = temp1;
    } else if(temp1 <= 300){
        temp1 = 0;
       outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
       tempust1 = temp1;
    }
    
  }
    if (incStr.indexOf("b7") >= 0) {
    if (temp1 >= 0 ){
      temp1=temp1-rtemp;
      outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      tempust1 = temp1;
    } else if (temp1 >= 0){
        temp1 = 300;
       outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
       tempust1 = temp1;
    }
    
  }
    if (incStr.indexOf("b8") >= 0) {
    if (temp2 <= 300){
      temp2=temp2+rtemp;
      outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      tempust2 = temp2;
    } else if (temp2 <= 300){
        temp2 = 0;
       outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
       tempust2 = temp2;
    }
    
  }
    if (incStr.indexOf("b9") >= 0) {
    if (temp2 >= 0){
      temp2=temp2-rtemp;
      outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      tempust2 = temp2;
    } else if (temp2 >= 0){
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
    if (kp < 10000.0){
      kp=kp+rtemp;
      Kp = kp;
      String t24 = "\"" + String(kp) + "\"";  // Отображение kp
      SendData("t24.txt", t24);
      
    } else if(kp == 10000.0){
        kp = 0.00;
        Kp = kp;
      String t24 = "\"" + String(kp) + "\"";  // Отображение kp
      SendData("t24.txt", t24);
      
    }
    
  }
    if (incStr.indexOf("b15") >= 0) {
    if (kp > 0.00 ){
      kp=kp-rtemp; 
      Kp = kp; 
      String t24 = "\"" + String(kp) + "\"";  // Отображение kp
      SendData("t24.txt", t24);
      
    } else if (kp == 0.00){
        kp = 10000.0;
        Kp = kp;
        String t24 = "\"" + String(kp) + "\"";  // Отображение kp
        SendData("t24.txt", t24);
        
    }
    
  }   
    if (incStr.indexOf("b16") >= 0) {
    if (ki < 10000.0){
      ki=ki+rtemp;
      Ki = ki;
      String t25 = "\"" + String(ki) + "\"";  // Отображение ki
      SendData("t25.txt", t25);
      
    } else if(ki == 10000.0){
        ki = 0.00;
        Ki = ki;
        String t25 = "\"" + String(ki) + "\"";  // Отображение ki
        SendData("t25.txt", t25);
        
    }
    
  }
    if (incStr.indexOf("b17") >= 0) {
    if (ki > 0.00 ){
      ki=ki-rtemp;
      Ki = ki;
      String t25 = "\"" + String(ki) + "\"";  // Отображение ki
      SendData("t25.txt", t25);
      
    } else if (ki == 0.00){
       ki = 10000.0;
       Ki = ki;
       String t25 = "\"" + String(ki) + "\"";  // Отображение ki
       SendData("t25.txt", t25);
       
    }
    
  }    
  if (incStr.indexOf("b18") >= 0) {
    if (kd < 10000.0){
      kd=kd+rtemp;
      Kd = kd;
      String t26 = "\"" + String(kd) + "\"";  // Отображение kd
      SendData("t26.txt", t26);
    } else if(kd == 10000.0){
        kd = 0.00;
        Kd = kd;
        String t26 = "\"" + String(kd) + "\"";  // Отображение kd
        SendData("t26.txt", t26);
        
    }
    
  }
    if (incStr.indexOf("b19") >= 0) {
      if (kd > 0.00 ){
        kd=kd-rtemp;
        Kd = kd;
        String t26 = "\"" + String(kd) + "\"";  // Отображение kd
        SendData("t26.txt", t26);
      } else if (kd == 0.00){
         kd = 10000.0;
         Kd = kd;
         String t26 = "\"" + String(kd) + "\"";  // Отображение kd
         SendData("t26.txt", t26);
       
      }
    } 
  if (incStr.indexOf("bcv") >= 0) {
    if (coolervh < 255){
      coolervh=coolervh+rtemp;
      outNumber("coolervh.val", coolervh);  // Отображение числа в числовом компоненте coolervh
      coolvust1 = coolervh;
    } else if(coolervh == 255){
        coolervh = 0;
       outNumber("coolervh.val", coolervh);  // Отображение числа в числовом компоненте coolervh
       coolvust1 = coolervh;
    }
    
  }
    if (incStr.indexOf("bv2") >= 0) {
    if (coolervh > 0 ){
      coolervh=coolervh-rtemp;
      outNumber("coolervh.val", coolervh);  // Отображение числа в числовом компоненте coolervh
      coolvust1 = coolervh;
    } else if (coolervh == 0){
       coolervh = 255;
       outNumber("coolervh.val", coolervh);  // Отображение числа в числовом компоненте coolervh
       coolvust1 = coolervh;
    }
    
  }

  if (incStr.indexOf("bcp") >= 0) {
    if (coolerp < 255){
      coolerp=coolerp+rtemp;
      outNumber("coolerp.val", coolerp);  // Отображение числа в числовом компоненте coolerp
      coolpust2 = coolerp;
    } else if(coolerp == 255){
        coolerp = 0;
       outNumber("coolerp.val", coolerp);  // Отображение числа в числовом компоненте coolerp
       coolpust2 = coolerp;
    }
    
  }
    if (incStr.indexOf("bp2") >= 0) {
    if (coolerp > 0 ){
      coolerp=coolerp-rtemp;
      outNumber("coolerp.val", coolerp);  // Отображение числа в числовом компоненте coolerp
      coolpust2 = coolerp;
    } else if (coolerp == 0){
        coolerp = 255;
       outNumber("coolerp.val", coolerp);  // Отображение числа в числовом компоненте coolerp
       coolpust2 = coolerp;
    }
    
  }
  
   if (incStr.indexOf("r1") >= 0) {
    r1=1.0;
    rtemp=r1;
  }   
    if (incStr.indexOf("r10") >= 0) {
    r10=10.0;
    rtemp=r10;
  }  
    if (incStr.indexOf("ro10") >= 0) {
    r100=100.0;
    rtemp=r100;
  }    
  if (incStr.indexOf("r01") >= 0) {
    r01=0.1;
    rtemp=r01;
  }   
    if (incStr.indexOf("ro01") >= 0) {
    r001=0.01;
    rtemp=r001;
  }     
  if (incStr.indexOf("b2") >= 0) {
    if (termoprofily < 2){
      termoprofily++;
      outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
      if (termoprofily == 0){
         shag = 0;
         sec = 3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
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
         shag = 0;
         sec = 3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec        
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
         EEPROM.get(22, sec);
         EEPROM.get(27, temp1);
         EEPROM.get(32, temp2);
         shag = 0;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
         profily="User 1";
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         
      }
    } else if(termoprofily == 2){
       termoprofily = 0;
       outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
       if (termoprofily == 0){
         shag = 0;
         sec = 3;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
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
         shag = 0;
         sec = 3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec        
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
         EEPROM.get(22, sec);
         EEPROM.get(27, temp1);
         EEPROM.get(32, temp2);
         shag = 0;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
         profily="User 1";
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
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
         shag = 0;
         sec = 3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec        
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
         shag = 0;
         sec = 3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec        
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
         EEPROM.get(22, sec);
         EEPROM.get(27, temp1);
         EEPROM.get(32, temp2);
         shag = 0;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
         profily="User 1";
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          
      }
    } else if(termoprofily == 0){
       termoprofily = 2;
       outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
       if (termoprofily == 0){
         shag = 0;
         sec = 3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
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
         shag = 0;
         sec = 3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec        
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
         EEPROM.get(22, sec);
         EEPROM.get(27, temp1);
         EEPROM.get(32, temp2);
         shag = 0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
         profily="User 1";
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
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

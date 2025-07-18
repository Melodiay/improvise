#include <GyverPID.h>
#include <PIDtuner.h>
#include <PIDtuner2.h>

#include <EEPROM.h>
#include <GyverMAX6675.h> // Подключаем библиотеку работы с микросхемой MAX6675   автор https://alexgyver.ru/lessons/
#include <GyverTimers.h>    // библиотека таймера

#ifndef ph == 0
#include <GyverRelay.h>
#else
#include <GyverPID.h>
#include <PIDtuner.h>
#include <PIDtuner2.h>
#endif

//#define compSerial Serial    // Передача температуры на компьютер для построения графика, заложено для будущего
#define nexSerial Serial1
#define nex2Serial Serial2  // График на втором дисплеи Nextion

#define INIT_ADDR 1023  // номер резервной ячейки
#define INIT_KEY 50     // ключ первого запуска. 0-254, на выбор
#define ZERO_PIN 2  // Для обращения к выводу 2 указываем имя ZERO_PIN, порт для детектора нуля
#define INT_NUM 0     // соответствующий ему номер прерывания
#define nigniy_1 3  // указываем порты 3 вывода нижнего нагревателя с ШИМ
#define verhniy_1 4 // указываем верхний нагреватель порт 5 вывода с ШИМ
#define coolerv 5   // Кулер встроеный в верхний нагреватель 
#define cooler 6  // Для подключения кулера с ШИМ, для охлаждения всей платы
#define lampa 7   // Для подключения лампы с ШИМ, подстветка при пайке
#define btn_start 8 // Подключение кнопки старт, Запускает пайку
#define btn_stop 9  // Останавливает пайку
#define relay_nigniy 10
#define relay_verhniy 11
#define DIM_AMOUNT 2  // количество диммеров для детектора нуля
const byte dimPins[] = {3, 4}; // их пины для детектора нуля

int dimmer[DIM_AMOUNT];     // переменная диммера
volatile int counter = 0;   // счётчик цикла


// Пины модуля MAX6675K верхняя термопара
#define CLK_PIN   22  // Пин SCK  указываем вывводы для программного ICP
#define DATA_PIN  23  // Пин SO
#define CS_PIN    24  // Пин CS

// указываем пины в порядке SCK SO CS
GyverMAX6675<CLK_PIN, DATA_PIN, CS_PIN> sens; // sens, CLK, DATA, CS если больше одного модуюя можно переменовать например в CLK1, DATA1, CS1, sens1
                                              // незабывать обращаться к sens1, а не к sens - это если не одна термопара
// Пины модуля MAX6675K нижняя термопара
#define CLK_PIN2  22  // Пин SCK указываем вывводы для программного ICP
#define DATA_PIN2 23  // Пин SO
#define CS_PIN2   25  // Пин CS

// указываем пины в порядке SCK SO CS
GyverMAX6675<CLK_PIN2, DATA_PIN2, CS_PIN2> sens2;






void outNumber(char *component, uint32_t number);
void outText(char *component, char *text);
void print_string(char *string);
void print_dec(uint32_t data);
void sendFFFFFF(void);

uint32_t myTimer0 = 0, myTimer1 = 0, myTimer2 = 0; // автор https://alexgyver.ru/lessons/


String incStr;    // объявляем переменую типа String не путать со string
String string;


char znak1 = '+';
char znak2 = '+';
float comptempt1 = 0.00; // Компенсация термопары номер 1, то есть верхнего нагревателя
float comptempt2 = 0.00; // Компенсация нижнего нагревателя термапара 2
float tempt1 = 0;      //  вывод температуры сдатчика на дисплей. облявляем переменную целочисленую для температуры 1, если нужно с дробной частью то объявляем float и переменной присваимваем 0
float tempt2 = 0;      // вывод температуры сдатчика на дисплей. температура 2 целочисленная, для дробной заменить int на float и переменной присваимваем 0
int temp = 0; // temp используется для активации нужной страницы и подсчета наней температуры, если temp = 0, то температуру не подсчитываем 
unsigned int temp1 = 225; // температура по умолчанию верхнего нагревателя 
unsigned int temp2 = 160; // температура по умолчанию нижнего нагревателя
unsigned int tempust1 = 0;     // установленая температура 'C градусов цельсия должна считываться с дисплея Nextion
unsigned int tempust2 = 0;     // установленая температура 'C градусов цельсия должна считываться с дисплея Nextion 
byte pwmv = 255; // ШИМ верхнего нагревателя по умолчанию 100% == в ШИМ = 255 
byte pwmn = 255; // ШИМ нижнего нагревателя по умолчанию 100% == в ШИМ = 255 
byte pwmust1 = 0; // Установленый ШИМ
byte pwmust2 = 0; // Установленый ШИМ
byte coolervh = 255; // ШИМ верхнего кулера в нагревателе 100% == ШИМ 255
byte coolerp = 255;  // ШИМ верхнего кулера в нагревателе 100% == ШИМ 255
byte coolvust1 = 0;
byte coolpust2 = 0;
float kpv = -8.3400000000; // ПИД регулирование порпорциональное   значение по умолчанию
float kiv =  1.1100000000; // ПИД регулирование интегральное       значение по умолчанию 
float kdv =  8.2100000000;  // ПИД регулирование дифферинциальное   значение по умолчанию
float Kpv = 0.00;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
float Kiv = 0.00;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
float Kdv = 0.00;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
float kpn = -11.8400000000; // ПИД регулирование порпорциональное   значение по умолчанию
float kin =   1.5800000000; // ПИД регулирование интегральное       значение по умолчанию 
float kdn =  11.6600000000;  // ПИД регулирование дифферинциальное   значение по умолчанию
float Kpn = 0.00;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
float Kin = 0.00;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
float Kdn = 0.00;  // ПИД регулирование для расчетов и подстановки в ПИД регулирование
float dtv = 0.10;  // dt верхнего нагревателя в ПИД регулировании поумлочанию 20 мк секунд
float Dtv = 0.00;  // dt верхнего нагревателя в ПИД регулировании, в переменную будет записываться dtv
float dtn = 0.10;  // dt нижнего нагревателя в ПИД регулировании поумлочанию 20 мк секунд
float Dtn = 0.00;  // dt нижнего нагревателя в ПИД регулировании, в переменную будет записываться dtn
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
float r0001;
float r00001;
float r000001;
float rtemp; // переменая где хранятся 100, 10, 1, 0.1, 0.01
int termoprofily = 0; // Номер термопрофиля по умолчанию выбран 0
String profily="Lead-free"; // Загружает Бессвинцовый термопрофиль по умолчанию
int shag = 0;
uint32_t sec = 0;
bool termoprofily0_1 = 0;
bool termoprofily1_9 = 0;
bool termoprofily10 = 0;
bool bt0 = 0;
bool flag = false;
uint32_t btnTimer = 0;
bool btnState;
bool detect_zero = 1; // Детектор ноля, когда равно 1 детектор ноля отключин, когда равно 0 то детектор ноля включин.
int shagt = 0;
float GradSecv = 0.0; // Набор верхнего нагревателя тепературы 2.0 градуса в секунду, можете изменить на свое значение хоть 1.5 градуса в секунду
float GradSecn = 0.0; // Набор нижнего нагревателя 1.0 Градус в Секунду
float gradsecv = 1.0; // Поумолчанию сохраняется 1 Градус в сек. Не забудьте, в Дисплеи на странице, Настроек выставить свое значение
float gradsecn = 1.0; // Поумолчанию сохраняется 1 Градус в сек. Не забудьте, в Дисплеи на странице, Настроек выставить свое значение

int SetOpov1 = 0;
int setopov1 = 0;
int SetOpov2 = 0;
int setopov2 = 0;
int SetOpov3 = 0;
int setopov3 = 0;
int SetOpov4 = 0;
int setopov4 = 0;

int ri100; //  увеличивается или уменьшается сотнями то есть +100 
int ri10;  //  увеличивается или уменьшается десятками то есть +10 
int ri1;   //  увеличивается или уменьшается единицами то есть +1
int itemp;
float hup = 0.25; // ширина гистерезиса
float kup = 0.5;  // коэффициент обратной связи (подбирается по факту)
int   dtup = 100; // ((установить время итерации для getResultTimer))
float hdown= 0.25; // ширина гистерезиса
float kdown = 0.5; // коэффициент обратной связи (подбирается по факту)
int   dtdown = 100;// установить время итерации для getResultTimer
float Hup = 0.00;   // ширина гистерезиса
float Kup = 0.00;  // коэффициент обратной связи (подбирается по факту)
int   DTup = 0;    // установить время итерации для getResultTimer
float Hdown= 0.00;  // ширина гистерезиса
float Kdown = 0.00; // коэффициент обратной связи (подбирается по факту)
int   DTdown = 0;   // установить время итерации для getResultTimer
bool  hnup = 1;
bool  hndown = 1;

bool nigniynagrev = 0;
bool verhniynagrev = 0;

GyverPID pid(Kpv, Kiv, Kdv, GradSecv, Dtv);
GyverPID pid2(Kpn, Kin, Kdn, GradSecn, Dtn);
GyverRelay regulator(REVERSEI); // установка, гистерезис, направление регулирования автор https://alexgyver.ru/lessons/
GyverRelay regulator2(REVERSEI);// установка, гистерезис, направление регулирования автор https://alexgyver.ru/lessons/
//PIDtuner2 tunerv;
//PIDtuner2 tunern;


void setup(void) 
{
  
  //детектор нуля
  pinMode(ZERO_PIN, INPUT_PULLUP);
  for (byte i = 0; i < DIM_AMOUNT; i++) pinMode(dimPins[i], OUTPUT);
  attachInterrupt(INT_NUM, isr, FALLING); // для самодельной схемы ставь RISING
  Timer2.disableISR();
  // 37 мкс - период прерываний для 255 шагов и 50 Гц  // Мягкое включение реле 37 мкс, жесткое включение строго перехода через ноль, попробовать поставить здесь 40 мкс
  // для 60 Гц ставь число 31 
  Timer2.setPeriod(40); // подставлять в (здесь значение ваше)
    
  Serial.begin(9600);   // Указваем скорость UART 9600 бод
  //compSerial.begin(9600);
  nex2Serial.begin(9600);
  Serial.begin(9600);   // Указваем скорость UART 9600 бод
  nexSerial.begin(9600);
  //pinMode(nigniy_1, OUTPUT); // нижний нагреватель номер 1 настраиваем на выход
  analogWrite(nigniy_1, 0); // отключаем выход
  //pinMode(verhniy_1, OUTPUT); // верхний нагреватель настраиваем на выход
  analogWrite(verhniy_1, 0); // отключаем выход, то есть не подаем пять вольт, подовать будем поже
  //pinMode(coolerv, OUTPUT);    // тоже самое настраиваем вывод на выход
  analogWrite(coolerv, 0);  // отключаем вывод
  //pinMode(cooler, OUTPUT);    // тоже самое настраиваем вывод на выход
  analogWrite(cooler, 0);  // отключаем вывод
  pinMode(lampa, OUTPUT);    // тоже самое настраиваем вывод на выход
  digitalWrite(lampa, LOW);  // отключаем вывод
  pinMode(btn_start, INPUT_PULLUP);
  pinMode(btn_stop, INPUT_PULLUP);
  pinMode(relay_nigniy, OUTPUT); // Настраиваем вывод на выход
  digitalWrite(relay_nigniy, LOW);  // отключаем вывод
  pinMode(relay_verhniy, OUTPUT); // Настраиваем вывод на выход
  digitalWrite(relay_verhniy, LOW);  // отключаем вывод


  if (EEPROM.read(INIT_ADDR) != INIT_KEY) 
  { // первый запуск
    //EEPROM.write(INIT_ADDR, INIT_KEY);    // записали ключ
    // записали стандартное значение пид
    // в данном случае это значение переменной, объявленное выше
    temp1 = 0; // температура по умолчанию верхнего нагревателя 
    temp2 = 0; // температура по умолчанию нижнего нагревателя
    pwmv = 255; // ШИМ верхнего нагревателя по умолчанию 100% == в ШИМ = 255 
    pwmn = 255; // ШИМ нижнего нагревателя по умолчанию 100% == в ШИМ = 255 
    kpv = -8.3400000000; // ПИД регулирование порпорциональное   значение по умолчанию
    kiv =  1.1100000000; // ПИД регулирование интегральное       значение по умолчанию 
    kdv =  8.2100000000;  // ПИД регулирование дифферинциальное   значение по умолчанию
    kpn = -11.8400000000; // ПИД регулирование порпорциональное   значение по умолчанию
    kin =   1.5800000000; // ПИД регулирование интегральное       значение по умолчанию 
    kdn =  11.6600000000;  // ПИД регулирование дифферинциальное   значение по умолчанию
    hup = 0.25; // ширина гистерезиса
    kup = 0.5;  // коэффициент обратной связи (подбирается по факту)
    dtup = 100; // ((установить время итерации для getResultTimer))
    hdown= 0.25; // ширина гистерезиса
    kdown = 0.5; // коэффициент обратной связи (подбирается по факту)
    dtdown = 100;// установить время итерации для getResultTimer

    //delay(10);
    //shag == 1 // termoprofily 2 верхний нагреватель
    EEPROM.put(7, kpv);
    EEPROM.put(12, kiv);
    EEPROM.put(17, kdv);
    //delay(10);
    EEPROM.put(22, sec);
    EEPROM.put(27, temp1);
    EEPROM.put(32, temp2);
    //delay(10);
    EEPROM.put(37, sec);
    EEPROM.put(42, temp1);
    EEPROM.put(47, temp2);
    //delay(10);
    EEPROM.put(52, sec);
    EEPROM.put(57, temp1);
    EEPROM.put(62, temp2);
    //delay(10);
    EEPROM.put(67, sec);
    EEPROM.put(72, temp1);
    EEPROM.put(77, temp2);
    //delay(10);
    EEPROM.put(82, sec);
    EEPROM.put(87, temp1);
    EEPROM.put(92, temp2);
    //delay(10);
    EEPROM.put(97, sec);
    EEPROM.put(102, temp1);
    EEPROM.put(107, temp2);
    //delay(10);
    EEPROM.put(112, sec);
    EEPROM.put(117, temp1);
    EEPROM.put(122, temp2);
    //delay(10);
    EEPROM.put(127, sec);
    EEPROM.put(132, temp1);
    EEPROM.put(137, temp2);
    //delay(10);
    EEPROM.put(142, sec);
    EEPROM.put(147, temp1);
    EEPROM.put(152, temp2);
    //delay(10);
    EEPROM.put(157, sec);
    EEPROM.put(162, temp1);
    EEPROM.put(167, temp2);
    //delay(10);
    EEPROM.put(172, sec);
    EEPROM.put(177, temp1);
    EEPROM.put(182, temp2);
    //delay(10);
    EEPROM.put(197, coolervh);
    EEPROM.put(202, coolerp);
    EEPROM.put(207, termoprofily);
    //shag == 1 // termoprofily 2 нижний нагреватель
    //delay(10);
    EEPROM.put(212, kpn);
    EEPROM.put(217, kin);
    EEPROM.put(222, kdn); 
    //delay(10);
    EEPROM.put(227, dtv); 
    EEPROM.put(232, dtn); 
    //delay(10);
    EEPROM.put(237, comptempt1);
    EEPROM.put(242, comptempt2);
    //delay(10);
    EEPROM.put(247, znak1);
    EEPROM.put(252, znak2);
    //delay(10);
    //TERMOPROFILY  ЗДЕСЬ НАЧИНАЕТСЯ КОД 
    //shag == 2 
    EEPROM.put(1030, kpv);
    EEPROM.put(1035, kiv);
    EEPROM.put(1040, kdv);
    //delay(10);
    //shag == 3
    EEPROM.put(1045, kpv);
    EEPROM.put(1050, kiv);
    EEPROM.put(1055, kdv);
    //delay(10);
    //shag == 4
    EEPROM.put(1060, kpv);
    EEPROM.put(1065, kiv);
    EEPROM.put(1070, kdv);
    //delay(10);
    //shag == 5
    EEPROM.put(1075, kpv);
    EEPROM.put(1080, kiv);
    EEPROM.put(1085, kdv);
    //delay(10);
    //shag == 6
    EEPROM.put(1090, kpv);
    EEPROM.put(1095, kiv);
    EEPROM.put(1100, kdv);
    //delay(10);
    //shag == 7
    EEPROM.put(1105, kpv);
    EEPROM.put(1110, kiv);
    EEPROM.put(1115, kdv);
    //delay(10);
    //shag == 8
    EEPROM.put(1120, kpv);
    EEPROM.put(1125, kiv);
    EEPROM.put(1130, kdv);
    //delay(10);
    //shag == 9
    EEPROM.put(1135, kpv);
    EEPROM.put(1140, kiv);
    EEPROM.put(1145, kdv);
    //delay(10);
    //shag == 10
    EEPROM.put(1150, kpv);
    EEPROM.put(1155, kiv);
    EEPROM.put(1160, kdv);
    //delay(10);
    //shag == 2
    EEPROM.put(1165, kpn);
    EEPROM.put(1170, kin);
    EEPROM.put(1175, kdn);
    //delay(10);
    //shag == 3
    EEPROM.put(1180, kpn);
    EEPROM.put(1185, kin);
    EEPROM.put(1190, kdn);
    //delay(10);
    //shag == 4
    EEPROM.put(1195, kpn);
    EEPROM.put(1200, kin);
    EEPROM.put(1205, kdn);
    //delay(10);
    //shag == 5
    EEPROM.put(1210, kpn);
    EEPROM.put(1215, kin);
    EEPROM.put(1220, kdn);
    //delay(10);
    //shag == 6
    EEPROM.put(1225, kpn);
    EEPROM.put(1230, kin);
    EEPROM.put(1235, kdn);
    //delay(10);
    //shag == 7
    EEPROM.put(1240, kpn);
    EEPROM.put(1245, kin);
    EEPROM.put(1250, kdn);
    //delay(10);
    //shag == 8
    EEPROM.put(1255, kpn);
    EEPROM.put(1260, kin);
    EEPROM.put(1265, kdn);
    //delay(10);
    //shag == 9
    EEPROM.put(1270, kpn);
    EEPROM.put(1275, kin);
    EEPROM.put(1280, kdn);
    //delay(10);
    //shag == 10
    EEPROM.put(1285, kpn);
    EEPROM.put(1290, kin);
    EEPROM.put(1295, kdn);

    //термопрофиль User 2 шаг 0
    EEPROM.put(1300, sec);
    EEPROM.put(1305, temp1);
    EEPROM.put(1310, temp2);
    //термопрофиль User 2 шаг 1
    EEPROM.put(1315, sec);
    EEPROM.put(1320, temp1);
    EEPROM.put(1325, temp2);
    //термопрофиль User 2 шаг 2
    EEPROM.put(1330, sec);
    EEPROM.put(1335, temp1);
    EEPROM.put(1340, temp2);
    //термопрофиль User 2 шаг 3
    EEPROM.put(1345, sec);
    EEPROM.put(1350, temp1);
    EEPROM.put(1355, temp2);
    //термопрофиль User 2 шаг 4
    EEPROM.put(1360, sec);
    EEPROM.put(1365, temp1);
    EEPROM.put(1370, temp2);
    //термопрофиль User 2 шаг 5
    EEPROM.put(1375, sec);
    EEPROM.put(1380, temp1);
    EEPROM.put(1385, temp2);
    //термопрофиль User 2 шаг 6
    EEPROM.put(1390, sec);
    EEPROM.put(1395, temp1);
    EEPROM.put(1400, temp2);
    //термопрофиль User 2 шаг 7
    EEPROM.put(1405, sec);
    EEPROM.put(1410, temp1);
    EEPROM.put(1415, temp2);
    //термопрофиль User 2 шаг 8
    EEPROM.put(1420, sec);
    EEPROM.put(1425, temp1);
    EEPROM.put(1430, temp2);
    //термопрофиль User 2 шаг 9
    EEPROM.put(1435, sec);
    EEPROM.put(1440, temp1);
    EEPROM.put(1445, temp2);
    //термопрофиль User 2 шаг 10
    EEPROM.put(1450, sec);
    EEPROM.put(1455, temp1);
    EEPROM.put(1460, temp2);
	
	//термопрофиль User 2
	//delay(10);
    EEPROM.put(1465, dtv); 
    EEPROM.put(1470, dtn); 
	//shag == 1 
    EEPROM.put(1475, kpv);
    EEPROM.put(1480, kiv);
    EEPROM.put(1485, kdv);
	//shag == 2 
    EEPROM.put(1490, kpv);
    EEPROM.put(1495, kiv);
    EEPROM.put(1500, kdv);
    //delay(10);
    //shag == 3
    EEPROM.put(1505, kpv);
    EEPROM.put(1510, kiv);
    EEPROM.put(1515, kdv);
    //delay(10);
    //shag == 4
    EEPROM.put(1520, kpv);
    EEPROM.put(1525, kiv);
    EEPROM.put(1530, kdv);
    //delay(10);
    //shag == 5
    EEPROM.put(1535, kpv);
    EEPROM.put(1540, kiv);
    EEPROM.put(1545, kdv);
    //delay(10);
    //shag == 6
    EEPROM.put(1550, kpv);
    EEPROM.put(1555, kiv);
    EEPROM.put(1560, kdv);
    //delay(10);
    //shag == 7
    EEPROM.put(1565, kpv);
    EEPROM.put(1570, kiv);
    EEPROM.put(1575, kdv);
    //delay(10);
    //shag == 8
    EEPROM.put(1580, kpv);
    EEPROM.put(1585, kiv);
    EEPROM.put(1590, kdv);
    //delay(10);
    //shag == 9
    EEPROM.put(1595, kpv);
    EEPROM.put(1600, kiv);
    EEPROM.put(1605, kdv);
    //delay(10);
    //shag == 10
    EEPROM.put(1610, kpv);
    EEPROM.put(1615, kiv);
    EEPROM.put(1620, kdv);
    //delay(10);
	//shag == 1
    EEPROM.put(1625, kpn);
    EEPROM.put(1630, kin);
    EEPROM.put(1635, kdn);
    //shag == 2
    EEPROM.put(1640, kpn);
    EEPROM.put(1645, kin);
    EEPROM.put(1650, kdn);
    //delay(10);
    //shag == 3
    EEPROM.put(1655, kpn);
    EEPROM.put(1660, kin);
    EEPROM.put(1665, kdn);
    //delay(10);
    //shag == 4
    EEPROM.put(1670, kpn);
    EEPROM.put(1675, kin);
    EEPROM.put(1680, kdn);
    //delay(10);
    //shag == 5
    EEPROM.put(1685, kpn);
    EEPROM.put(1690, kin);
    EEPROM.put(1695, kdn);
    //delay(10);
    //shag == 6
    EEPROM.put(1700, kpn);
    EEPROM.put(1705, kin);
    EEPROM.put(1710, kdn);
    //delay(10);
    //shag == 7
    EEPROM.put(1715, kpn);
    EEPROM.put(1720, kin);
    EEPROM.put(1725, kdn);
    //delay(10);
    //shag == 8
    EEPROM.put(1730, kpn);
    EEPROM.put(1735, kin);
    EEPROM.put(1740, kdn);
    //delay(10);
    //shag == 9
    EEPROM.put(1745, kpn);
    EEPROM.put(1750, kin);
    EEPROM.put(1755, kdn);
    //delay(10);
    //shag == 10
    EEPROM.put(1760, kpn);
    EEPROM.put(1765, kin);
    EEPROM.put(1770, kdn);

    	//термопрофиль User 3 шаг 0
    EEPROM.put(1775, sec);
    EEPROM.put(1780, temp1);
    EEPROM.put(1785, temp2);
    //термопрофиль User 3 шаг 1
    EEPROM.put(1790, sec);
    EEPROM.put(1795, temp1);
    EEPROM.put(1800, temp2);
    //термопрофиль User 3 шаг 2
    EEPROM.put(1805, sec);
    EEPROM.put(1810, temp1);
    EEPROM.put(1815, temp2);
    //термопрофиль User 3 шаг 3
    EEPROM.put(1820, sec);
    EEPROM.put(1825, temp1);
    EEPROM.put(1830, temp2);
    //термопрофиль User 3 шаг 4
    EEPROM.put(1835, sec);
    EEPROM.put(1840, temp1);
    EEPROM.put(1845, temp2);
    //термопрофиль User 3 шаг 5
    EEPROM.put(1850, sec);
    EEPROM.put(1855, temp1);
    EEPROM.put(1860, temp2);
    //термопрофиль User 3 шаг 6
    EEPROM.put(1865, sec);
    EEPROM.put(1870, temp1);
    EEPROM.put(1875, temp2);
    //термопрофиль User 3 шаг 7
    EEPROM.put(1880, sec);
    EEPROM.put(1885, temp1);
    EEPROM.put(1890, temp2);
    //термопрофиль User 3 шаг 8
    EEPROM.put(1895, sec);
    EEPROM.put(1900, temp1);
    EEPROM.put(1905, temp2);
    //термопрофиль User 3 шаг 9
    EEPROM.put(1910, sec);
    EEPROM.put(1915, temp1);
    EEPROM.put(1920, temp2);
    //термопрофиль User 3 шаг 10
    EEPROM.put(1925, sec);
    EEPROM.put(1930, temp1);
    EEPROM.put(1935, temp2);
	
	//термопрофиль User 3
	//delay(10);
    EEPROM.put(1940, dtv); 
    EEPROM.put(1945, dtn); 
	//shag == 1 
    EEPROM.put(1950, kpv);
    EEPROM.put(1955, kiv);
    EEPROM.put(1960, kdv);
	//shag == 2 
    EEPROM.put(1965, kpv);
    EEPROM.put(1970, kiv);
    EEPROM.put(1975, kdv);
    //delay(10);
    //shag == 3
    EEPROM.put(1980, kpv);
    EEPROM.put(1985, kiv);
    EEPROM.put(1990, kdv);
    //delay(10);
    //shag == 4
    EEPROM.put(1995, kpv);
    EEPROM.put(2000, kiv);
    EEPROM.put(2005, kdv);
    //delay(10);
    //shag == 5
    EEPROM.put(2010, kpv);
    EEPROM.put(2015, kiv);
    EEPROM.put(2020, kdv);
    //delay(10);
    //shag == 6
    EEPROM.put(2025, kpv);
    EEPROM.put(2030, kiv);
    EEPROM.put(2035, kdv);
    //delay(10);
    //shag == 7
    EEPROM.put(2040, kpv);
    EEPROM.put(2045, kiv);
    EEPROM.put(2050, kdv);
    //delay(10);
    //shag == 8
    EEPROM.put(2055, kpv);
    EEPROM.put(2060, kiv);
    EEPROM.put(2065, kdv);
    //delay(10);
    //shag == 9
    EEPROM.put(2070, kpv);
    EEPROM.put(2075, kiv);
    EEPROM.put(2080, kdv);
    //delay(10);
    //shag == 10
    EEPROM.put(2085, kpv);
    EEPROM.put(2090, kiv);
    EEPROM.put(2095, kdv);
	
    //delay(10);
	//shag == 1
    EEPROM.put(2100, kpn);
    EEPROM.put(2105, kin);
    EEPROM.put(2110, kdn);
    //shag == 2
    EEPROM.put(2115, kpn);
    EEPROM.put(2120, kin);
    EEPROM.put(2125, kdn);
    //delay(10);
    //shag == 3
    EEPROM.put(2130, kpn);
    EEPROM.put(2135, kin);
    EEPROM.put(2140, kdn);
    //delay(10);
    //shag == 4
    EEPROM.put(2145, kpn);
    EEPROM.put(2150, kin);
    EEPROM.put(2155, kdn);
    //delay(10);
    //shag == 5
    EEPROM.put(2160, kpn);
    EEPROM.put(2165, kin);
    EEPROM.put(2170, kdn);
    //delay(10);
    //shag == 6
    EEPROM.put(2175, kpn);
    EEPROM.put(2180, kin);
    EEPROM.put(2185, kdn);
    //delay(10);
    //shag == 7
    EEPROM.put(2190, kpn);
    EEPROM.put(2195, kin);
    EEPROM.put(2200, kdn);
    //delay(10);
    //shag == 8
    EEPROM.put(2205, kpn);
    EEPROM.put(2210, kin);
    EEPROM.put(2215, kdn);
    //delay(10);
    //shag == 9
    EEPROM.put(2220, kpn);
    EEPROM.put(2225, kin);
    EEPROM.put(2230, kdn);
    //delay(10);
    //shag == 10
    EEPROM.put(2235, kpn);
    EEPROM.put(2240, kin);
    EEPROM.put(2245, kdn);

    	//термопрофиль User 4 шаг 0
    EEPROM.put(2250, sec);
    EEPROM.put(2255, temp1);
    EEPROM.put(2260, temp2);
    //термопрофиль User 4 шаг 1
    EEPROM.put(2265, sec);
    EEPROM.put(2270, temp1);
    EEPROM.put(2275, temp2);
    //термопрофиль User 4 шаг 2
    EEPROM.put(2280, sec);
    EEPROM.put(2285, temp1);
    EEPROM.put(2290, temp2);
    //термопрофиль User 4 шаг 3
    EEPROM.put(2295, sec);
    EEPROM.put(2300, temp1);
    EEPROM.put(2305, temp2);
    //термопрофиль User 4 шаг 4
    EEPROM.put(2310, sec);
    EEPROM.put(2315, temp1);
    EEPROM.put(2320, temp2);
    //термопрофиль User 4 шаг 5
    EEPROM.put(2325, sec);
    EEPROM.put(2330, temp1);
    EEPROM.put(2335, temp2);
    //термопрофиль User 4 шаг 6
    EEPROM.put(2340, sec);
    EEPROM.put(2345, temp1);
    EEPROM.put(2350, temp2);
    //термопрофиль User 4 шаг 7
    EEPROM.put(2355, sec);
    EEPROM.put(2360, temp1);
    EEPROM.put(2365, temp2);
    //термопрофиль User 4 шаг 8
    EEPROM.put(2370, sec);
    EEPROM.put(2375, temp1);
    EEPROM.put(2380, temp2);
    //термопрофиль User 4 шаг 9
    EEPROM.put(2385, sec);
    EEPROM.put(2390, temp1);
    EEPROM.put(2395, temp2);
    //термопрофиль User 4 шаг 10
    EEPROM.put(2400, sec);
    EEPROM.put(2405, temp1);
    EEPROM.put(2410, temp2);
	
	//термопрофиль User 4
	//delay(10);
    EEPROM.put(2415, dtv); 
    EEPROM.put(2420, dtn); 
	//shag == 1 
    EEPROM.put(2425, kpv);
    EEPROM.put(2430, kiv);
    EEPROM.put(2435, kdv);
	//shag == 2 
    EEPROM.put(2440, kpv);
    EEPROM.put(2445, kiv);
    EEPROM.put(2450, kdv);
    //delay(10);
    //shag == 3
    EEPROM.put(2455, kpv);
    EEPROM.put(2460, kiv);
    EEPROM.put(2465, kdv);
    //delay(10);
    //shag == 4
    EEPROM.put(2470, kpv);
    EEPROM.put(2475, kiv);
    EEPROM.put(2480, kdv);
    //delay(10);
    //shag == 5
    EEPROM.put(2485, kpv);
    EEPROM.put(2490, kiv);
    EEPROM.put(2495, kdv);
    //delay(10);
    //shag == 6
    EEPROM.put(2500, kpv);
    EEPROM.put(2505, kiv);
    EEPROM.put(2510, kdv);
    //delay(10);
    //shag == 7
    EEPROM.put(2515, kpv);
    EEPROM.put(2520, kiv);
    EEPROM.put(2525, kdv);
    //delay(10);
    //shag == 8
    EEPROM.put(2530, kpv);
    EEPROM.put(2535, kiv);
    EEPROM.put(2540, kdv);
    //delay(10);
    //shag == 9
    EEPROM.put(2545, kpv);
    EEPROM.put(2550, kiv);
    EEPROM.put(2555, kdv);
    //delay(10);
    //shag == 10
    EEPROM.put(2560, kpv);
    EEPROM.put(2565, kiv);
    EEPROM.put(2570, kdv);
	
    //delay(10);
	//shag == 1
    EEPROM.put(2575, kpn);
    EEPROM.put(2580, kin);
    EEPROM.put(2585, kdn);
    //shag == 2
    EEPROM.put(2590, kpn);
    EEPROM.put(2595, kin);
    EEPROM.put(2600, kdn);
    //delay(10);
    //shag == 3
    EEPROM.put(2605, kpn);
    EEPROM.put(2610, kin);
    EEPROM.put(2615, kdn);
    //delay(10);
    //shag == 4
    EEPROM.put(2620, kpn);
    EEPROM.put(2625, kin);
    EEPROM.put(2630, kdn);
    //delay(10);
    //shag == 5
    EEPROM.put(2635, kpn);
    EEPROM.put(2640, kin);
    EEPROM.put(2645, kdn);
    //delay(10);
    //shag == 6
    EEPROM.put(2650, kpn);
    EEPROM.put(2655, kin);
    EEPROM.put(2660, kdn);
    //delay(10);
    //shag == 7
    EEPROM.put(2665, kpn);
    EEPROM.put(2670, kin);
    EEPROM.put(2675, kdn);
    //delay(10);
    //shag == 8
    EEPROM.put(2680, kpn);
    EEPROM.put(2685, kin);
    EEPROM.put(2690, kdn);
    //delay(10);
    //shag == 9
    EEPROM.put(2695, kpn);
    EEPROM.put(2700, kin);
    EEPROM.put(2705, kdn);
    //delay(10);
    //shag == 10
    EEPROM.put(2710, kpn);
    EEPROM.put(2715, kin);
    EEPROM.put(2720, kdn);

    //термопрофиль User 5 шаг 0
    EEPROM.put(2725, sec);
    EEPROM.put(2730, temp1);
    EEPROM.put(2735, temp2);
    //термопрофиль User 5 шаг 1
    EEPROM.put(2740, sec);
    EEPROM.put(2745, temp1);
    EEPROM.put(2750, temp2);
    //термопрофиль User 5 шаг 2
    EEPROM.put(2755, sec);
    EEPROM.put(2760, temp1);
    EEPROM.put(2765, temp2);
    //термопрофиль User 5 шаг 3
    EEPROM.put(2770, sec);
    EEPROM.put(2775, temp1);
    EEPROM.put(2780, temp2);
    //термопрофиль User 5 шаг 4
    EEPROM.put(2785, sec);
    EEPROM.put(2790, temp1);
    EEPROM.put(2795, temp2);
    //термопрофиль User 5 шаг 5
    EEPROM.put(2800, sec);
    EEPROM.put(2805, temp1);
    EEPROM.put(2810, temp2);
    //термопрофиль User 5 шаг 6
    EEPROM.put(2815, sec);
    EEPROM.put(2820, temp1);
    EEPROM.put(2825, temp2);
    //термопрофиль User 5 шаг 7
    EEPROM.put(2830, sec);
    EEPROM.put(2835, temp1);
    EEPROM.put(2840, temp2);
    //термопрофиль User 5 шаг 8
    EEPROM.put(2845, sec);
    EEPROM.put(2850, temp1);
    EEPROM.put(2855, temp2);
    //термопрофиль User 5 шаг 9
    EEPROM.put(2860, sec);
    EEPROM.put(2865, temp1);
    EEPROM.put(2870, temp2);
    //термопрофиль User 5 шаг 10
    EEPROM.put(2875, sec);
    EEPROM.put(2880, temp1);
    EEPROM.put(2885, temp2);
	
	//термопрофиль User 5
	//delay(10);
    EEPROM.put(2890, dtv); 
    EEPROM.put(2895, dtn); 
	//shag == 1 
    EEPROM.put(2900, kpv);
    EEPROM.put(2905, kiv);
    EEPROM.put(2910, kdv);
	//shag == 2 
    EEPROM.put(2915, kpv);
    EEPROM.put(2920, kiv);
    EEPROM.put(2925, kdv);
    //delay(10);
    //shag == 3
    EEPROM.put(2930, kpv);
    EEPROM.put(2935, kiv);
    EEPROM.put(2940, kdv);
    //delay(10);
    //shag == 4
    EEPROM.put(2945, kpv);
    EEPROM.put(2950, kiv);
    EEPROM.put(2955, kdv);
    //delay(10);
    //shag == 5
    EEPROM.put(2960, kpv);
    EEPROM.put(2965, kiv);
    EEPROM.put(2970, kdv);
    //delay(10);
    //shag == 6
    EEPROM.put(2975, kpv);
    EEPROM.put(2980, kiv);
    EEPROM.put(2985, kdv);
    //delay(10);
    //shag == 7
    EEPROM.put(2990, kpv);
    EEPROM.put(2995, kiv);
    EEPROM.put(3000, kdv);
    //delay(10);
    //shag == 8
    EEPROM.put(3005, kpv);
    EEPROM.put(3010, kiv);
    EEPROM.put(3015, kdv);
    //delay(10);
    //shag == 9
    EEPROM.put(3020, kpv);
    EEPROM.put(3025, kiv);
    EEPROM.put(3030, kdv);
    //delay(10);
    //shag == 10
    EEPROM.put(3035, kpv);
    EEPROM.put(3040, kiv);
    EEPROM.put(3045, kdv);
	
    //термопрофиль User 5
    //delay(10);
	  //shag == 1
    EEPROM.put(3050, kpn);
    EEPROM.put(3055, kin);
    EEPROM.put(3060, kdn);
    //shag == 2
    EEPROM.put(3065, kpn);
    EEPROM.put(3070, kin);
    EEPROM.put(3075, kdn);
    //delay(10);
    //shag == 3
    EEPROM.put(3080, kpn);
    EEPROM.put(3085, kin);
    EEPROM.put(3090, kdn);
    //delay(10);
    //shag == 4
    EEPROM.put(3095, kpn);
    EEPROM.put(3100, kin);
    EEPROM.put(3105, kdn);
    //delay(10);
    //shag == 5
    EEPROM.put(3110, kpn);
    EEPROM.put(3115, kin);
    EEPROM.put(3120, kdn);
    //delay(10);
    //shag == 6
    EEPROM.put(3125, kpn);
    EEPROM.put(3130, kin);
    EEPROM.put(3135, kdn);
    //delay(10);
    //shag == 7
    EEPROM.put(3140, kpn);
    EEPROM.put(3145, kin);
    EEPROM.put(3150, kdn);
    //delay(10);
    //shag == 8
    EEPROM.put(3155, kpn);
    EEPROM.put(3160, kin);
    EEPROM.put(3165, kdn);
    //delay(10);
    //shag == 9
    EEPROM.put(3170, kpn);
    EEPROM.put(3175, kin);
    EEPROM.put(3180, kdn);
    //delay(10);
    //shag == 10
    EEPROM.put(3185, kpn);
    EEPROM.put(3190, kin);
    EEPROM.put(3195, kdn);

    // Градус в Секунду
    	//термопрофиль User 1 шаг 0
	EEPROM.put(3200, gradsecv);
	EEPROM.put(3205, gradsecn);
    //термопрофиль User 1 шаг 1
	EEPROM.put(3210, gradsecv);
	EEPROM.put(3215, gradsecn);
    //термопрофиль User 1 шаг 2
	EEPROM.put(3220, gradsecv);
	EEPROM.put(3225, gradsecn);
    //термопрофиль User 1 шаг 3
	EEPROM.put(3230, gradsecv);
	EEPROM.put(3235, gradsecn);
    //термопрофиль User 1 шаг 4
	EEPROM.put(3240, gradsecv);
	EEPROM.put(3245, gradsecn);
    //термопрофиль User 1 шаг 5
 	EEPROM.put(3250, gradsecv);
	EEPROM.put(3255, gradsecn);
    //термопрофиль User 1 шаг 6
	EEPROM.put(3260, gradsecv);
	EEPROM.put(3265, gradsecn); 
    //термопрофиль User 1 шаг 7
	EEPROM.put(3270, gradsecv);
	EEPROM.put(3275, gradsecn); 
    //термопрофиль User 1 шаг 8
 	EEPROM.put(3280, gradsecv);
	EEPROM.put(3285, gradsecn);
    //термопрофиль User 1 шаг 9
	EEPROM.put(3290, gradsecv);
	EEPROM.put(3295, gradsecn);
    //термопрофиль User 1 шаг 10
	EEPROM.put(3300, gradsecv);
	EEPROM.put(3305, gradsecn); 

    //термопрофиль User 2 шаг 0
	EEPROM.put(3310, gradsecv);
	EEPROM.put(3315, gradsecn);
    //термопрофиль User 2 шаг 1
	EEPROM.put(3320, gradsecv);
	EEPROM.put(3325, gradsecn);
    //термопрофиль User 2 шаг 2
	EEPROM.put(3330, gradsecv);
	EEPROM.put(3335, gradsecn);
    //термопрофиль User 2 шаг 3
	EEPROM.put(3340, gradsecv);
	EEPROM.put(3345, gradsecn);
    //термопрофиль User 2 шаг 4
	EEPROM.put(3350, gradsecv);
	EEPROM.put(3355, gradsecn);
    //термопрофиль User 2 шаг 5
 	EEPROM.put(3360, gradsecv);
	EEPROM.put(3365, gradsecn);
    //термопрофиль User 2 шаг 6
	EEPROM.put(3370, gradsecv);
	EEPROM.put(3375, gradsecn); 
    //термопрофиль User 2 шаг 7
	EEPROM.put(3380, gradsecv);
	EEPROM.put(3385, gradsecn); 
    //термопрофиль User 2 шаг 8
 	EEPROM.put(3390, gradsecv);
	EEPROM.put(3395, gradsecn);
    //термопрофиль User 2 шаг 9
	EEPROM.put(3400, gradsecv);
	EEPROM.put(3405, gradsecn);
    //термопрофиль User 2 шаг 10
	EEPROM.put(3410, gradsecv);
	EEPROM.put(3415, gradsecn);

    //термопрофиль User 3 шаг 0
	EEPROM.put(3420, gradsecv);
	EEPROM.put(3425, gradsecn);
    //термопрофиль User 3 шаг 1
	EEPROM.put(3430, gradsecv);
	EEPROM.put(3435, gradsecn);
    //термопрофиль User 3 шаг 2
	EEPROM.put(3440, gradsecv);
	EEPROM.put(3445, gradsecn);
    //термопрофиль User 3 шаг 3
	EEPROM.put(3450, gradsecv);
	EEPROM.put(3455, gradsecn);
    //термопрофиль User 3 шаг 4
	EEPROM.put(3460, gradsecv);
	EEPROM.put(3465, gradsecn);
    //термопрофиль User 3 шаг 5
 	EEPROM.put(3470, gradsecv);
	EEPROM.put(3475, gradsecn);
    //термопрофиль User 3 шаг 6
	EEPROM.put(3480, gradsecv);
	EEPROM.put(3485, gradsecn); 
    //термопрофиль User 3 шаг 7
	EEPROM.put(3490, gradsecv);
	EEPROM.put(3495, gradsecn); 
    //термопрофиль User 3 шаг 8
 	EEPROM.put(3500, gradsecv);
	EEPROM.put(3505, gradsecn);
    //термопрофиль User 3 шаг 9
	EEPROM.put(3510, gradsecv);
	EEPROM.put(3515, gradsecn);
    //термопрофиль User 3 шаг 10
	EEPROM.put(3520, gradsecv);
	EEPROM.put(3525, gradsecn);

    //термопрофиль User 4 шаг 0
	EEPROM.put(3530, gradsecv);
	EEPROM.put(3535, gradsecn);
    //термопрофиль User 4 шаг 1
	EEPROM.put(3540, gradsecv);
	EEPROM.put(3545, gradsecn);
    //термопрофиль User 4 шаг 2
	EEPROM.put(3550, gradsecv);
	EEPROM.put(3555, gradsecn);
    //термопрофиль User 4 шаг 3
	EEPROM.put(3560, gradsecv);
	EEPROM.put(3565, gradsecn);
    //термопрофиль User 4 шаг 4
	EEPROM.put(3570, gradsecv);
	EEPROM.put(3575, gradsecn);
    //термопрофиль User 4 шаг 5
 	EEPROM.put(3580, gradsecv);
	EEPROM.put(3585, gradsecn);
    //термопрофиль User 4 шаг 6
	EEPROM.put(3590, gradsecv);
	EEPROM.put(3595, gradsecn); 
    //термопрофиль User 4 шаг 7
	EEPROM.put(3600, gradsecv);
	EEPROM.put(3605, gradsecn); 
    //термопрофиль User 4 шаг 8
 	EEPROM.put(3610, gradsecv);
	EEPROM.put(3615, gradsecn);
    //термопрофиль User 4 шаг 9
	EEPROM.put(3620, gradsecv);
	EEPROM.put(3625, gradsecn);
    //термопрофиль User 4 шаг 10
	EEPROM.put(3630, gradsecv);
	EEPROM.put(3635, gradsecn);

      //термопрофиль User 5 шаг 0
	EEPROM.put(3640, gradsecv);
	EEPROM.put(3645, gradsecn);
    //термопрофиль User 5 шаг 1
	EEPROM.put(3650, gradsecv);
	EEPROM.put(3655, gradsecn);
    //термопрофиль User 5 шаг 2
	EEPROM.put(3660, gradsecv);
	EEPROM.put(3665, gradsecn);
    //термопрофиль User 5 шаг 3
	EEPROM.put(3670, gradsecv);
	EEPROM.put(3675, gradsecn);
    //термопрофиль User 5 шаг 4
	EEPROM.put(3680, gradsecv);
	EEPROM.put(3685, gradsecn);
    //термопрофиль User 5 шаг 5
 	EEPROM.put(3690, gradsecv);
	EEPROM.put(3695, gradsecn);
    //термопрофиль User 5 шаг 6
	EEPROM.put(3700, gradsecv);
	EEPROM.put(3705, gradsecn); 
    //термопрофиль User 5 шаг 7
	EEPROM.put(3710, gradsecv);
	EEPROM.put(3715, gradsecn); 
    //термопрофиль User 5 шаг 8
 	EEPROM.put(3720, gradsecv);
	EEPROM.put(3725, gradsecn);
    //термопрофиль User 5 шаг 9
	EEPROM.put(3730, gradsecv);
	EEPROM.put(3735, gradsecn);
    //термопрофиль User 5 шаг 10
	EEPROM.put(3740, gradsecv);
	EEPROM.put(3745, gradsecn);

  // Watt
	//термопрофиль User 1 шаг 0
	EEPROM.put(3746, pwmv);
	EEPROM.put(3747, pwmn);
    //термопрофиль User 1 шаг 1
	EEPROM.put(3748, pwmv);
	EEPROM.put(3749, pwmn);
    //термопрофиль User 1 шаг 2
	EEPROM.put(3750, pwmv);
	EEPROM.put(3751, pwmn);
    //термопрофиль User 1 шаг 3
	EEPROM.put(3752, pwmv);
	EEPROM.put(3753, pwmn);
    //термопрофиль User 1 шаг 4
	EEPROM.put(3754, pwmv);
	EEPROM.put(3755, pwmn);
    //термопрофиль User 1 шаг 5
 	EEPROM.put(3756, pwmv);
	EEPROM.put(3757, pwmn);
    //термопрофиль User 1 шаг 6
	EEPROM.put(3758, pwmv);
	EEPROM.put(3759, pwmn); 
    //термопрофиль User 1 шаг 7
	EEPROM.put(3760, pwmv);
	EEPROM.put(3761, pwmn); 
    //термопрофиль User 1 шаг 8
 	EEPROM.put(3762, pwmv);
	EEPROM.put(3763, pwmn);
    //термопрофиль User 1 шаг 9
	EEPROM.put(3764, pwmv);
	EEPROM.put(3765, pwmn);
    //термопрофиль User 1 шаг 10
	EEPROM.put(3766, pwmv);
	EEPROM.put(3767, pwmn);
  // Watt
  //термопрофиль User 2 шаг 0
	EEPROM.put(3768, pwmv);
	EEPROM.put(3769, pwmn);
    //термопрофиль User 2 шаг 1
	EEPROM.put(3770, pwmv);
	EEPROM.put(3771, pwmn);
    //термопрофиль User 2 шаг 2
	EEPROM.put(3772, pwmv);
	EEPROM.put(3773, pwmn);
    //термопрофиль User 2 шаг 3
	EEPROM.put(3774, pwmv);
	EEPROM.put(3775, pwmn);
    //термопрофиль User 2 шаг 4
	EEPROM.put(3776, pwmv);
	EEPROM.put(3777, pwmn);
    //термопрофиль User 2 шаг 5
 	EEPROM.put(3778, pwmv);
	EEPROM.put(3779, pwmn);
    //термопрофиль User 2 шаг 6
	EEPROM.put(3780, pwmv);
	EEPROM.put(3781, pwmn); 
    //термопрофиль User 2 шаг 7
	EEPROM.put(3782, pwmv);
	EEPROM.put(3783, pwmn); 
    //термопрофиль User 2 шаг 8
 	EEPROM.put(3784, pwmv);
	EEPROM.put(3785, pwmn);
    //термопрофиль User 2 шаг 9
	EEPROM.put(3786, pwmv);
	EEPROM.put(3787, pwmn);
    //термопрофиль User 2 шаг 10
	EEPROM.put(3788, pwmv);
	EEPROM.put(3789, pwmn);
  // Watt
  //термопрофиль User 3 шаг 0
	EEPROM.put(3790, pwmv);
	EEPROM.put(3791, pwmn);
    //термопрофиль User 3 шаг 1
	EEPROM.put(3792, pwmv);
	EEPROM.put(3793, pwmn);
    //термопрофиль User 3 шаг 2
	EEPROM.put(3794, pwmv);
	EEPROM.put(3795, pwmn);
    //термопрофиль User 3 шаг 3
	EEPROM.put(3796, pwmv);
	EEPROM.put(3797, pwmn);
    //термопрофиль User 3 шаг 4
	EEPROM.put(3798, pwmv);
	EEPROM.put(3799, pwmn);
    //термопрофиль User 3 шаг 5
 	EEPROM.put(3800, pwmv);
	EEPROM.put(3801, pwmn);
    //термопрофиль User 3 шаг 6
	EEPROM.put(3802, pwmv);
	EEPROM.put(3803, pwmn); 
    //термопрофиль User 3 шаг 7
	EEPROM.put(3804, pwmv);
	EEPROM.put(3805, pwmn); 
    //термопрофиль User 3 шаг 8
 	EEPROM.put(3806, pwmv);
	EEPROM.put(3807, pwmn);
    //термопрофиль User 3 шаг 9
	EEPROM.put(3808, pwmv);
	EEPROM.put(3809, pwmn);
    //термопрофиль User 3 шаг 10
	EEPROM.put(3810, pwmv);
	EEPROM.put(3811, pwmn);
	// Watt
	//термопрофиль User 4 шаг 0
	EEPROM.put(3812, pwmv);
	EEPROM.put(3813, pwmn);
    //термопрофиль User 4 шаг 1
	EEPROM.put(3814, pwmv);
	EEPROM.put(3815, pwmn);
    //термопрофиль User 4 шаг 2
	EEPROM.put(3816, pwmv);
	EEPROM.put(3817, pwmn);
    //термопрофиль User 4 шаг 3
	EEPROM.put(3818, pwmv);
	EEPROM.put(3819, pwmn);
    //термопрофиль User 4 шаг 4
	EEPROM.put(3820, pwmv);
	EEPROM.put(3821, pwmn);
    //термопрофиль User 4 шаг 5
 	EEPROM.put(3822, pwmv);
	EEPROM.put(3823, pwmn);
    //термопрофиль User 4 шаг 6
	EEPROM.put(3824, pwmv);
	EEPROM.put(3825, pwmn); 
    //термопрофиль User 4 шаг 7
	EEPROM.put(3826, pwmv);
	EEPROM.put(3827, pwmn); 
    //термопрофиль User 4 шаг 8
 	EEPROM.put(3828, pwmv);
	EEPROM.put(3829, pwmn);
    //термопрофиль User 4 шаг 9
	EEPROM.put(3830, pwmv);
	EEPROM.put(3831, pwmn);
    //термопрофиль User 4 шаг 10
	EEPROM.put(3832, pwmv);
	EEPROM.put(3833, pwmn);
	// Watt
	//термопрофиль User 5 шаг 0
	EEPROM.put(3834, pwmv);
	EEPROM.put(3835, pwmn);
    //термопрофиль User 5 шаг 1
	EEPROM.put(3836, pwmv);
	EEPROM.put(3837, pwmn);
    //термопрофиль User 5 шаг 2
	EEPROM.put(3838, pwmv);
	EEPROM.put(3839, pwmn);
    //термопрофиль User 5 шаг 3
	EEPROM.put(3840, pwmv);
	EEPROM.put(3841, pwmn);
    //термопрофиль User 5 шаг 4
	EEPROM.put(3842, pwmv);
	EEPROM.put(3843, pwmn);
    //термопрофиль User 5 шаг 5
 	EEPROM.put(3844, pwmv);
	EEPROM.put(3845, pwmn);
    //термопрофиль User 5 шаг 6
	EEPROM.put(3846, pwmv);
	EEPROM.put(3847, pwmn); 
    //термопрофиль User 5 шаг 7
	EEPROM.put(3848, pwmv);
	EEPROM.put(3849, pwmn); 
    //термопрофиль User 5 шаг 8
 	EEPROM.put(3850, pwmv);
	EEPROM.put(3851, pwmn);
    //термопрофиль User 5 шаг 9
	EEPROM.put(3852, pwmv);
	EEPROM.put(3853, pwmn);
    //термопрофиль User 5 шаг 10
	EEPROM.put(3854, pwmv);
	EEPROM.put(3855, pwmn);

  // Оповещение температуры User 1
  EEPROM.put(3856,setopov1);
  EEPROM.put(3861,setopov2);
  EEPROM.put(3866,setopov3);
  EEPROM.put(3871,setopov4);
  // Оповещение температуры User 2
  EEPROM.put(3876,setopov1);
  EEPROM.put(3881,setopov2);
  EEPROM.put(3886,setopov3);
  EEPROM.put(3891,setopov4);
  // Оповещение температуры User 3
  EEPROM.put(3896,setopov1);
  EEPROM.put(3901,setopov2);
  EEPROM.put(3906,setopov3);
  EEPROM.put(3911,setopov4);
  // Оповещение температуры User 4
  EEPROM.put(3916,setopov1);
  EEPROM.put(3921,setopov2);
  EEPROM.put(3926,setopov3);
  EEPROM.put(3931,setopov4);
  // Оповещение температуры User 5
  EEPROM.put(3936,setopov1);
  EEPROM.put(3941,setopov2);
  EEPROM.put(3946,setopov3);
  EEPROM.put(3951,setopov4);

  EEPROM.put(3956,hup);
  EEPROM.put(3961,kup);
  EEPROM.put(3966,dtup);
  EEPROM.put(3971,hdown);
  EEPROM.put(3976,kdown);
  EEPROM.put(3981,dtdown);

    delay(20000);
    // TERMOPROFILY  заканчивается код
    EEPROM.write(INIT_ADDR, INIT_KEY);
  }

  EEPROM.get(197, coolervh);
  EEPROM.get(202, coolerp);
  EEPROM.get(237, comptempt1);
  EEPROM.get(242, comptempt2);
  EEPROM.get(247, znak1);
  EEPROM.get(252, znak2);


  if (EEPROM.get(207, termoprofily) == 0)
  {
         shag = 0;
         sec = 3;    
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
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
         
  } else if (EEPROM.get(207, termoprofily) == 1)
  {
         shag = 0;
         sec = 3;
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
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
         
  }else if (EEPROM.get(207, termoprofily) == 2)
  {
         EEPROM.get(227, dtv); 
         EEPROM.get(232, dtn); 
         EEPROM.get(22, sec);
         EEPROM.get(27, temp1);
         EEPROM.get(32, temp2);
             //термопрофиль User 1 шаг 0
          EEPROM.get(3200, gradsecv);
          EEPROM.get(3205, gradsecn);
          		  EEPROM.get(3956,hup);
		  EEPROM.get(3961,kup);
		  EEPROM.get(3966,dtup);
			
		  EEPROM.get(3971,hdown);
		  EEPROM.get(3976,kdown);
		  EEPROM.get(3981,dtdown);
          //термопрофиль User 1 шаг 0
          EEPROM.get(3746, pwmv);
          EEPROM.get(3747, pwmn);
           // Оповещение температуры User 1
          EEPROM.get(3856,setopov1);
          EEPROM.get(3861,setopov2);
          EEPROM.get(3866,setopov3);
          EEPROM.get(3871,setopov4);
         shag = 0;
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
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
         Dtv = dtv;
         String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
         SendData("t57.txt", t57);
         Dtn = dtn;
         String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
         SendData("t58.txt", t58); 
        GradSecv = gradsecv;
        String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
        SendData("tgcs3.txt", tgcs3);
        GradSecn = gradsecn;
        String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
        SendData("tgcs4.txt", tgcs4);
        Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
        pwmust2 = pwmn; 
  }else if (EEPROM.get(207, termoprofily) == 3)
  {
         EEPROM.get(1465, dtv); 
         EEPROM.get(1470, dtn); 
	       //термопрофиль User 2 шаг 0
         EEPROM.get(1300, sec);
         EEPROM.get(1305, temp1);
         EEPROM.get(1310, temp2);
         //термопрофиль User 2 шаг 0
          EEPROM.get(3310, gradsecv);
          EEPROM.get(3315, gradsecn);
          EEPROM.get(3956,hup);
		  EEPROM.get(3961,kup);
		  EEPROM.get(3966,dtup);
			
		  EEPROM.get(3971,hdown);
		  EEPROM.get(3976,kdown);
		  EEPROM.get(3981,dtdown);
          //термопрофиль User 2 шаг 0
          EEPROM.get(3768, pwmv);
          EEPROM.get(3769, pwmn);
          // Оповещение температуры User 2
          EEPROM.get(3876,setopov1);
          EEPROM.get(3881,setopov2);
          EEPROM.get(3886,setopov3);
          EEPROM.get(3891,setopov4);
         shag = 0;
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Dtv = dtv;
         String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
         SendData("t57.txt", t57);
         Dtn = dtn;
         String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
         SendData("t58.txt", t58);  
        GradSecv = gradsecv;
        String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
        SendData("tgcs3.txt", tgcs3);
        GradSecn = gradsecn;
        String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
        SendData("tgcs4.txt", tgcs4); 
        Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
        pwmust2 = pwmn;        
  }else if (EEPROM.get(207, termoprofily) == 4)
  {
         EEPROM.get(1940, dtv); 
         EEPROM.get(1945, dtn); 
         //термопрофиль User 3 шаг 0
         EEPROM.get(1775, sec);
         EEPROM.get(1780, temp1);
         EEPROM.get(1785, temp2);
             //термопрофиль User 3 шаг 0
          EEPROM.get(3420, gradsecv);
          EEPROM.get(3425, gradsecn);
          EEPROM.get(3956,hup);
		  EEPROM.get(3961,kup);
		  EEPROM.get(3966,dtup);
			
		  EEPROM.get(3971,hdown);
		  EEPROM.get(3976,kdown);
		  EEPROM.get(3981,dtdown);
          //термопрофиль User 3 шаг 0
          EEPROM.get(3790, pwmv);
          EEPROM.get(3791, pwmn);
          // Оповещение температуры User 3
          EEPROM.get(3896,setopov1);
          EEPROM.get(3901,setopov2);
          EEPROM.get(3906,setopov3);
          EEPROM.get(3911,setopov4);
         shag = 0;
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Dtv = dtv;
         String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
         SendData("t57.txt", t57);
         Dtn = dtn;
         String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
         SendData("t58.txt", t58);  
        GradSecv = gradsecv;
        String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
        SendData("tgcs3.txt", tgcs3);
        GradSecn = gradsecn;
        String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
        SendData("tgcs4.txt", tgcs4);
        Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
        pwmust2 = pwmn;         
  }else if (EEPROM.get(207, termoprofily) == 5)
  {
          EEPROM.get(2415, dtv); 
          EEPROM.get(2420, dtn);
        //термопрофиль User 4 шаг 0
          EEPROM.get(2250, sec);
          EEPROM.get(2255, temp1);
          EEPROM.get(2260, temp2);
            //термопрофиль User 4 шаг 0
          EEPROM.get(3530, gradsecv);
          EEPROM.get(3535, gradsecn);
		  EEPROM.get(3956,hup);
		  EEPROM.get(3961,kup);
		  EEPROM.get(3966,dtup);
			
		  EEPROM.get(3971,hdown);
		  EEPROM.get(3976,kdown);
		  EEPROM.get(3981,dtdown);          
          //термопрофиль User 4 шаг 0
          EEPROM.get(3812, pwmv);
          EEPROM.get(3813, pwmn); 
          // Оповещение температуры User 4
          EEPROM.get(3916,setopov1);
          EEPROM.get(3921,setopov2);
          EEPROM.get(3926,setopov3);
          EEPROM.get(3931,setopov4);         
         shag = 0;
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Dtv = dtv;
         String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
         SendData("t57.txt", t57);
         Dtn = dtn;
         String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
         SendData("t58.txt", t58);
        GradSecv = gradsecv;
        String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
        SendData("tgcs3.txt", tgcs3);
        GradSecn = gradsecn;
        String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
        SendData("tgcs4.txt", tgcs4); 
        Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
        pwmust2 = pwmn;          
  }else if (EEPROM.get(207, termoprofily) == 6)
  {
      //термопрофиль User 5
        //delay(10);
          EEPROM.get(2890, dtv); 
          EEPROM.get(2895, dtn); 
        //термопрофиль User 5 шаг 0
          EEPROM.get(2725, sec);
          EEPROM.get(2730, temp1);
          EEPROM.get(2735, temp2);
          //термопрофиль User 5 шаг 0
          EEPROM.get(3640, gradsecv);
          EEPROM.get(3645, gradsecn);
            EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
          //термопрофиль User 5 шаг 0
          EEPROM.get(3834, pwmv);
          EEPROM.get(3835, pwmn);
          // Оповещение температуры User 5
          EEPROM.get(3936,setopov1);
          EEPROM.get(3941,setopov2);
          EEPROM.get(3946,setopov3);
          EEPROM.get(3951,setopov4);
         shag = 0;
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Dtv = dtv;
         String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
         SendData("t57.txt", t57);
         Dtn = dtn;
         String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
         SendData("t58.txt", t58);
        GradSecv = gradsecv;
        String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
        SendData("tgcs3.txt", tgcs3);
        GradSecn = gradsecn;
        String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
        SendData("tgcs4.txt", tgcs4); 
                Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
        pwmust2 = pwmn;          
  }

  tempust1 = temp1;
  tempust2 = temp2;
  pwmust1 = pwmv;
  pwmust2 = pwmn;
  coolvust1 = coolervh;
  coolpust2 = coolerp;
  GradSecv = gradsecv;
  GradSecn = gradsecn;
  SetOpov1 = setopov1;
  SetOpov2 = setopov2;
  SetOpov3 = setopov3;
  SetOpov4 = setopov4;
  Kpv = kpv;
  Kpn = kpn;
  Kiv = kiv;
  Kin = kin;
  Kdv = kdv;
  Kdn = kdn;
  Dtv = dtv;
  Dtn = dtn;
  shag = 0;
  Hup    = hup;
  Kup    = kup;
  DTup   = dtup;
  Hdown  = hdown;
  Kdown  = kdown;
  DTdown = dtdown;


   // Настройки ПИД регулятора
  pid.setDirection(NORMAL); // направление регулирования (NORMAL/REVERSE). ПО УМОЛЧАНИЮ СТОИТ NORMAL
  pid.setMode(ON_ERROR); // режим: работа по входной ошибке ON_ERROR (0) или по изменению ON_RATE (1)
  pid.setLimits(0, 255);    // пределы (ставим для 8 битного ШИМ). ПО УМОЛЧАНИЮ СТОЯТ 0 И 255
  pid.setpoint = tempust1;        // сообщаем регулятору температуру, которую он должен поддерживать
  // в процессе работы можно менять коэффициенты
  pid.Kp = Kpv;
  pid.Ki = Kiv;
  pid.Kd = Kdv;
  pid.integral = 0.0;
  pid.window = GradSecv;
     

  pid2.setDirection(NORMAL); // направление регулирования (NORMAL/REVERSE). ПО УМОЛЧАНИЮ СТОИТ NORMAL
  pid2.setMode(ON_ERROR); // режим: работа по входной ошибке ON_ERROR (0) или по изменению ON_RATE (1)
  pid2.setLimits(0, 255);    // пределы (ставим для 8 битного ШИМ). ПО УМОЛЧАНИЮ СТОЯТ 0 И 255
  pid2.setpoint = tempust2;        // сообщаем регулятору температуру, которую он должен поддерживать
  // в процессе работы можно менять коэффициенты
  pid2.Kp = Kpn;
  pid2.Ki = Kin;
  pid2.Kd = Kdn;
  pid2.integral = 0.0;
  pid2.window = GradSecn;
  
  // направление, начальный сигнал, конечный, период плато, точность, время стабилизации, период итерации
  //tunerv.setParameters(NORMAL, 225, 235, 80000, 2.00, 100);
  //tunern.setParameters(NORMAL, 0, 50, 80000, 1.00, 100);

}



void loop(void) 
{
  /**
  if (compSerial.available()) 
  {
    char inc;
    inc = compSerial.read();
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
  **/
  if (nexSerial.available()) 
  {
    char inc;
    inc = nexSerial.read();
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
  if (nex2Serial.available()) 
  {
    char inc;
    inc = nex2Serial.read();
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


  if ((incStr.indexOf("00"))>=0)
  { // когда находимся на странице 0 обновляем компоненты
      EEPROM.get(247, znak1);
      EEPROM.get(252, znak2);      
      temp = 1;
      outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
  } else if((incStr.indexOf("01"))>=0)
  {
    temp = 0;
    
  } else if((incStr.indexOf("02"))>=0)
  {
      temp = 0;
  } else if((incStr.indexOf("03"))>=0)
  {
      temp = 0;
  } else if((incStr.indexOf("04"))>=0) // когда находимся на странице 4 обновляем компоненты
  {
      temp = 0;
      
      outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
      String t13= "\"" + String(profily) + "\"";  // Отображение 
      SendData("t13.txt", t13);
      outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
      outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
      outNumber("main.shagt.val", shag);  // Отображение числа в числовом компоненте shag
      outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
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
      outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
       String t13= "\"" + String(profily) + "\"";  // Отображение 
       SendData("t13.txt", t13);
      outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
      outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
      outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
  }else if((incStr.indexOf("06"))>=0)
  {
      temp = 0;
  }else if((incStr.indexOf("07"))>=0) // когда находимся на странице 7 обновляем компоненты
  { 
       temp = 0;
       outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
       String t13= "\"" + String(profily) + "\"";  // Отображение 
       SendData("t13.txt", t13);
       String t24 = "\"" + String(Kpv,5) + "\"";  // выводим пропорциональное
       SendData("t24.txt", t24);
       String t25 = "\"" + String(Kiv,5) + "\"";  // выводим интегральное
       SendData("t25.txt", t25);
       String t26= "\"" + String(Kdv,5) + "\"";  // выводим дефференциальное
       SendData("t26.txt", t26);
       String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
       SendData("t57.txt", t57);
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
  }else if((incStr.indexOf("08"))>=0) // когда находимся на странице 8 обновляем компоненты
  { 
       temp = 0;
       outNumber("coolervh.val", coolervh);  // Отображение числа в числовом компоненте coolervh
       outNumber("coolerp.val", coolerp);  // Отображение числа в числовом компоненте coolerp
  }else if((incStr.indexOf("09"))>=0) // когда находимся на странице 9 обновляем компоненты
  { 
       temp = 0;      
  }else if((incStr.indexOf("10"))>=0) // когда находимся на странице 10 обновляем компоненты
  { 
       temp = 0;
       outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
       String t13= "\"" + String(profily) + "\"";  // Отображение 
       SendData("t13.txt", t13);
       String t54 = "\"" + String(Kpn,5) + "\"";  // выводим пропорциональное
       SendData("t54.txt", t54);
       String t55 = "\"" + String(Kin,5) + "\"";  // выводим интегральное
       SendData("t55.txt", t55);
       String t56= "\"" + String(Kdn,5) + "\"";  // выводим дефференциальное
       SendData("t56.txt", t56);
       String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
       SendData("t58.txt", t58);
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
  }else if((incStr.indexOf("11"))>=0) // когда находимся на странице 10 обновляем компоненты
  { 
        EEPROM.get(247, znak1);
        EEPROM.get(252, znak2);
        temp = 0;
        String t61 = "\"" + String(znak1) + String(comptempt1) + "\"";  
        SendData("t61.txt", t61);  
        String t62 = "\"" + String(znak2) + String(comptempt2) + "\"";  
        SendData("t62.txt", t62);
        
  }else if((incStr.indexOf("12"))>=0) // когда находимся на странице 10 обновляем компоненты
  { 
    temp = 0;
    outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
    String t13= "\"" + String(profily) + "\"";  // Отображение 
    SendData("t13.txt", t13);
    String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
    SendData("tgcs3.txt", tgcs3);
    String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
    SendData("tgcs4.txt", tgcs4);
    outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
  }else if((incStr.indexOf("13"))>=0) // когда находимся на странице 10 обновляем компоненты
  { 
    temp = 0;
  }else if((incStr.indexOf("14"))>=0) // когда находимся на странице 10 обновляем компоненты
  { 
    temp = 0;
    outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
    String t13= "\"" + String(profily) + "\"";  // Отображение 
    SendData("t13.txt", t13);
    String setop1 = "\"" + String(setopov1) + "\"";  // Отображение kd
    SendData("setop1.txt", setop1);
    String setop2 = "\"" + String(setopov2) + "\"";  // Отображение kd
    SendData("setop2.txt", setop2);
    String setop3 = "\"" + String(setopov3) + "\"";  // Отображение kd
    SendData("setop3.txt", setop3);
    String setop4 = "\"" + String(setopov4) + "\"";  // Отображение kd
    SendData("setop4.txt", setop4);
  }else if((incStr.indexOf("15"))>=0) // когда находимся на странице 10 обновляем компоненты
  { 
    temp = 0;

  }else if((incStr.indexOf("16"))>=0) // когда находимся на странице 10 обновляем компоненты
  { 
    temp = 0;
    String thup2 = "\"" + String(hup) + "\"";  // Отображение 
    SendData("thup2.txt", thup2);
    String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
    SendData("tkup2.txt", tkup2);
    String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
    SendData("tdtup2.txt", tdtup2);

  }else if((incStr.indexOf("17"))>=0) // когда находимся на странице 10 обновляем компоненты
  { 
    temp = 0;
    String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
    SendData("thdown2.txt", thdown2);
    String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
    SendData("tkdown2.txt", tkdown2);
    String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
    SendData("tdtdown2.txt", tdtdown2);
  }

  if (temp==1)
  {
     if (!(nexSerial.available()))
     {   // если не принимаем и не передаем даные, то считываем температуры temp1 и temp2, в противном случае игнорируем temp1 и temp2
          if (tempt1 = (sens.readTemp()))
          {            // Читаем температуру
            // Если чтение прошло успешно - выводим в Serial
            tempt1 = (sens.getTemp());   // Забираем температуру через getTemp - вещественные числа (с float) с дробной частью, не забудьте объявить переменную temp1 нужного типа должна быть float а не int
            //temp1 = (sens.getTempInt());   // или getTempInt - целые числа (без float), можно объявить переменную temp1 типом int
            
            compensaciya_tempt1();
            //take1_averaged_reading();
            String t1 = "\"" + String(tempt1) + "'C\"";  // выводим температуру и градусы цельсия 
            SendData("t0.txt", t1);                     // на дисплей Nextion в компонент t0 с параметром txt
            //String t2 = "\"" + String(tempt1) + "'C\"";  // выводим температуру и градусы цельсия 
            //compSendData("t0.txt", t2); 
            
          } else 
          {
            String t1 = "\"" + String(tempt1) + "Error\"";  // если произошла ошибка выодим Error
            SendData("t0.txt", t1);
            //String t2 = "\"" + String(tempt1) + "Error\"";  // если произошла ошибка выодим Error
            //compSendData("t0.txt", t2);
            if(bt0==1) // Кнопка bt0 равна 1
            {
              page_main();
              bt0_click();
              bt0 = 0;
              reley_n=0;
              Timer2.disableISR();
              shag = 0;
              termoprofily1_9 = 0;
              termoprofily10 = 0;
              
              if (termoprofily == 0)
                {
                  shag = 0;
                  sec=3;
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
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
                } else if (termoprofily == 1)
                {
                  shag = 0;      
                  sec=3;
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag                  
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
                } else if (termoprofily == 2)
                {
                  EEPROM.get(22, sec);
                  EEPROM.get(27, temp1);
                  EEPROM.get(32, temp2);
                      //термопрофиль User 1 шаг 0
                  EEPROM.get(3200, gradsecv);
                  EEPROM.get(3205, gradsecn);
                    EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
                   //термопрофиль User 1 шаг 0
                  EEPROM.get(3746, pwmv);
                  EEPROM.get(3747, pwmn);  
                    // Оповещение температуры User 1
                  EEPROM.get(3856,setopov1);
                  EEPROM.get(3861,setopov2);
                  EEPROM.get(3866,setopov3);
                  EEPROM.get(3871,setopov4);            
                  shag = 0;
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
                  profily="User 1";
                  String t13= "\"" + String(profily) + "\"";  // Отображение 
                  SendData("t13.txt", t13);
                  // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
                  outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  tempust1 = temp1;
                  // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
                  outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
                  tempust2 = temp2;
                  outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
                }else if (termoprofily == 3)
                {
                  //термопрофиль User 2 шаг 0
                  EEPROM.get(1300, sec);
                  EEPROM.get(1305, temp1);
                  EEPROM.get(1310, temp2);
                  //термопрофиль User 2 шаг 0
                  EEPROM.get(3310, gradsecv);
                  EEPROM.get(3315, gradsecn);
                    EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
                  //термопрофиль User 2 шаг 0
                  EEPROM.get(3768, pwmv);
                  EEPROM.get(3769, pwmn);
                  // Оповещение температуры User 2
                  EEPROM.get(3876,setopov1);
                  EEPROM.get(3881,setopov2);
                  EEPROM.get(3886,setopov3);
                  EEPROM.get(3891,setopov4);
                  shag = 0;    
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag     
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
                  profily="User 2";
                  String t13= "\"" + String(profily) + "\"";  // Отображение 
                  SendData("t13.txt", t13);
                  // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
                  outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  tempust1 = temp1;
                   // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
                  outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
                  tempust2 = temp2;
                  outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
                }  else if (termoprofily == 4)
                {
                //термопрофиль User 3 шаг 0
                  EEPROM.get(1775, sec);
                  EEPROM.get(1780, temp1);
                  EEPROM.get(1785, temp2);
                    //термопрофиль User 3 шаг 0
                  EEPROM.get(3420, gradsecv);
                  EEPROM.get(3425, gradsecn);
                    EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
                  //термопрофиль User 3 шаг 0
                  EEPROM.get(3790, pwmv);
                  EEPROM.get(3791, pwmn); 
                  // Оповещение температуры User 3
                  EEPROM.get(3896,setopov1);
                  EEPROM.get(3901,setopov2);
                  EEPROM.get(3906,setopov3);
                  EEPROM.get(3911,setopov4);                 
                  shag = 0;    
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag     
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
                  profily="User 3";
                  String t13= "\"" + String(profily) + "\"";  // Отображение 
                  SendData("t13.txt", t13);
                  // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
                  outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  tempust1 = temp1;
                   // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
                  outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
                  tempust2 = temp2;
                  outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
                } else if (termoprofily == 5)
                {
                //термопрофиль User 4 шаг 0
                  EEPROM.get(2250, sec);
                  EEPROM.get(2255, temp1);
                  EEPROM.get(2260, temp2);
                    //термопрофиль User 4 шаг 0
                  EEPROM.get(3530, gradsecv);
                  EEPROM.get(3535, gradsecn);
                    EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
                  //термопрофиль User 4 шаг 0
                  EEPROM.get(3812, pwmv);
                  EEPROM.get(3813, pwmn); 
                  // Оповещение температуры User 4
                  EEPROM.get(3916,setopov1);
                  EEPROM.get(3921,setopov2);
                  EEPROM.get(3926,setopov3);
                  EEPROM.get(3931,setopov4);                 
                  shag = 0;    
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag     
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
                  profily="User 4";
                  String t13= "\"" + String(profily) + "\"";  // Отображение 
                  SendData("t13.txt", t13);
                  // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
                  outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  tempust1 = temp1;
                   // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
                  outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
                  tempust2 = temp2;
                  outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
                } else if (termoprofily == 6)
                {
                //термопрофиль User 5 шаг 0
                  EEPROM.get(2725, sec);
                  EEPROM.get(2730, temp1);
                  EEPROM.get(2735, temp2);
                  //термопрофиль User 5 шаг 0
                  EEPROM.get(3640, gradsecv);
                  EEPROM.get(3645, gradsecn);
                    EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
                  //термопрофиль User 5 шаг 0
                  EEPROM.get(3834, pwmv);
                  EEPROM.get(3835, pwmn);
                  // Оповещение температуры User 5
                  EEPROM.get(3936,setopov1);
                  EEPROM.get(3941,setopov2);
                  EEPROM.get(3946,setopov3);
                  EEPROM.get(3951,setopov4);
                  shag = 0;    
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag     
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
                  profily="User 5";
                  String t13= "\"" + String(profily) + "\"";  // Отображение 
                  SendData("t13.txt", t13);
                  // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
                  outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  tempust1 = temp1;
                   // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
                  outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
                  tempust2 = temp2;
                  outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
                }                                                    
            }
          }
          delay(1000);   // задержка в 1 секунду, так как нужно примерно 750 мс чтобы считать температуру успешно
    
          if (tempt2 = (sens2.readTemp()))
          {            // Читаем температуру
             // Если чтение прошло успешно - выводим в Serial
            tempt2 = (sens2.getTemp());   // Забираем температуру через getTemp - вещественные числа (с float)
            //temp2 = (sens2.getTempInt());   // или getTempInt - целые числа (без float)
            
            compensaciya_tempt2();
            
            //take2_averaged_reading();
            String t1 = "\"" + String(tempt2) + "'C\"";
            SendData("t1.txt", t1);
            //String t2 = "\"" + String(tempt2) + "'C\"";
            //compSendData("t1.txt", t2);
            
          } else 
          {
            String t1 = "\"" + String(tempt2) + "Error\"";  // если произошла ошибка выодим Error
            SendData("t1.txt", t1);
            //String t2 = "\"" + String(tempt2) + "Error\"";  // если произошла ошибка выодим Error
            //compSendData("t1.txt", t2);
            if(bt0==1) // Кнопка bt0 равна 1
            {
              page_main();
              bt0_click();
              bt0 = 0;
              reley_n=0;
              Timer2.disableISR();
              shag = 0;
              termoprofily1_9 = 0;
              termoprofily10 = 0;
              
              if (termoprofily == 0)
               {
                  shag = 0;
                  sec=3;
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag                  
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
                } else if (termoprofily == 1)
                {
                  shag = 0;      
                  sec=3;
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag                    
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
                } else if (termoprofily == 2)
                {
                  EEPROM.get(22, sec);
                  EEPROM.get(27, temp1);
                  EEPROM.get(32, temp2);
                      //термопрофиль User 1 шаг 0
                  EEPROM.get(3200, gradsecv);
                  EEPROM.get(3205, gradsecn);
                    EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
                  //термопрофиль User 1 шаг 0
                  EEPROM.get(3746, pwmv);
                  EEPROM.get(3747, pwmn);
                  // Оповещение температуры User 1
                  EEPROM.get(3856,setopov1);
                  EEPROM.get(3861,setopov2);
                  EEPROM.get(3866,setopov3);
                  EEPROM.get(3871,setopov4);
                  shag = 0;    
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag     
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
                  profily="User 1";
                  String t13= "\"" + String(profily) + "\"";  // Отображение 
                  SendData("t13.txt", t13);
                  // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
                  outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  tempust1 = temp1;
                   // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
                  outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
                  tempust2 = temp2;
                  outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
                } else if (termoprofily == 3)
                {
                  //термопрофиль User 2 шаг 0
                  EEPROM.get(1300, sec);
                  EEPROM.get(1305, temp1);
                  EEPROM.get(1310, temp2);
                      //термопрофиль User 2 шаг 0
                  EEPROM.get(3310, gradsecv);
                  EEPROM.get(3315, gradsecn);
                    EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
                  //термопрофиль User 2 шаг 0
                  EEPROM.get(3768, pwmv);
                  EEPROM.get(3769, pwmn);
                  // Оповещение температуры User 2
                  EEPROM.get(3876,setopov1);
                  EEPROM.get(3881,setopov2);
                  EEPROM.get(3886,setopov3);
                  EEPROM.get(3891,setopov4);
                  shag = 0;    
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag     
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
                  profily="User 2";
                  String t13= "\"" + String(profily) + "\"";  // Отображение 
                  SendData("t13.txt", t13);
                  // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
                  outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  tempust1 = temp1;
                   // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
                  outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
                  tempust2 = temp2;
                  outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
                } else if (termoprofily == 4)
                {
                //термопрофиль User 3 шаг 0
                  EEPROM.get(1775, sec);
                  EEPROM.get(1780, temp1);
                  EEPROM.get(1785, temp2);
                    //термопрофиль User 3 шаг 0
                  EEPROM.get(3420, gradsecv);
                  EEPROM.get(3425, gradsecn); 
                    EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
                  //термопрофиль User 3 шаг 0
                  EEPROM.get(3790, pwmv);
                  EEPROM.get(3791, pwmn);
                  // Оповещение температуры User 3
                  EEPROM.get(3896,setopov1);
                  EEPROM.get(3901,setopov2);
                  EEPROM.get(3906,setopov3);
                  EEPROM.get(3911,setopov4);                 
                  shag = 0;    
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag     
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
                  profily="User 3";
                  String t13= "\"" + String(profily) + "\"";  // Отображение 
                  SendData("t13.txt", t13);
                  // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
                  outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  tempust1 = temp1;
                   // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
                  outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
                  tempust2 = temp2;
                  outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
                } else if (termoprofily == 5)
                {
                //термопрофиль User 4 шаг 0
                  EEPROM.get(2250, sec);
                  EEPROM.get(2255, temp1);
                  EEPROM.get(2260, temp2);
                    //термопрофиль User 4 шаг 0
                  EEPROM.get(3530, gradsecv);
                  EEPROM.get(3535, gradsecn); 
                    EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
                  //термопрофиль User 4 шаг 0
                  EEPROM.get(3812, pwmv);
                  EEPROM.get(3813, pwmn);
                  // Оповещение температуры User 4
                  EEPROM.get(3916,setopov1);
                  EEPROM.get(3921,setopov2);
                  EEPROM.get(3926,setopov3);
                  EEPROM.get(3931,setopov4);                 
                  shag = 0;    
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag     
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
                  profily="User 4";
                  String t13= "\"" + String(profily) + "\"";  // Отображение 
                  SendData("t13.txt", t13);
                  // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
                  outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  tempust1 = temp1;
                   // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
                  outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
                  tempust2 = temp2;
                  outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
                }  else if (termoprofily == 6)
                {
                //термопрофиль User 5 шаг 0
                  EEPROM.get(2725, sec);
                  EEPROM.get(2730, temp1);
                  EEPROM.get(2735, temp2);
                  //термопрофиль User 5 шаг 0
                  EEPROM.get(3640, gradsecv);
                  EEPROM.get(3645, gradsecn);
                    EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
                  //термопрофиль User 5 шаг 0
                  EEPROM.get(3834, pwmv);
                  EEPROM.get(3835, pwmn);
                  // Оповещение температуры User 5
                  EEPROM.get(3936,setopov1);
                  EEPROM.get(3941,setopov2);
                  EEPROM.get(3946,setopov3);
                  EEPROM.get(3951,setopov4);
                  shag = 0;    
                  outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag     
                  outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
                  outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
                  profily="User 5";
                  String t13= "\"" + String(profily) + "\"";  // Отображение 
                  SendData("t13.txt", t13);
                  // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
                  outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  tempust1 = temp1;
                   // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
                  outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
                  tempust2 = temp2;
                  outNumber("n0_temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
                  outNumber("n1_temp2.val", temp2);  // Отображение числа в числовом компоненте temp2         
                }                                
            }
          }
          delay(1000); 
          
      }
  }

  if (!(nex2Serial.available()))
     {  
      /**if (tempt1 = (sens.readTemp()))
          {            // Читаем температуру
            // Если чтение прошло успешно - выводим в Serial
            tempt1 = (sens.getTemp());   // Забираем температуру через getTemp - вещественные числа (с float) с дробной частью, не забудьте объявить переменную temp1 нужного типа должна быть float а не int
            //temp1 = (sens.getTempInt());   // или getTempInt - целые числа (без float), можно объявить переменную temp1 типом int
         **/   
            gafick1();
          //}
        /**if (tempt2 = (sens.readTemp()))
          {            // Читаем температуру
            // Если чтение прошло успешно - выводим в Serial
            tempt2 = (sens.getTemp());   // Забираем температуру через getTemp - вещественные числа (с float) с дробной частью, не забудьте объявить переменную temp1 нужного типа должна быть float а не int
            //temp2 = (sens.getTempInt());   // или getTempInt - целые числа (без float), можно объявить переменную temp1 типом int
           **/ 
            gafick2();
          //}
     }

  // задаём значение
  //dimmer[0] = map(analogRead(A0), 0, 1023, 0, 9500);
  //dimmer[0] = 50;
  //dimmer[1] = 120;
  //delay(100); // в реальном коде задержек быть не должно

  // Настройки Гистерезиса
  if (hnup == 0)
  {
    regulator.setDirection(NORMALO);
  }else if (hnup == 1)
  {
    regulator.setDirection(REVERSEI);  
  }
  regulator.setpoint = tempust1;    // установка (температуры)
  regulator.hysteresis = Hup;   // ширина гистерезиса
  regulator.k = Kup;          // коэффициент обратной связи (подбирается по факту)
  regulator.dT = DTup;       // установить время итерации для getResultTimer
    
  if (hndown == 0)
  {
    regulator2.setDirection(NORMALO);
  }else if (hndown == 1)
  {
    regulator2.setDirection(REVERSEI); 
  }
  regulator2.setpoint = tempust2;    // установка (температуры)
  regulator2.hysteresis = Hdown;   // ширина гистерезиса
  regulator2.k = Kdown;          // коэффициент обратной связи (подбирается по факту)
  regulator2.dT = DTdown;       // установить время итерации для getResultTimer
   
  if (millis() >= myTimer0 + 1*100) 
  {   // таймер на 100 мс (1 раза в 0.100 сек) автор таймера https://alexgyver.ru/lessons/
      //myTimer0 = millis();
      if (ph==1)
      {
        if(reley_n==1)
        {
          Timer2.enableISR();
          if ((reley_n1==1) && (nigniynagrev == 1) && (tempust2 != 0))
          {
            pidCountrolN();// Пид регулирование
            myTimer0 = millis();
          }else
          {
            reley_n1=0;
            analogWrite(nigniy_1, 0);
            
            myTimer0 = millis();
          }
          if ((reley_v==1) && (verhniynagrev == 1) && (tempust1 != 0))
          {
            pidCountrolV(); // Пид регулирование          
            myTimer0 = millis();
          }else
          {
            reley_v=0;
            analogWrite(verhniy_1, 0);  
            
            myTimer0 = millis();
          }
          
        }
    
  }else {
        if(reley_n==1)
        {
          Timer2.enableISR();
          if ((reley_n1==1) && (nigniynagrev == 1) && (tempust2 != 0))
          {
            reguln();  // Гистерезис
            myTimer0 = millis(); 
          }else
          {
            reley_n1=0;
            analogWrite(nigniy_1, 0);
            
            myTimer0 = millis();
          }   


          if((reley_v==1) && (verhniynagrev == 1) && (tempust1 != 0))
          {
            regul();   // Гистерезис
            myTimer0 = millis();   
          }else
          {
            reley_v=0;
            analogWrite(verhniy_1, 0);  
            
            myTimer0 = millis();
          }
          
        }
      
      }
   
  }
    
  if (termoprofily1_9 == 1)
  { 
    if (millis() >= myTimer1 + sec*1000) 
    {   // таймер на 80000 мс (1 раза в 80 сек) автор таймера https://alexgyver.ru/lessons/
      //myTimer1 = millis();
      termoprofily0_1_9();
      termoprofily1_9 = 0;   // выполнить действие 1
      myTimer1 = millis();
    }
  }
  if (termoprofily10 == 1)
  { 
    if (millis() >= myTimer2 + sec*1000) 
    {   // таймер на 80000 мс (1 раза в 80 сек) автор таймера https://alexgyver.ru/lessons/
      //myTimer2 = millis();
      termoprofily_10();
      termoprofily10 = 0;   // выполнить действие 1
      myTimer2 = millis();
    }
  }
if (!(nexSerial.available()))
{
  if (termoprofily == 2)
  {
    if ((setopov1 != 0) && (setopov1 <= tempt1) && (setopov2 != 0) && (setopov2 >= tempt1))   //  это верхний нагреватель
    {  // пока находится в диапазоне 190-193 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();       
    }
    if ((setopov3 != 0) && (setopov3 <= tempt2) && (setopov4 != 0) && (setopov4 >= tempt2))   //  это верхний нагреватель
    {  // пока находится в диапазоне 140-143 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();
    }
  }else if (termoprofily == 3)
  {
    if ((setopov1 != 0) && (setopov1 <= tempt1) && (setopov2 != 0) && (setopov2 >= tempt1))   //  это верхний нагреватель
    {  // пока находится в диапазоне 190-193 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();       
    }
    if ((setopov3 != 0) && (setopov3 <= tempt2) && (setopov4 != 0) && (setopov4 >= tempt2))   //  это верхний нагреватель
    {  // пока находится в диапазоне 140-143 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();
    }
  }else if (termoprofily == 4)
  { 
    if ((setopov1 != 0) && (setopov1 <= tempt1) && (setopov2 != 0) && (setopov2 >= tempt1))   //  это верхний нагреватель
    {  // пока находится в диапазоне 190-193 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();       
    }
    if ((setopov3 != 0) && (setopov3 <= tempt2) && (setopov4 != 0) && (setopov4 >= tempt2))   //  это верхний нагреватель
    {  // пока находится в диапазоне 140-143 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();
    }
  }else if (termoprofily == 5)
  {
    if ((setopov1 != 0) && (setopov1 <= tempt1) && (setopov2 != 0) && (setopov2 >= tempt1))   //  это верхний нагреватель
    {  // пока находится в диапазоне 190-193 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();       
    }
    if ((setopov3 != 0) && (setopov3 <= tempt2) && (setopov4 != 0) && (setopov4 >= tempt2))   //  это верхний нагреватель
    {  // пока находится в диапазоне 140-143 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();
    }
  }else if (termoprofily == 6)
  {
    if ((setopov1 != 0) && (setopov1 <= tempt1) && (setopov2 != 0) && (setopov2 >= tempt1))   //  это верхний нагреватель
    {  // пока находится в диапазоне 190-193 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();       
    }
    if ((setopov3 != 0) && (setopov3 <= tempt2) && (setopov4 != 0) && (setopov4 >= tempt2))   //  это верхний нагреватель
    {  // пока находится в диапазоне 140-143 градуса пищим, если за диапазоном не пищим
      delay(10);
      sound_click();
    }
  }
}

  
  
  if ((tempust1 == 0) && (verhniynagrev == 0))
  {
    reley_v=0; analogWrite(verhniy_1, 0);
  } else if((tempust1 != 0) && (verhniynagrev == 1))
  {
    reley_v=1;   
  } else
  {
    reley_v=0; analogWrite(verhniy_1, 0);
  }



  if ((tempust2 == 0) && (nigniynagrev == 0))
  {
    reley_n1=0; analogWrite(nigniy_1, 0);
  } else if ((tempust2 != 0) && (nigniynagrev == 1))
  {
    reley_n1=1;
  } else
  {
    reley_n1=0; analogWrite(nigniy_1, 0);
  }
  


  if((reley_n==1) && (tempust1 == 0) && (tempust2 == 0))
  {
      termoprofily10 = 1;
      Timer2.disableISR();
      shag = 0;
      reley_n=0;
      reley_n1=0;
      reley_v=0;
      analogWrite(nigniy_1, 0);
      analogWrite(verhniy_1, 0);
  } 



}

// прерывание детектора нуля
void isr() 
{
  counter = 255;
  Timer2.restart();
}
// прерывание таймера
ISR(TIMER2_A) 
{
  for (byte i = 0; i < DIM_AMOUNT; i++) 
  {
    if (counter == dimmer[i]) digitalWrite(dimPins[i], 1);  // на текущем тике включаем
    else if (counter == dimmer[i] - 1) digitalWrite(dimPins[i], 0);  // на следующем выключаем
  }
  counter--;
}

void pidCountrolN()
{
   
  if (detect_zero == 1)
  {  // без детектора ноля                
    pid2.input = tempt2;   // сообщаем регулятору текущую температуру
    
    //tunern.setInput(tempt2);
    //tunern.compute();
    // getResultTimer возвращает значение для управляющего устройства
    // (после вызова можно получать это значение как regulator.output)
    // обновление происходит по встроенному таймеру на millis()
    
    if (tempt2 < tempust2)
    { 
      pid2.output = pwmust2; //правил здесь 
      // analogWrite(nigniy_1, tunern.getOutput());
      
      analogWrite(nigniy_1, pid2.getResult());  // отправляем на мосфет
      
      
      // .getResultTimer() по сути возвращает regulator.output 
      // выводит в порт текстовые отладочные данные, включая коэффициенты
      //tunern.debugText();

      // выводит в порт данные для построения графиков, без коэффициентов
      //tunern.debugPlot();
      /**
      if (tunern.getState() == 7) //и при наступлении этого условия получить коэффициенты:
      {
       //tuner.getPI_p(); //- p для ПИ регулятора
       //tuner.getPI_i(); //- i для ПИ регулятора
    
        kpn = tunern.getPID_p(); //- p для ПИД регулятора
        Kpn = kpn;
        kin = tunern.getPID_i(); //- i для ПИД регулятора
        Kin = kin;
        kdn = tunern.getPID_d(); //- d для ПИД регулятора	
        Kdn = kdn;
      }**/
    }else if(tempt2 >= tempust2)
    {
      analogWrite(nigniy_1, 0);
    }
  }else // С детектором ноля
  {          
    pid2.input = tempt2;   // сообщаем регулятору текущую температуру
    
    //tunern.setInput(tempt2);
    //tunern.compute();
    // getResultTimer возвращает значение для управляющего устройства
    // (после вызова можно получать это значение как regulator.output)
    // обновление происходит по встроенному таймеру на millis()
    
    if (tempt2 < tempust2) 
    {
      pid2.output = pwmust2; //правил здесь 
      // getResultTimer возвращает значение для управляющего устройства
      // (после вызова можно получать это значение как regulator.output)
      // обновление происходит по встроенному таймеру на millis()
      //analogWrite(nigniy_1, dimmer[0] = tunern.getOutput());  // отправляем на мосфет
      // .getResultTimer() по сути возвращает regulator.output 
      
      analogWrite(nigniy_1, dimmer[0] = pid2.getResult());  // отправляем на мосфет
         
     
      // .getResultTimer() по сути возвращает regulator.output 
      // выводит в порт текстовые отладочные данные, включая коэффициенты
      //tunern.debugText();

      // выводит в порт данные для построения графиков, без коэффициентов
      //tunern.debugPlot();
      /**
      if (tunern.getState() == 7) //и при наступлении этого условия получить коэффициенты:
      {
       //tunern.getPI_p(); //- p для ПИ регулятора
       //tunern.getPI_i(); //- i для ПИ регулятора
    
        kpn = tunern.getPID_p(); //- p для ПИД регулятора
        Kpn = kpn;
        kin = tunern.getPID_i(); //- i для ПИД регулятора
        Kin = kin;
        kdn = tunern.getPID_d(); //- d для ПИД регулятора	
        Kdn = kdn;
      }**/
    }else if(tempt2 >= tempust2) 
    {
      analogWrite(nigniy_1, 0);
    }

  }
        
  
}

void pidCountrolV()
{
  
  if (detect_zero == 1)
  { // без детектора ноля
   
    pid.input = tempt1;   // сообщаем регулятору текущую температуру
    
    //tunerv.setInput(tempt1);
    //tunerv.compute();
    // getResultTimer возвращает значение для управляющего устройства
    // (после вызова можно получать это значение как regulator.output)
    // обновление происходит по встроенному таймеру на millis()
  
    if (tempt1 < tempust1)
    {
      pid.output = pwmust1; //правил здесь
      //analogWrite(verhniy_1, tunerv.getOutput());
         
      
      analogWrite(verhniy_1, pid.getResult());  // отправляем на мосфет
      
     /** 
      // .getResultTimer() по сути возвращает regulator.output
      
      // выводит в порт текстовые отладочные данные, включая коэффициенты
      //tunerv.debugText();

      // выводит в порт данные для построения графиков, без коэффициентов
      //tunerv.debugPlot();
      /**
      if (tunerv.getState() == 7) //и при наступлении этого условия получить коэффициенты:
      {
       //tunerv.getPI_p(); //- p для ПИ регулятора
       //tunerv.getPI_i(); //- i для ПИ регулятора
    
        kpv = tunerv.getPID_p(); //- p для ПИД регулятора
        Kpv = kpv;
        kiv = tunerv.getPID_i(); //- i для ПИД регулятора
        Kiv = kiv;
        kdv = tunerv.getPID_d(); //- d для ПИД регулятора	
        Kdv = kdv;
      }**/
    }	else if(tempt1 >= tempust1)
    {
      analogWrite(verhniy_1, 0);
    }
  } else
  {
    //с детектором ноля
    pid.input = tempt1;   // сообщаем регулятору текущую температуру

    //tunerv.setInput(tempt1);
    //tunerv.compute();
    // getResultTimer возвращает значение для управляющего устройства
    // (после вызова можно получать это значение как regulator.output)
    // обновление происходит по встроенному таймеру на millis()
    
    if (tempt1 < tempust1)
    {
      pid.output = pwmust1; //правил здесь
      //analogWrite(verhniy_1, dimmer[1] = tunerv.getOutput());
      
      analogWrite(verhniy_1, dimmer[1] = pid.getResult());  // отправляем на мосфет
      
      // .getResultTimer() по сути возвращает regulator.output

      // выводит в порт текстовые отладочные данные, включая коэффициенты
      //tunerv.debugText();

      // выводит в порт данные для построения графиков, без коэффициентов
      //tunerv.debugPlot();
      /**
      if (tunerv.getState() == 7) //и при наступлении этого условия получить коэффициенты:
      {
       //tuner.getPI_p(); //- p для ПИ регулятора
       //tuner.getPI_i(); //- i для ПИ регулятора
    
        kpv = tunerv.getPID_p(); //- p для ПИД регулятора
        Kpv = kpv;
        kiv = tunerv.getPID_i(); //- i для ПИД регулятора
        Kiv = kiv;
        kdv = tunerv.getPID_d(); //- d для ПИД регулятора	
        Kdv = kdv;
      }**/

    
    }else if (tempt1 >= tempust1)
    {
      analogWrite(verhniy_1, 0);
    }
    
  }
}

void yield() {
  // а тут можно опрашивать кнопку
  // и не пропустить нажатия из за delay!
  
// читаем инвертированное значение для удобства
  btnState = !digitalRead(btn_start);
  if (btnState && !flag && millis() - btnTimer > 100) {
    flag = true;
    btnTimer = millis();
    //Serial.println("press");
    
  }
  if (btnState && flag && millis() - btnTimer >= 3000) {
    btnTimer = millis();
    //Serial.println("press hold");
    btn_start_click();
  }
  if (!btnState && flag && millis() - btnTimer > 3000) {
    flag = false;
    btnTimer = millis();
    //Serial.println("release");
  }

// читаем инвертированное значение для удобства
  btnState = !digitalRead(btn_stop);
  if (btnState && !flag && millis() - btnTimer > 100) {
    flag = true;
    btnTimer = millis();
    //Serial.println("press");

  }
  if (btnState && flag && millis() - btnTimer >= 3000) {
    btnTimer = millis();
    //Serial.println("press hold");
    btn_stop_click();
    termoprofily1_9 = 0;
    termoprofily10 = 0;
    Timer2.disableISR();
    shag = 0;
    reley_n=0;
    reley_n1=0;
    reley_v=0;
    analogWrite(nigniy_1, 0);
    analogWrite(verhniy_1, 0);
    
  }
  if (!btnState && flag && millis() - btnTimer > 3000) {
    flag = false;
    btnTimer = millis();
    //Serial.println("release");

  }
}


// этот код и остальной нужен
// компенсация термопары
void compensaciya_tempt1(void)
{
  if (znak1 == '+')
  {
    tempt1 = tempt1 + comptempt1;
  }else if(znak1 == '-')
  {
   tempt1 = tempt1 - comptempt1;
  }
}

void compensaciya_tempt2(void)
{
  if (znak2 == '+')
  {
    tempt2 = tempt2 + comptempt2;
  }else if(znak2 == '-')
  {
   tempt2 = tempt2 - comptempt2;
  }
}
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
  nexSerial.print("click sound,1");
  nexSerial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nexSerial.write(0xff);
  nexSerial.write(0xff);  
}

void btn_start_click(void){
  nexSerial.print("click btnstart,1");
  nexSerial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nexSerial.write(0xff);
  nexSerial.write(0xff); 
}
void btn_stop_click(void){
  nexSerial.print("click btnstop,1");
  nexSerial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nexSerial.write(0xff);
  nexSerial.write(0xff); 
}

void bt0_click(void){
  nexSerial.print("click bt0,1");
  nexSerial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nexSerial.write(0xff);
  nexSerial.write(0xff); 
  nexSerial.print("click bt0,0");
  nexSerial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nexSerial.write(0xff);
  nexSerial.write(0xff);  
}
void b4_click(void){
  nexSerial.print("click b4,1");
  nexSerial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nexSerial.write(0xff);
  nexSerial.write(0xff);
}
void page_main(void){
  nexSerial.print("page main");
  nexSerial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nexSerial.write(0xff);
  nexSerial.write(0xff);
}
void page_termoprofily(void){
  nexSerial.print("page termoprofily");
  nexSerial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nexSerial.write(0xff);
  nexSerial.write(0xff);
}

float take1_averaged_reading ()
{ 
  int N = 10; // количество шагов и на неё делим
  float sum = 0.0 ;
  for (byte i = 0 ; i < N ; i++)
    sum += tempt1 ; // whatever code reads the sensor
  return sum / N ;
}

float take2_averaged_reading ()
{
  int N = 10; // количество шагов и на неё делим
  float sum = 0.0 ;
  for (byte i = 0 ; i < N ; i++)
    sum += tempt2 ; // whatever code reads the sensor
  return sum / N ;
}
/**
void cle(void){
  nex2Serial.print("cle 1,255");
  nex2Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nex2Serial.write(0xff);
  nex2Serial.write(0xff);
  delay(8);
} 
**/

void gafick1(void){
  nex2Serial.print("add 1,0,");
  print_dec2(tempt1);
  nex2Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nex2Serial.write(0xff);
  nex2Serial.write(0xff);
  delay(8); 
}

void gafick2(void){
  nex2Serial.print("add 1,1,");
  print_dec2(tempt2);
  nex2Serial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nex2Serial.write(0xff);
  nex2Serial.write(0xff);
  delay(8);
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
  while(*string) nexSerial.write(*string++); // Пока разыменованное значение не ноль, отправлять через UART
}


void print_dec(uint32_t data)
{
  uint16_t num;  // вспомогательная переменная
  //for(num=0; data >= 1000000; num++) data -= 1000000; // Выделение десятков тысяч в переменной num
  //nexSerial.write(num + '0'); 
  for(num=0; data >= 100000; num++) data -= 100000; // Выделение десятков тысяч в переменной num
  nexSerial.write(num + '0'); 
  for(num=0; data >= 10000; num++) data -= 10000; // Выделение десятков тысяч в переменной num
  nexSerial.write(num + '0');              // Печать десятков тысяч в ASCII
  for(num=0; data >= 1000; num++) data -= 1000; // Выделение тысяч в переменной num
  nexSerial.write(num + '0');              // Печать тысяч в ASCII
  for(num=0; data >= 100; num++) data -= 100;   // Выделение сотен в переменной num
  nexSerial.write(num + '0');              // Печать сотен в ASCII
  for(num=0; data >= 10; num++) data -= 10;   // Выделение десятков в переменной num
  nexSerial.write(num + '0');              // Печать десятков в ASCII
  nexSerial.write(data + '0');             // Печать единиц в ASCII
}
void print_dec2(uint32_t data)
{
  uint16_t num;  // вспомогательная переменная
  //for(num=0; data >= 1000000; num++) data -= 1000000; // Выделение десятков тысяч в переменной num
  //nexSerial.write(num + '0'); 
  for(num=0; data >= 100000; num++) data -= 100000; // Выделение десятков тысяч в переменной num
  nex2Serial.write(num + '0'); 
  for(num=0; data >= 10000; num++) data -= 10000; // Выделение десятков тысяч в переменной num
  nex2Serial.write(num + '0');              // Печать десятков тысяч в ASCII
  for(num=0; data >= 1000; num++) data -= 1000; // Выделение тысяч в переменной num
  nex2Serial.write(num + '0');              // Печать тысяч в ASCII
  for(num=0; data >= 100; num++) data -= 100;   // Выделение сотен в переменной num
  nex2Serial.write(num + '0');              // Печать сотен в ASCII
  for(num=0; data >= 10; num++) data -= 10;   // Выделение десятков в переменной num
  nex2Serial.write(num + '0');              // Печать десятков в ASCII
  nex2Serial.write(data + '0');             // Печать единиц в ASCII
}

void sendFFFFFF(void)
{
  nexSerial.write(0xFF);
  nexSerial.write(0xFF);
  nexSerial.write(0xFF);
} // Здесь закачивается код Максима Селиванова 


// Гистерезис нижний нагреватель
void reguln()
{
  if (detect_zero == 1)
  { // без детектора ноля
    regulator2.input =  tempt2; 
    digitalWrite(nigniy_1, regulator2.getResultTimer());   // отправляем на реле (ОС работает по своему таймеру)
 
  }else
  { // с детектором ноля
    regulator2.input =  tempt2; 
    digitalWrite(nigniy_1, dimmer[0] = regulator2.getResultTimer());   // отправляем на реле (ОС работает по своему таймеру)
 
  }
    
}
// Гистерезис верхний нагреватель
void regul() 
{
  if (detect_zero == 1)
  { // без детектора ноля
    regulator.input =  tempt1;
    digitalWrite(verhniy_1, regulator.getResultTimer());   // отправляем на реле (ОС работает по своему таймеру)
  
  }else
  { // с детектором ноля
    regulator.input =  tempt1;
    digitalWrite(verhniy_1, dimmer[1] = regulator.getResultTimer());   // отправляем на реле (ОС работает по своему таймеру)
  
  }
   
}

void AnalyseString(String incStr) 
{
  if ((incStr.indexOf("bt0-on") >= 0) || (incStr.indexOf("btn_start") >= 0))
  {     // слушаем UART на передачу команды bt0-on подаем 5 вольт на вывод
    reley_n=1;
    sec = 3;
    termoprofily1_9 = 1;
    bt0 = 1;
    
  } else if ((incStr.indexOf("bt0-off") >= 0) || (incStr.indexOf("btn_stop") >= 0))
  { //слушаем UART на команду bt0-off и снимаем 5 вольт с вывода
    reley_n=0;
    bt0 = 0;
    Timer2.disableISR();
    shag = 0;
    //sec=0;
    termoprofily1_9 = 0;
    termoprofily10 = 0;
    reley_n1=0;
    reley_v=0;
    analogWrite(nigniy_1, 0);
    analogWrite(verhniy_1, 0);
  
    if (termoprofily == 0)
    {
         shag = 0;
         sec=3;
         outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag
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
    } else if (termoprofily == 1)
    {
         shag = 0;      
         sec=3;
         outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag         
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
    } else if (termoprofily == 2)
    {
         EEPROM.get(22, sec);
         EEPROM.get(27, temp1);
         EEPROM.get(32, temp2);
         //термопрофиль User 1 шаг 0
         EEPROM.get(3200, gradsecv);
         EEPROM.get(3205, gradsecn);
           EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
         //термопрофиль User 1 шаг 0
         EEPROM.get(3746, pwmv);
         EEPROM.get(3747, pwmn);
         // Оповещение температуры User 1
        EEPROM.get(3856,setopov1);
        EEPROM.get(3861,setopov2);
        EEPROM.get(3866,setopov3);
        EEPROM.get(3871,setopov4);
         shag = 0;         
         //sec=0;
         outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag         
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
    } else if (termoprofily == 3)
    {
        //термопрофиль User 2 шаг 0
          EEPROM.get(1300, sec);
          EEPROM.get(1305, temp1);
          EEPROM.get(1310, temp2);
              //термопрофиль User 2 шаг 0
          EEPROM.get(3310, gradsecv);
          EEPROM.get(3315, gradsecn);
            EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
          //термопрофиль User 2 шаг 0
          EEPROM.get(3768, pwmv);
          EEPROM.get(3769, pwmn);
          // Оповещение температуры User 2
          EEPROM.get(3876,setopov1);
          EEPROM.get(3881,setopov2);
          EEPROM.get(3886,setopov3);
          EEPROM.get(3891,setopov4);
         shag = 0;         
         //sec=0;
         outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag         
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
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
    } else if (termoprofily == 4)
    {
        //термопрофиль User 3 шаг 0
          EEPROM.get(1775, sec);
          EEPROM.get(1780, temp1);
          EEPROM.get(1785, temp2);
            //термопрофиль User 3 шаг 0
          EEPROM.get(3420, gradsecv);
          EEPROM.get(3425, gradsecn); 
            EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
          //термопрофиль User 3 шаг 0
          EEPROM.get(3790, pwmv);
          EEPROM.get(3791, pwmn);
          // Оповещение температуры User 3
          EEPROM.get(3896,setopov1);
          EEPROM.get(3901,setopov2);
          EEPROM.get(3906,setopov3);
          EEPROM.get(3911,setopov4);         
         shag = 0;         
         //sec=0;
         outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag         
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
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
    } else if (termoprofily == 5)
    {
        //термопрофиль User 4 шаг 0
          EEPROM.get(2250, sec);
          EEPROM.get(2255, temp1);
          EEPROM.get(2260, temp2);
         //термопрофиль User 4 шаг 0
          EEPROM.get(3530, gradsecv);
          EEPROM.get(3535, gradsecn);
            EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
          //термопрофиль User 4 шаг 0
         EEPROM.get(3812, pwmv);
         EEPROM.get(3813, pwmn);
         // Оповещение температуры User 4
        EEPROM.get(3916,setopov1);
        EEPROM.get(3921,setopov2);
        EEPROM.get(3926,setopov3);
        EEPROM.get(3931,setopov4);
         shag = 0;         
         //sec=0;
         outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag         
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
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
    } else if (termoprofily == 6)
    {
      //термопрофиль User 5 шаг 0
        EEPROM.get(2725, sec);
        EEPROM.get(2730, temp1);
        EEPROM.get(2735, temp2);
          //термопрофиль User 5 шаг 0
        EEPROM.get(3640, gradsecv);
        EEPROM.get(3645, gradsecn);
           EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
        //термопрофиль User 5 шаг 0
        EEPROM.get(3834, pwmv);
        EEPROM.get(3835, pwmn); 
        // Оповещение температуры User 5
        EEPROM.get(3936,setopov1);
        EEPROM.get(3941,setopov2);
        EEPROM.get(3946,setopov3);
        EEPROM.get(3951,setopov4);      
         shag = 0;         
         //sec=0;
         outNumber("shagt.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("termoprofily.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidv.shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("pidn.shag.val", shag);  // Отображение числа в числовом компоненте shag         
         outNumber("termoprofily.sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
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

  }
  if (incStr.indexOf("c0-on") >= 0) 
  {      // тоже самое что и bt0
    nigniynagrev = 1;
    digitalWrite(relay_nigniy, HIGH);
     
  }else if (incStr.indexOf("c0-off") >= 0)
  {
    nigniynagrev = 0;
    digitalWrite(relay_nigniy, LOW);
  }
  if (incStr.indexOf("c3-on") >= 0)
  { 
     verhniynagrev = 1; 
     digitalWrite(relay_verhniy, HIGH);
    
  } else if (incStr.indexOf("c3-off") >= 0)
  {
    verhniynagrev = 0;
    digitalWrite(relay_verhniy, LOW);
  }
  if (incStr.indexOf("coolerv-on") >= 0) 
  {      // тоже самое что и bt0
      analogWrite(coolerv, coolvust1);
  }  else if (incStr.indexOf("coolerv-off") >= 0) 
  {
      analogWrite(coolerv, 0);
  }
  if (incStr.indexOf("cooler-on") >= 0) 
  {      // тоже самое что и bt0
     analogWrite(cooler, coolpust2);
  }  else if (incStr.indexOf("cooler-off") >= 0) 
  {
     analogWrite(cooler, 0);
  }
  if (incStr.indexOf("bt1-on") >= 0) 
  {      // тоже самое что и bt0
     digitalWrite(lampa, HIGH);
  }  else if (incStr.indexOf("bt1-off") >= 0) 
  {
     digitalWrite(lampa, LOW);
  }
   if (incStr.indexOf("pid") >= 0) 
   {     // выбран пид регулирование
     ph=1;
    
   } else if (incStr.indexOf("hesterezis") >= 0) 
   { // выбран гистерезис
     ph=0;
   }
  if (incStr.indexOf("detectzero1") >= 0) 
  {     
    detect_zero = 1;
  }
  if (incStr.indexOf("detectzero0") >= 0) 
  {     
    detect_zero = 0;
  }
  if (incStr.indexOf("hnupr") >= 0) 
  {     
    hnup = 1;
  }
  if (incStr.indexOf("hnupn") >= 0) 
  {     
    hnup = 0;
  }
  if (incStr.indexOf("hndownr") >= 0) 
  {     
    hndown = 1;
  }
  if (incStr.indexOf("hndownn") >= 0) 
  {     
    hndown = 0;
  }
  
  if (incStr.indexOf("ntermo") >= 0) 
  {     // выбран пид регулирование
     EEPROM.put(207, termoprofily);
  }
  if (incStr.indexOf("wattsave") >= 0) 
  {     // Сохранение Мощности в eeprom память по умолчанию
    
    if (shag == 0)
    {
      if (termoprofily == 2)
      { // User 1
        //термопрофиль User 1 шаг 0
        EEPROM.put(3746, pwmv);
        EEPROM.put(3747, pwmn);   
      }else if (termoprofily == 3)
      { // User 2
        //термопрофиль User 2 шаг 0
        EEPROM.put(3768, pwmv);
        EEPROM.put(3769, pwmn);        	
      }else if (termoprofily == 4)
      { // User 3
        //термопрофиль User 3 шаг 0
        EEPROM.put(3790, pwmv);
        EEPROM.put(3791, pwmn);
      }else if (termoprofily == 5)
      { // User 4
        //термопрофиль User 4 шаг 0
        EEPROM.put(3812, pwmv);
        EEPROM.put(3813, pwmn);                       
      }else if (termoprofily == 6)
      { // User 5
        //термопрофиль User 5 шаг 0
        EEPROM.put(3834, pwmv);
        EEPROM.put(3835, pwmn);        
      }
    }else if (shag == 1)
    {
      if (termoprofily == 2)
      { // User 1
        //термопрофиль User 1 шаг 1
        EEPROM.put(3748, pwmv);
        EEPROM.put(3749, pwmn);     
      }else if (termoprofily == 3)
      { // User 2
        //термопрофиль User 2 шаг 1
        EEPROM.put(3770, pwmv);
        EEPROM.put(3771, pwmn);        	
      }else if (termoprofily == 4)
      { // User 3
        //термопрофиль User 3 шаг 1
        EEPROM.put(3792, pwmv);
        EEPROM.put(3793, pwmn);         
      }else if (termoprofily == 5)
      { // User 4
        //термопрофиль User 4 шаг 1
        EEPROM.put(3814, pwmv);
        EEPROM.put(3815, pwmn);                       
      }else if (termoprofily == 6)
      { // User 5
        //термопрофиль User 5 шаг 1
        EEPROM.put(3836, pwmv);
        EEPROM.put(3837, pwmn);        
      }
    }else if (shag == 2)
    {
      if (termoprofily == 2)
      { // User 1
        //термопрофиль User 1 шаг 2
        EEPROM.put(3750, pwmv);
        EEPROM.put(3751, pwmn);    
      }else if (termoprofily == 3)
      { // User 2
        //термопрофиль User 2 шаг 2
        EEPROM.put(3772, pwmv);
        EEPROM.put(3773, pwmn);        	
      }else if (termoprofily == 4)
      { // User 3
        //термопрофиль User 3 шаг 2
        EEPROM.put(3794, pwmv);
        EEPROM.put(3795, pwmn);         
      }else if (termoprofily == 5)
      { // User 4
        //термопрофиль User 4 шаг 2
        EEPROM.put(3816, pwmv);
        EEPROM.put(3817, pwmn);                       
      }else if (termoprofily == 6)
      { // User 5
        //термопрофиль User 5 шаг 2
        EEPROM.put(3838, pwmv);
        EEPROM.put(3839, pwmn);        
      }
    }else if (shag == 3)
    {
      if (termoprofily == 2)
      { // User 1
        //термопрофиль User 1 шаг 3
        EEPROM.put(3752, pwmv);
        EEPROM.put(3753, pwmn);     
      }else if (termoprofily == 3)
      { // User 2
        //термопрофиль User 2 шаг 3
        EEPROM.put(3774, pwmv);
        EEPROM.put(3775, pwmn);        	
      }else if (termoprofily == 4)
      { // User 3
        //термопрофиль User 3 шаг 3
        EEPROM.put(3796, pwmv);
        EEPROM.put(3797, pwmn);         
      }else if (termoprofily == 5)
      { // User 4
        //термопрофиль User 4 шаг 3
        EEPROM.put(3818, pwmv);
        EEPROM.put(3819, pwmn);                       
      }else if (termoprofily == 6)
      { // User 5
        //термопрофиль User 5 шаг 3
        EEPROM.put(3840, pwmv);
        EEPROM.put(3841, pwmn);        
      }
    }else if (shag == 4)
    {
      if (termoprofily == 2)
      { // User 1
        //термопрофиль User 1 шаг 4
        EEPROM.put(3754, pwmv);
        EEPROM.put(3755, pwmn);   
      }else if (termoprofily == 3)
      { // User 2
        //термопрофиль User 2 шаг 4
        EEPROM.put(3776, pwmv);
        EEPROM.put(3777, pwmn);        	
      }else if (termoprofily == 4)
      { // User 3
        //термопрофиль User 3 шаг 4
        EEPROM.put(3798, pwmv);
        EEPROM.put(3799, pwmn);         
      }else if (termoprofily == 5)
      { // User 4
        //термопрофиль User 4 шаг 4
        EEPROM.put(3820, pwmv);
        EEPROM.put(3821, pwmn);                       
      }else if (termoprofily == 6)
      { // User 5
        //термопрофиль User 5 шаг 4
        EEPROM.put(3842, pwmv);
        EEPROM.put(3843, pwmn);        
      }
    }else if (shag == 5)
    {
      if (termoprofily == 2)
      { // User 1
        //термопрофиль User 1 шаг 5
        EEPROM.put(3756, pwmv);
        EEPROM.put(3757, pwmn);     
      }else if (termoprofily == 3)
      { // User 2
        //термопрофиль User 2 шаг 5
        EEPROM.put(3778, pwmv);
        EEPROM.put(3779, pwmn);        	
      }else if (termoprofily == 4)
      { // User 3
        //термопрофиль User 3 шаг 5
        EEPROM.put(3800, pwmv);
        EEPROM.put(3801, pwmn);         
      }else if (termoprofily == 5)
      { // User 4
        //термопрофиль User 4 шаг 5
        EEPROM.put(3822, pwmv);
        EEPROM.put(3823, pwmn);                       
      }else if (termoprofily == 6)
      { // User 5
        //термопрофиль User 5 шаг 5
        EEPROM.put(3844, pwmv);
        EEPROM.put(3845, pwmn);        
      }
    }else if (shag == 6)
    {
      if (termoprofily == 2)
      { // User 1
        //термопрофиль User 1 шаг 6
        EEPROM.put(3758, pwmv);
        EEPROM.put(3759, pwmn);     
      }else if (termoprofily == 3)
      { // User 2
        //термопрофиль User 2 шаг 6
        EEPROM.put(3780, pwmv);
        EEPROM.put(3781, pwmn);        	
      }else if (termoprofily == 4)
      { // User 3
        //термопрофиль User 3 шаг 6
        EEPROM.put(3802, pwmv);
        EEPROM.put(3803, pwmn);         
      }else if (termoprofily == 5)
      { // User 4
        //термопрофиль User 4 шаг 6
        EEPROM.put(3824, pwmv);
        EEPROM.put(3825, pwmn);                        
      }else if (termoprofily == 6)
      { // User 5
        //термопрофиль User 5 шаг 6
        EEPROM.put(3846, pwmv);
        EEPROM.put(3847, pwmn);        
      }
    }else if (shag == 7)
    {
      if (termoprofily == 2)
      { // User 1
        //термопрофиль User 1 шаг 7
        EEPROM.put(3760, pwmv);
        EEPROM.put(3761, pwmn);      
      }else if (termoprofily == 3)
      { // User 2
        //термопрофиль User 2 шаг 7
        EEPROM.put(3782, pwmv);
        EEPROM.put(3783, pwmn);        	
      }else if (termoprofily == 4)
      { // User 3
        //термопрофиль User 3 шаг 7
        EEPROM.put(3804, pwmv);
        EEPROM.put(3805, pwmn);         
      }else if (termoprofily == 5)
      { // User 4
        //термопрофиль User 4 шаг 7
        EEPROM.put(3826, pwmv);
        EEPROM.put(3827, pwmn);                       
      }else if (termoprofily == 6)
      { // User 5
        //термопрофиль User 5 шаг 7
        EEPROM.put(3848, pwmv);
        EEPROM.put(3849, pwmn);        
      }
    }else if (shag == 8)
    {
      if (termoprofily == 2)
      { // User 1
        //термопрофиль User 1 шаг 8
        EEPROM.put(3762, pwmv);
        EEPROM.put(3763, pwmn);     
      }else if (termoprofily == 3)
      { // User 2
        //термопрофиль User 2 шаг 8
        EEPROM.put(3784, pwmv);
        EEPROM.put(3785, pwmn);        	
      }else if (termoprofily == 4)
      { // User 3
        //термопрофиль User 3 шаг 8
        EEPROM.put(3806, pwmv);
        EEPROM.put(3807, pwmn);         
      }else if (termoprofily == 5)
      { // User 4
        //термопрофиль User 4 шаг 8
        EEPROM.put(3828, pwmv);
        EEPROM.put(3829, pwmn);                       
      }else if (termoprofily == 6)
      { // User 5
        //термопрофиль User 5 шаг 8
        EEPROM.put(3850, pwmv);
        EEPROM.put(3851, pwmn);        
      }
    }else if (shag == 9)
    {
      if (termoprofily == 2)
      { // User 1
        //термопрофиль User 1 шаг 9
        EEPROM.put(3764, pwmv);
        EEPROM.put(3765, pwmn);    
      }else if (termoprofily == 3)
      { // User 2
        //термопрофиль User 2 шаг 9
        EEPROM.put(3786, pwmv);
        EEPROM.put(3787, pwmn);        	
      }else if (termoprofily == 4)
      { // User 3
        //термопрофиль User 3 шаг 9
        EEPROM.put(3808, pwmv);
        EEPROM.put(3809, pwmn);         
      }else if (termoprofily == 5)
      { // User 4
        //термопрофиль User 4 шаг 9
        EEPROM.put(3830, pwmv);
        EEPROM.put(3831, pwmn);                       
      }else if (termoprofily == 6)
      { // User 5
        //термопрофиль User 5 шаг 9
        EEPROM.put(3852, pwmv);
        EEPROM.put(3853, pwmn);        
      }
    }else if (shag == 10)
    {
      if (termoprofily == 2)
      { // User 1
        //термопрофиль User 1 шаг 10
        EEPROM.put(3766, pwmv);
        EEPROM.put(3767, pwmn);    
      }else if (termoprofily == 3)
      { // User 2
        //термопрофиль User 2 шаг 10
        EEPROM.put(3788, pwmv);
        EEPROM.put(3789, pwmn);        	
      }else if (termoprofily == 4)
      { // User 3
        //термопрофиль User 3 шаг 10
        EEPROM.put(3810, pwmv);
        EEPROM.put(3811, pwmn);         
      }else if (termoprofily == 5)
      { // User 4
        //термопрофиль User 4 шаг 10
        EEPROM.put(3832, pwmv);
        EEPROM.put(3833, pwmn);                       
      }else if (termoprofily == 6)
      { // User 5
        //термопрофиль User 5 шаг 10
        EEPROM.put(3854, pwmv);
        EEPROM.put(3855, pwmn);        
      }
    }
  } 
  if (incStr.indexOf("coolerssave") >= 0) 
  {     // Сохранение Мощности в eeprom память по умолчанию
    if (EEPROM.get(197, coolervh) != coolervh && EEPROM.get(202, coolerp) != coolerp)
    {
      EEPROM.put(197, coolervh);
      EEPROM.put(202, coolerp);  
    }
  } 
  if (incStr.indexOf("pidvsave") >= 0) 
  {     // Сохранение ПИД регулирование, П И Д составляющих и dt в eeprom память по умолчанию
          if (shag == 0)
          {
            if (termoprofily == 2)
            { 
              EEPROM.put(227, dtv);
            }  else if (termoprofily == 3)
              { 
                	//термопрофиль User 2
	                //delay(10);
                  EEPROM.put(1465, dtv);
              } else if (termoprofily == 4)
              { 
                //термопрофиль User 3
                //delay(10);
                EEPROM.put(1940, dtv); 
              }else if (termoprofily == 5)
              { 
                //термопрофиль User 4
                //delay(10);
                EEPROM.put(2415, dtv);                
              }else if (termoprofily == 6)
              { 
                //термопрофиль User 5
                EEPROM.put(2890, dtv);
              }
          }
          //shag == 1 // termoprofily 2 верхний нагреватель
           else if (shag == 1)
          {
            if (termoprofily == 2)
            { 
              EEPROM.put(7, kpv);
              EEPROM.put(12, kiv);
              EEPROM.put(17, kdv);
            } else if (termoprofily == 3)
              { 
                //shag == 1 
                EEPROM.put(1475, kpv);
                EEPROM.put(1480, kiv);
                EEPROM.put(1485, kdv);  
              }else if (termoprofily == 4)
              { 
                //shag == 1 
                EEPROM.put(1950, kpv);
                EEPROM.put(1955, kiv);
                EEPROM.put(1960, kdv);
              }else if (termoprofily == 5)
              { 
                //shag == 1 
                EEPROM.put(2425, kpv);
                EEPROM.put(2430, kiv);
                EEPROM.put(2435, kdv);
              }else if (termoprofily == 6)
              { 
                //термопрофиль User 5
                //shag == 1 
                EEPROM.put(2900, kpv);
                EEPROM.put(2905, kiv);
                EEPROM.put(2910, kdv);
              }
          } else if (shag == 2)
          {
            if (termoprofily == 2)
            { 
              EEPROM.put(1030, kpv);
              EEPROM.put(1035, kiv);
              EEPROM.put(1040, kdv);
            } else if (termoprofily == 3)
              { 
                //shag == 2 
                EEPROM.put(1490, kpv);
                EEPROM.put(1495, kiv);
                EEPROM.put(1500, kdv);  
              }else if (termoprofily == 4)
              { 
              //shag == 2 
                EEPROM.put(1965, kpv);
                EEPROM.put(1970, kiv);
                EEPROM.put(1975, kdv);
              }else if (termoprofily == 5)
              { 
              //shag == 2 
                EEPROM.put(2440, kpv);
                EEPROM.put(2445, kiv);
                EEPROM.put(2450, kdv);
              }else if (termoprofily == 6)
              { 
                //термопрофиль User 5
                //shag == 2 
                EEPROM.put(2915, kpv);
                EEPROM.put(2920, kiv);
                EEPROM.put(2925, kdv);
              }
          } else if (shag == 3)
          {
            if (termoprofily == 2)
            { 
              EEPROM.put(1045, kpv);
              EEPROM.put(1050, kiv);
              EEPROM.put(1055, kdv);
            } else if (termoprofily == 3)
              { 
                //shag == 3
                EEPROM.put(1505, kpv);
                EEPROM.put(1510, kiv);
                EEPROM.put(1515, kdv);  
              }else if (termoprofily == 4)
              { 
              //shag == 3
                EEPROM.put(1980, kpv);
                EEPROM.put(1985, kiv);
                EEPROM.put(1990, kdv);
              }else if (termoprofily == 5)
              { 
                //shag == 3
                EEPROM.put(2455, kpv);
                EEPROM.put(2460, kiv);
                EEPROM.put(2465, kdv);
              }else if (termoprofily == 6)
              { 
                //термопрофиль User 5
                //shag == 3
                EEPROM.put(2930, kpv);
                EEPROM.put(2935, kiv);
                EEPROM.put(2940, kdv);
              }
          } else if (shag == 4)
          {
            if (termoprofily == 2)
            { 
              EEPROM.put(1060, kpv);
              EEPROM.put(1065, kiv);
              EEPROM.put(1070, kdv);
            } else if (termoprofily == 3)
              { 
                //shag == 4
                EEPROM.put(1520, kpv);
                EEPROM.put(1525, kiv);
                EEPROM.put(1530, kdv);
              }else if (termoprofily == 4)
              { 
                //shag == 4
                EEPROM.put(1995, kpv);
                EEPROM.put(2000, kiv);
                EEPROM.put(2005, kdv);
              }else if (termoprofily == 5)
              { 
                //shag == 4
                EEPROM.put(2470, kpv);
                EEPROM.put(2475, kiv);
                EEPROM.put(2480, kdv);
              }else if (termoprofily == 6)
              { 
                //термопрофиль User 5
                //shag == 4
                EEPROM.put(2945, kpv);
                EEPROM.put(2950, kiv);
                EEPROM.put(2955, kdv);
              }
          } else if (shag == 5)
          {
            if (termoprofily == 2)
            { 
              EEPROM.put(1075, kpv);
              EEPROM.put(1080, kiv);
              EEPROM.put(1085, kdv);
            } else if (termoprofily == 3)
              { 
                //shag == 5
                EEPROM.put(1535, kpv);
                EEPROM.put(1540, kiv);
                EEPROM.put(1545, kdv);
              }else if (termoprofily == 4)
              { 
                //shag == 5
                EEPROM.put(2010, kpv);
                EEPROM.put(2015, kiv);
                EEPROM.put(2020, kdv);
              }else if (termoprofily == 5)
              { 
                //shag == 5
                EEPROM.put(2485, kpv);
                EEPROM.put(2490, kiv);
                EEPROM.put(2495, kdv);
              }else if (termoprofily == 6)
              { 
                //термопрофиль User 5
                //shag == 5
                EEPROM.put(2960, kpv);
                EEPROM.put(2965, kiv);
                EEPROM.put(2970, kdv);
              }
          } else if (shag == 6)
          {
            if (termoprofily == 2)
            { 
              EEPROM.put(1090, kpv);
              EEPROM.put(1095, kiv);
              EEPROM.put(1100, kdv);
            } else if (termoprofily == 3)
              { 
                //shag == 6
                EEPROM.put(1550, kpv);
                EEPROM.put(1555, kiv);
                EEPROM.put(1560, kdv);
              }else if (termoprofily == 4)
              { 
                //shag == 6
                EEPROM.put(2025, kpv);
                EEPROM.put(2030, kiv);
                EEPROM.put(2035, kdv);
              }else if (termoprofily == 5)
              { 
                //shag == 6
                EEPROM.put(2500, kpv);
                EEPROM.put(2505, kiv);
                EEPROM.put(2510, kdv);
              }else if (termoprofily == 6)
              { 
                //термопрофиль User 5
                //shag == 6
                EEPROM.put(2975, kpv);
                EEPROM.put(2980, kiv);
                EEPROM.put(2985, kdv);
              }
          } else if (shag == 7)
          {
            if (termoprofily == 2)
            { 
              EEPROM.put(1105, kpv);
              EEPROM.put(1110, kiv);
              EEPROM.put(1115, kdv);
            } else if (termoprofily == 3)
              { 
                //shag == 7
                EEPROM.put(1565, kpv);
                EEPROM.put(1570, kiv);
                EEPROM.put(1575, kdv);
              }else if (termoprofily == 4)
              { 
                //shag == 7
                EEPROM.put(2040, kpv);
                EEPROM.put(2045, kiv);
                EEPROM.put(2050, kdv);
              }else if (termoprofily == 5)
              { 
                //shag == 7
                EEPROM.put(2515, kpv);
                EEPROM.put(2520, kiv);
                EEPROM.put(2525, kdv);
              }else if (termoprofily == 6)
              { 
                //термопрофиль User 5
                //shag == 7
                EEPROM.put(2990, kpv);
                EEPROM.put(2995, kiv);
                EEPROM.put(3000, kdv);
              }
          } else if (shag == 8)
          {
            if (termoprofily == 2)
            { 
              EEPROM.put(1120, kpv);
              EEPROM.put(1125, kiv);
              EEPROM.put(1130, kdv);
            } else if (termoprofily == 3)
              { 
                //shag == 8
                EEPROM.put(1580, kpv);
                EEPROM.put(1585, kiv);
                EEPROM.put(1590, kdv);
              }else if (termoprofily == 4)
              { 
                //shag == 8
                EEPROM.put(2055, kpv);
                EEPROM.put(2060, kiv);
                EEPROM.put(2065, kdv);
              }else if (termoprofily == 5)
              { 
                //shag == 8
                EEPROM.put(2530, kpv);
                EEPROM.put(2535, kiv);
                EEPROM.put(2540, kdv);
              }else if (termoprofily == 6)
              { 
                //термопрофиль User 5
                //shag == 8
                EEPROM.put(3005, kpv);
                EEPROM.put(3010, kiv);
                EEPROM.put(3015, kdv);
              }
          } else if (shag == 9)
          {
            if (termoprofily == 2)
            { 
              EEPROM.put(1135, kpv);
              EEPROM.put(1140, kiv);
              EEPROM.put(1145, kdv);
            } else if (termoprofily == 3)
              { 
                //shag == 9
                EEPROM.put(1595, kpv);
                EEPROM.put(1600, kiv);
                EEPROM.put(1605, kdv);
              }else if (termoprofily == 4)
              { 
                //shag == 9
                EEPROM.put(2070, kpv);
                EEPROM.put(2075, kiv);
                EEPROM.put(2080, kdv);
              }else if (termoprofily == 5)
              { 
                //shag == 9
                EEPROM.put(2545, kpv);
                EEPROM.put(2550, kiv);
                EEPROM.put(2555, kdv);
              }else if (termoprofily == 6)
              { 
                //термопрофиль User 5
                //shag == 9
                EEPROM.put(3020, kpv);
                EEPROM.put(3025, kiv);
                EEPROM.put(3030, kdv);
              }
          } else if (shag == 10)
          {
            if (termoprofily == 2)
            { 
              EEPROM.put(1150, kpv);
              EEPROM.put(1155, kiv);
              EEPROM.put(1160, kdv);
            } else if (termoprofily == 3)
              { 
                //shag == 10
                EEPROM.put(1610, kpv);
                EEPROM.put(1615, kiv);
                EEPROM.put(1620, kdv);
              }else if (termoprofily == 4)
              { 
                //shag == 10
                EEPROM.put(2085, kpv);
                EEPROM.put(2090, kiv);
                EEPROM.put(2095, kdv);
              }else if (termoprofily == 5)
              { 
                //shag == 10
                EEPROM.put(2560, kpv);
                EEPROM.put(2565, kiv);
                EEPROM.put(2570, kdv);
              }else if (termoprofily == 6)
              { 
                //термопрофиль User 5
                //shag == 10
                EEPROM.put(3035, kpv);
                EEPROM.put(3040, kiv);
                EEPROM.put(3045, kdv);
              }
          } 
  } 
  if (incStr.indexOf("pidnsave") >= 0) 
  {     // Сохранение ПИД регулирование, П И Д составляющих  и dt в eeprom память по умолчанию
      if (shag == 0)
      {
        if (termoprofily == 2)
        { 
          EEPROM.put(232, dtn); 
        } else if (termoprofily == 3)
        { 
          //термопрофиль User 2
	        //delay(10);
          EEPROM.put(1470, dtn); 
        } else if (termoprofily == 4)
          { 
            EEPROM.put(1945, dtn); 
          }else if (termoprofily == 5)
          { 
            EEPROM.put(2420, dtn); 
          }else if (termoprofily == 6)
          { 
            EEPROM.put(2895, dtn);
          }
      }//shag == 1 // termoprofily 2 нижний нагреватель
        else if (shag == 1)
      {
        if (termoprofily == 2)
        { 
          //shag == 1 // termoprofily 2 нижний нагреватель
          EEPROM.put(212, kpn);
          EEPROM.put(217, kin);
          EEPROM.put(222, kdn);  
        } else if (termoprofily == 3)
        { 
          //shag == 1
          EEPROM.put(1625, kpn);
          EEPROM.put(1630, kin);
          EEPROM.put(1635, kdn);
        } else if (termoprofily == 4)
          { 
          //shag == 1
            EEPROM.put(2100, kpn);
            EEPROM.put(2105, kin);
            EEPROM.put(2110, kdn);
          }else if (termoprofily == 5)
          { 
          //shag == 1
            EEPROM.put(2575, kpn);
            EEPROM.put(2580, kin);
            EEPROM.put(2585, kdn);
          }else if (termoprofily == 6)
          {
          //shag == 1
            EEPROM.put(3050, kpn);
            EEPROM.put(3055, kin);
            EEPROM.put(3060, kdn);
          }
      }else if (shag == 2)
      {
        if (termoprofily == 2)
        { 
          //shag == 2
          EEPROM.put(1165, kpn);
          EEPROM.put(1170, kin);
          EEPROM.put(1175, kdn);
        } else if (termoprofily == 3)
        { 
          //shag == 2
          EEPROM.put(1640, kpn);
          EEPROM.put(1645, kin);
          EEPROM.put(1650, kdn);
        } else if (termoprofily == 4)
          { 
            //shag == 2
            EEPROM.put(2115, kpn);
            EEPROM.put(2120, kin);
            EEPROM.put(2125, kdn);
          }else if (termoprofily == 5)
          { 
            //shag == 2
            EEPROM.put(2590, kpn);
            EEPROM.put(2595, kin);
            EEPROM.put(2600, kdn);
          }else if (termoprofily == 6)
          {
            //shag == 2
            EEPROM.put(3065, kpn);
            EEPROM.put(3070, kin);
            EEPROM.put(3075, kdn);
          }
      }else if (shag == 3)
      {
        if (termoprofily == 2)
        { 
          //shag == 3
          EEPROM.put(1180, kpn);
          EEPROM.put(1185, kin);
          EEPROM.put(1190, kdn);
        } else if (termoprofily == 3)
        { 
          //shag == 3
          EEPROM.put(1655, kpn);
          EEPROM.put(1660, kin);
          EEPROM.put(1665, kdn);
        } else if (termoprofily == 4)
          { 
            //shag == 3
            EEPROM.put(2130, kpn);
            EEPROM.put(2135, kin);
            EEPROM.put(2140, kdn);
          }else if (termoprofily == 5)
          { 
            //shag == 3
            EEPROM.put(2605, kpn);
            EEPROM.put(2610, kin);
            EEPROM.put(2615, kdn);
          }else if (termoprofily == 6)
          {
            //shag == 3
            EEPROM.put(3080, kpn);
            EEPROM.put(3085, kin);
            EEPROM.put(3090, kdn);
          }
      }else if (shag == 4)
      {
        if (termoprofily == 2)
        { 
          //shag == 4
          EEPROM.put(1195, kpn);
          EEPROM.put(1200, kin);
          EEPROM.put(1205, kdn);
        } else if (termoprofily == 3)
        { 
          //shag == 4
          EEPROM.put(1670, kpn);
          EEPROM.put(1675, kin);
          EEPROM.put(1680, kdn);
        } else if (termoprofily == 4)
          { 
            //shag == 4
            EEPROM.put(2145, kpn);
            EEPROM.put(2150, kin);
            EEPROM.put(2155, kdn);
          }else if (termoprofily == 5)
          { 
            //shag == 4
            EEPROM.put(2620, kpn);
            EEPROM.put(2625, kin);
            EEPROM.put(2630, kdn);
          }else if (termoprofily == 6)
          {
            //shag == 4
            EEPROM.put(3095, kpn);
            EEPROM.put(3100, kin);
            EEPROM.put(3105, kdn);
          }
      }else if (shag == 5)
      {
        if (termoprofily == 2)
        { 
          //shag == 5
          EEPROM.put(1210, kpn);
          EEPROM.put(1215, kin);
          EEPROM.put(1220, kdn);
        } else if (termoprofily == 3)
        { 
          //shag == 5
          EEPROM.put(1685, kpn);
          EEPROM.put(1690, kin);
          EEPROM.put(1695, kdn);
        } else if (termoprofily == 4)
          { 
            //shag == 5
            EEPROM.put(2160, kpn);
            EEPROM.put(2165, kin);
            EEPROM.put(2170, kdn);
          }else if (termoprofily == 5)
          { 
            //shag == 5
            EEPROM.put(2635, kpn);
            EEPROM.put(2640, kin);
            EEPROM.put(2645, kdn);
          }else if (termoprofily == 6)
          {
            //shag == 5
            EEPROM.put(3110, kpn);
            EEPROM.put(3115, kin);
            EEPROM.put(3120, kdn);
          }
      }else if (shag == 6)
      {
        if (termoprofily == 2)
        { 
          //shag == 6
          EEPROM.put(1225, kpn);
          EEPROM.put(1230, kin);
          EEPROM.put(1235, kdn);
        } else if (termoprofily == 3)
        { 
          //shag == 6
          EEPROM.put(1700, kpn);
          EEPROM.put(1705, kin);
          EEPROM.put(1710, kdn);
        } else if (termoprofily == 4)
          { 
            //shag == 6
            EEPROM.put(2175, kpn);
            EEPROM.put(2180, kin);
            EEPROM.put(2185, kdn);
          }else if (termoprofily == 5)
          { 
            //shag == 6
            EEPROM.put(2650, kpn);
            EEPROM.put(2655, kin);
            EEPROM.put(2660, kdn);
          }else if (termoprofily == 6)
          {
            //shag == 6
            EEPROM.put(3125, kpn);
            EEPROM.put(3130, kin);
            EEPROM.put(3135, kdn);
          }
      }else if (shag == 7)
      {
        if (termoprofily == 2)
        { 
          //shag == 7
          EEPROM.put(1240, kpn);
          EEPROM.put(1245, kin);
          EEPROM.put(1250, kdn);
        } else if (termoprofily == 3)
        { 
          //shag == 7
          EEPROM.put(1715, kpn);
          EEPROM.put(1720, kin);
          EEPROM.put(1725, kdn);
        } else if (termoprofily == 4)
          { 
            //shag == 7
            EEPROM.put(2190, kpn);
            EEPROM.put(2195, kin);
            EEPROM.put(2200, kdn);
          }else if (termoprofily == 5)
          { 
            //shag == 7
            EEPROM.put(2665, kpn);
            EEPROM.put(2670, kin);
            EEPROM.put(2675, kdn);
          }else if (termoprofily == 6)
          {
            //shag == 7
            EEPROM.put(3140, kpn);
            EEPROM.put(3145, kin);
            EEPROM.put(3150, kdn);
          }
      }else if (shag == 8)
      {
        if (termoprofily == 2)
        { 
          //shag == 8
          EEPROM.put(1255, kpn);
          EEPROM.put(1260, kin);
          EEPROM.put(1265, kdn);
        } else if (termoprofily == 3)
        { 
          //shag == 8
          EEPROM.put(1730, kpn);
          EEPROM.put(1735, kin);
          EEPROM.put(1740, kdn);
        } else if (termoprofily == 4)
          { 
            //shag == 8
            EEPROM.put(2205, kpn);
            EEPROM.put(2210, kin);
            EEPROM.put(2215, kdn);
          }else if (termoprofily == 5)
          { 
            //shag == 8
            EEPROM.put(2680, kpn);
            EEPROM.put(2685, kin);
            EEPROM.put(2690, kdn);
          }else if (termoprofily == 6)
          {
            //shag == 8
            EEPROM.put(3155, kpn);
            EEPROM.put(3160, kin);
            EEPROM.put(3165, kdn);
          }
      }else if (shag == 9)
      {
        if (termoprofily == 2)
        { 
          //shag == 9
          EEPROM.put(1270, kpn);
          EEPROM.put(1275, kin);
          EEPROM.put(1280, kdn);
        } else if (termoprofily == 3)
        { 
          //shag == 9
          EEPROM.put(1745, kpn);
          EEPROM.put(1750, kin);
          EEPROM.put(1755, kdn);
        } else if (termoprofily == 4)
          { 
            //shag == 9
            EEPROM.put(2220, kpn);
            EEPROM.put(2225, kin);
            EEPROM.put(2230, kdn);
          }else if (termoprofily == 5)
          { 
            //shag == 9
            EEPROM.put(2695, kpn);
            EEPROM.put(2700, kin);
            EEPROM.put(2705, kdn);
          }else if (termoprofily == 6)
          {
            //shag == 9
            EEPROM.put(3170, kpn);
            EEPROM.put(3175, kin);
            EEPROM.put(3180, kdn);
          }
      }else if (shag == 10)
      {
        if (termoprofily == 2)
        { 
          //shag == 10
          EEPROM.put(1285, kpn);
          EEPROM.put(1290, kin);
          EEPROM.put(1295, kdn);
        } else if (termoprofily == 3)
        { 
          //shag == 10
          EEPROM.put(1760, kpn);
          EEPROM.put(1765, kin);
          EEPROM.put(1770, kdn);
        } else if (termoprofily == 4)
          { 
            //shag == 10
            EEPROM.put(2235, kpn);
            EEPROM.put(2240, kin);
            EEPROM.put(2245, kdn);
          }else if (termoprofily == 5)
          { 
            //shag == 10
            EEPROM.put(2710, kpn);
            EEPROM.put(2715, kin);
            EEPROM.put(2720, kdn);
          }else if (termoprofily == 6)
          {
            //shag == 10
            EEPROM.put(3185, kpn);
            EEPROM.put(3190, kin);
            EEPROM.put(3195, kdn);
          }
      }
  } 
  if (incStr.indexOf("compensave1") >= 0) 
  {     
    
     EEPROM.put(237, comptempt1);
     EEPROM.put(247, znak1);
    
  }
  if (incStr.indexOf("compensave2") >= 0) 
  {     
    
     EEPROM.put(242, comptempt2);
     EEPROM.put(252, znak2);
    
  } 
  
  if (incStr.indexOf("termosave") >= 0) 
  {     // Сохранение ПИД регулирование, П И Д составляющих в eeprom память по умолчанию
     if(shag == 0)
     {
       if (termoprofily == 2)
       {
         
           EEPROM.put(22, sec);
           EEPROM.put(27, temp1);
           EEPROM.put(32, temp2);
         
       } else if (termoprofily == 3)
        {
	        //термопрофиль User 2 шаг 0
          EEPROM.put(1300, sec);
          EEPROM.put(1305, temp1);
          EEPROM.put(1310, temp2);
        } else if (termoprofily == 4)
          { 
          //термопрофиль User 3 шаг 0
            EEPROM.put(1775, sec);
            EEPROM.put(1780, temp1);
            EEPROM.put(1785, temp2);
          }else if (termoprofily == 5)
          { 
          //термопрофиль User 4 шаг 0
            EEPROM.put(2250, sec);
            EEPROM.put(2255, temp1);
            EEPROM.put(2260, temp2);
          }else if (termoprofily == 6)
          { 
          //термопрофиль User 5 шаг 0
            EEPROM.put(2725, sec);
            EEPROM.put(2730, temp1);
            EEPROM.put(2735, temp2);
          }
    } else if(shag == 1)
    {
       if (termoprofily == 2)
       {
         
           EEPROM.put(37, sec);
           EEPROM.put(42, temp1);
           EEPROM.put(47, temp2);           
         
      
       } else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 1
          EEPROM.put(1315, sec);
          EEPROM.put(1320, temp1);
          EEPROM.put(1325, temp2);
        } else if (termoprofily == 4)
          { 
            //термопрофиль User 3 шаг 1
            EEPROM.put(1790, sec);
            EEPROM.put(1795, temp1);
            EEPROM.put(1800, temp2);
          }else if (termoprofily == 5)
          { 
            //термопрофиль User 4 шаг 1
            EEPROM.put(2265, sec);
            EEPROM.put(2270, temp1);
            EEPROM.put(2275, temp2);
          }else if (termoprofily == 6)
          { 
            //термопрофиль User 5 шаг 1
            EEPROM.put(2740, sec);
            EEPROM.put(2745, temp1);
            EEPROM.put(2750, temp2);
          }
    } else if(shag == 2)
    {
       if (termoprofily == 2)
       {
         
           EEPROM.put(52, sec);
           EEPROM.put(57, temp1);
           EEPROM.put(62, temp2);           
         
       }else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 2
          EEPROM.put(1330, sec);
          EEPROM.put(1335, temp1);
          EEPROM.put(1340, temp2);
        } else if (termoprofily == 4)
          { 
            //термопрофиль User 3 шаг 2
            EEPROM.put(1805, sec);
            EEPROM.put(1810, temp1);
            EEPROM.put(1815, temp2);
          }else if (termoprofily == 5)
          { 
            //термопрофиль User 4 шаг 2
            EEPROM.put(2280, sec);
            EEPROM.put(2285, temp1);
            EEPROM.put(2290, temp2);
          }else if (termoprofily == 6)
          { 
            //термопрофиль User 5 шаг 2
            EEPROM.put(2755, sec);
            EEPROM.put(2760, temp1);
            EEPROM.put(2765, temp2);
          }
    } else if(shag == 3)
    {
       if (termoprofily == 2)
       {
         
           EEPROM.put(67, sec);
           EEPROM.put(72, temp1);
           EEPROM.put(77, temp2);           
         
       }else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 3
          EEPROM.put(1345, sec);
          EEPROM.put(1350, temp1);
          EEPROM.put(1355, temp2);
        } else if (termoprofily == 4)
          { 
            //термопрофиль User 3 шаг 3
            EEPROM.put(1820, sec);
            EEPROM.put(1825, temp1);
            EEPROM.put(1830, temp2);
          }else if (termoprofily == 5)
          { 
            //термопрофиль User 4 шаг 3
            EEPROM.put(2295, sec);
            EEPROM.put(2300, temp1);
            EEPROM.put(2305, temp2);
          }else if (termoprofily == 6)
          { 
            //термопрофиль User 5 шаг 3
            EEPROM.put(2770, sec);
            EEPROM.put(2775, temp1);
            EEPROM.put(2780, temp2);
          }
    } else if(shag == 4)
    {
       if (termoprofily == 2)
       {
         
           EEPROM.put(82, sec);
           EEPROM.put(87, temp1);
           EEPROM.put(92, temp2);           
         
       }else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 4
          EEPROM.put(1360, sec);
          EEPROM.put(1365, temp1);
          EEPROM.put(1370, temp2);
        } else if (termoprofily == 4)
          { 
            //термопрофиль User 3 шаг 4
            EEPROM.put(1835, sec);
            EEPROM.put(1840, temp1);
            EEPROM.put(1845, temp2);
          }else if (termoprofily == 5)
          { 
            //термопрофиль User 4 шаг 4
            EEPROM.put(2310, sec);
            EEPROM.put(2315, temp1);
            EEPROM.put(2320, temp2);
          }else if (termoprofily == 6)
          { 
            //термопрофиль User 5 шаг 4
            EEPROM.put(2785, sec);
            EEPROM.put(2790, temp1);
            EEPROM.put(2795, temp2);
          }
    } else if(shag == 5)
    {
       if (termoprofily == 2)
       {
        
           EEPROM.put(97, sec);
           EEPROM.put(102, temp1);
           EEPROM.put(107, temp2);           
         
       } else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 5
          EEPROM.put(1375, sec);
          EEPROM.put(1380, temp1);
          EEPROM.put(1385, temp2);
        } else if (termoprofily == 4)
          { 
            //термопрофиль User 3 шаг 5
            EEPROM.put(1850, sec);
            EEPROM.put(1855, temp1);
            EEPROM.put(1860, temp2);
          }else if (termoprofily == 5)
          { 
            //термопрофиль User 4 шаг 5
            EEPROM.put(2325, sec);
            EEPROM.put(2330, temp1);
            EEPROM.put(2335, temp2);
          }else if (termoprofily == 6)
          { 
            //термопрофиль User 5 шаг 5
            EEPROM.put(2800, sec);
            EEPROM.put(2805, temp1);
            EEPROM.put(2810, temp2);
          }
    } else if(shag == 6)
    {
       if (termoprofily == 2)
       {
         
            EEPROM.put(112, sec);
            EEPROM.put(117, temp1);
            EEPROM.put(122, temp2);           
         
       } else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 6
          EEPROM.put(1390, sec);
          EEPROM.put(1395, temp1);
          EEPROM.put(1400, temp2);
        } else if (termoprofily == 4)
          { 
            //термопрофиль User 3 шаг 6
            EEPROM.put(1865, sec);
            EEPROM.put(1870, temp1);
            EEPROM.put(1875, temp2);
          }else if (termoprofily == 5)
          { 
            //термопрофиль User 4 шаг 6
            EEPROM.put(2340, sec);
            EEPROM.put(2345, temp1);
            EEPROM.put(2350, temp2);
          }else if (termoprofily == 6)
          { 
            //термопрофиль User 5 шаг 6
            EEPROM.put(2815, sec);
            EEPROM.put(2820, temp1);
            EEPROM.put(2825, temp2);
          }
       
    } else if(shag == 7)
    {
       if (termoprofily == 2)
       {
         
            EEPROM.put(127, sec);
            EEPROM.put(132, temp1);
            EEPROM.put(137, temp2);           
         
       } else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 7
          EEPROM.put(1405, sec);
          EEPROM.put(1410, temp1);
          EEPROM.put(1415, temp2);
        } else if (termoprofily == 4)
          { 
            //термопрофиль User 3 шаг 7
            EEPROM.put(1880, sec);
            EEPROM.put(1885, temp1);
            EEPROM.put(1890, temp2);
          }else if (termoprofily == 5)
          { 
            //термопрофиль User 4 шаг 7
            EEPROM.put(2355, sec);
            EEPROM.put(2360, temp1);
            EEPROM.put(2365, temp2);
          }else if (termoprofily == 6)
          { 
            //термопрофиль User 5 шаг 7
            EEPROM.put(2830, sec);
            EEPROM.put(2835, temp1);
            EEPROM.put(2840, temp2);
          }
       
    } else if(shag == 8)
    {
       if (termoprofily == 2)
       {
         
            EEPROM.put(142, sec);
            EEPROM.put(147, temp1);
            EEPROM.put(152, temp2);           
         
       } else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 8
          EEPROM.put(1420, sec);
          EEPROM.put(1425, temp1);
          EEPROM.put(1430, temp2);
        } else if (termoprofily == 4)
          { 
            //термопрофиль User 3 шаг 8
            EEPROM.put(1895, sec);
            EEPROM.put(1900, temp1);
            EEPROM.put(1905, temp2);
          }else if (termoprofily == 5)
          { 
            //термопрофиль User 4 шаг 8
            EEPROM.put(2370, sec);
            EEPROM.put(2375, temp1);
            EEPROM.put(2380, temp2);
          }else if (termoprofily == 6)
          { 
            //термопрофиль User 5 шаг 8
            EEPROM.put(2845, sec);
            EEPROM.put(2850, temp1);
            EEPROM.put(2855, temp2);
          }
       
    } else if(shag == 9)
    {
       if (termoprofily == 2)
       {
      
            EEPROM.put(157, sec);
            EEPROM.put(162, temp1);
            EEPROM.put(167, temp2);           
         
       } else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 9
          EEPROM.put(1435, sec);
          EEPROM.put(1440, temp1);
          EEPROM.put(1445, temp2);
        } else if (termoprofily == 4)
          { 
            //термопрофиль User 3 шаг 9
            EEPROM.put(1910, sec);
            EEPROM.put(1915, temp1);
            EEPROM.put(1920, temp2);
          }else if (termoprofily == 5)
          { 
            //термопрофиль User 4 шаг 9
            EEPROM.put(2385, sec);
            EEPROM.put(2390, temp1);
            EEPROM.put(2395, temp2);
          }else if (termoprofily == 6)
          { 
            //термопрофиль User 5 шаг 9
            EEPROM.put(2860, sec);
            EEPROM.put(2865, temp1);
            EEPROM.put(2870, temp2);
          }
    } else if(shag == 10)
    {
        if (termoprofily == 2)
        {
          
            EEPROM.put(172, sec);
            EEPROM.put(177, temp1);
            EEPROM.put(182, temp2);           

        } else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 10
          EEPROM.put(1450, sec);
          EEPROM.put(1455, temp1);
          EEPROM.put(1460, temp2);
        } else if (termoprofily == 4)
          { 
            //термопрофиль User 3 шаг 10
            EEPROM.put(1925, sec);
            EEPROM.put(1930, temp1);
            EEPROM.put(1935, temp2);
          }else if (termoprofily == 5)
          { 
            //термопрофиль User 4 шаг 10
            EEPROM.put(2400, sec);
            EEPROM.put(2405, temp1);
            EEPROM.put(2410, temp2);
          }else if (termoprofily == 6)
          { 
            //термопрофиль User 5 шаг 10
            EEPROM.put(2875, sec);
            EEPROM.put(2880, temp1);
            EEPROM.put(2885, temp2);
          }
    } 
     
  } 
  if (incStr.indexOf("degreessave") >= 0) 
  { 
    if(shag == 0)
    {
       if (termoprofily == 2)
       {
         	//термопрофиль User 1 шаг 0
          EEPROM.put(3200, gradsecv);
          EEPROM.put(3205, gradsecn);
         
       } else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 0
          EEPROM.put(3310, gradsecv);
          EEPROM.put(3315, gradsecn);	        
        } else if (termoprofily == 4)
          { 
              //термопрофиль User 3 шаг 0
            EEPROM.put(3420, gradsecv);
            EEPROM.put(3425, gradsecn);          
          }else if (termoprofily == 5)
          { 
              //термопрофиль User 4 шаг 0
            EEPROM.put(3530, gradsecv);
            EEPROM.put(3535, gradsecn);          
          }else if (termoprofily == 6)
          { 
              //термопрофиль User 5 шаг 0
            EEPROM.put(3640, gradsecv);
            EEPROM.put(3645, gradsecn);          
          }
    } else if(shag == 1)
    {
      if (termoprofily == 2)
       {
          //термопрофиль User 1 шаг 1
        EEPROM.put(3210, gradsecv);
        EEPROM.put(3215, gradsecn);         
         
       } else if (termoprofily == 3)
        {
            //термопрофиль User 2 шаг 1
          EEPROM.put(3320, gradsecv);
          EEPROM.put(3325, gradsecn);	        
        } else if (termoprofily == 4)
          { 
              //термопрофиль User 3 шаг 1
            EEPROM.put(3430, gradsecv);
            EEPROM.put(3435, gradsecn);          
          }else if (termoprofily == 5)
          { 
              //термопрофиль User 4 шаг 1
            EEPROM.put(3540, gradsecv);
            EEPROM.put(3545, gradsecn);          
          }else if (termoprofily == 6)
          { 
              //термопрофиль User 5 шаг 1
            EEPROM.put(3650, gradsecv);
            EEPROM.put(3655, gradsecn);          
          }
    }else if(shag == 2)
    {
      if (termoprofily == 2)
       {
          //термопрофиль User 1 шаг 2
        EEPROM.put(3220, gradsecv);
        EEPROM.put(3225, gradsecn);         
         
       } else if (termoprofily == 3)
        {
            //термопрофиль User 2 шаг 2
          EEPROM.put(3330, gradsecv);
          EEPROM.put(3335, gradsecn);	        
        } else if (termoprofily == 4)
          { 
              //термопрофиль User 3 шаг 2
            EEPROM.put(3440, gradsecv);
            EEPROM.put(3445, gradsecn);          
          }else if (termoprofily == 5)
          { 
              //термопрофиль User 4 шаг 2
            EEPROM.put(3550, gradsecv);
            EEPROM.put(3555, gradsecn);          
          }else if (termoprofily == 6)
          { 
              //термопрофиль User 5 шаг 2
            EEPROM.put(3660, gradsecv);
            EEPROM.put(3665, gradsecn);          
          }
    }else if(shag == 3)
    {
      if (termoprofily == 2)
       {
          //термопрофиль User 1 шаг 3
          EEPROM.put(3230, gradsecv);
          EEPROM.put(3235, gradsecn);         
         
       } else if (termoprofily == 3)
        {
            //термопрофиль User 2 шаг 3
          EEPROM.put(3340, gradsecv);
          EEPROM.put(3345, gradsecn);	        
        } else if (termoprofily == 4)
          { 
              //термопрофиль User 3 шаг 3
            EEPROM.put(3450, gradsecv);
            EEPROM.put(3455, gradsecn);          
          }else if (termoprofily == 5)
          { 
              //термопрофиль User 4 шаг 3
            EEPROM.put(3560, gradsecv);
            EEPROM.put(3565, gradsecn);          
          }else if (termoprofily == 6)
          { 
              //термопрофиль User 5 шаг 3
            EEPROM.put(3670, gradsecv);
            EEPROM.put(3675, gradsecn);          
          }
    }else if(shag == 4)
    {
      if (termoprofily == 2)
       {
        //термопрофиль User 1 шаг 4
        EEPROM.put(3240, gradsecv);
        EEPROM.put(3245, gradsecn);         
         
       } else if (termoprofily == 3)
        {
            //термопрофиль User 2 шаг 4
          EEPROM.put(3350, gradsecv);
          EEPROM.put(3355, gradsecn);	        
        } else if (termoprofily == 4)
          { 
              //термопрофиль User 3 шаг 4
            EEPROM.put(3460, gradsecv);
            EEPROM.put(3465, gradsecn);          
          }else if (termoprofily == 5)
          { 
              //термопрофиль User 4 шаг 4
            EEPROM.put(3570, gradsecv);
            EEPROM.put(3575, gradsecn);          
          }else if (termoprofily == 6)
          { 
              //термопрофиль User 5 шаг 4
            EEPROM.put(3680, gradsecv);
            EEPROM.put(3685, gradsecn);          
          }
    }else if(shag == 5)
    {
      if (termoprofily == 2)
       {
          //термопрофиль User 1 шаг 5
          EEPROM.put(3250, gradsecv);
          EEPROM.put(3255, gradsecn);         
         
       } else if (termoprofily == 3)
        {
            //термопрофиль User 2 шаг 5
          EEPROM.put(3360, gradsecv);
          EEPROM.put(3365, gradsecn);	        
        } else if (termoprofily == 4)
          { 
              //термопрофиль User 3 шаг 5
            EEPROM.put(3470, gradsecv);
            EEPROM.put(3475, gradsecn);          
          }else if (termoprofily == 5)
          { 
            //термопрофиль User 4 шаг 5
            EEPROM.put(3580, gradsecv);
            EEPROM.put(3585, gradsecn);          
          }else if (termoprofily == 6)
          { 
              //термопрофиль User 5 шаг 5
            EEPROM.put(3690, gradsecv);
            EEPROM.put(3695, gradsecn);          
          }
    }else if(shag == 6)
    {
      if (termoprofily == 2)
       {
          //термопрофиль User 1 шаг 6
          EEPROM.put(3260, gradsecv);
          EEPROM.put(3265, gradsecn);          
         
       } else if (termoprofily == 3)
        {
            //термопрофиль User 2 шаг 6
          EEPROM.put(3370, gradsecv);
          EEPROM.put(3375, gradsecn); 	        
        } else if (termoprofily == 4)
          { 
              //термопрофиль User 3 шаг 6
            EEPROM.put(3480, gradsecv);
            EEPROM.put(3485, gradsecn);             
          }else if (termoprofily == 5)
          { 
              //термопрофиль User 4 шаг 6
            EEPROM.put(3590, gradsecv);
            EEPROM.put(3595, gradsecn);            
          }else if (termoprofily == 6)
          { 
              //термопрофиль User 5 шаг 6
            EEPROM.put(3700, gradsecv);
            EEPROM.put(3705, gradsecn);          
          }
    }else if(shag == 7)
    {
      if (termoprofily == 2)
       {
          //термопрофиль User 1 шаг 7
          EEPROM.put(3270, gradsecv);
          EEPROM.put(3275, gradsecn);          
         
       } else if (termoprofily == 3)
        {
            //термопрофиль User 2 шаг 7
          EEPROM.put(3380, gradsecv);
          EEPROM.put(3385, gradsecn); 	        
        } else if (termoprofily == 4)
          { 
              //термопрофиль User 3 шаг 7
            EEPROM.put(3490, gradsecv);
            EEPROM.put(3495, gradsecn);          
          }else if (termoprofily == 5)
          { 
              //термопрофиль User 4 шаг 7
            EEPROM.put(3600, gradsecv);
            EEPROM.put(3605, gradsecn);          
          }else if (termoprofily == 6)
          { 
              //термопрофиль User 5 шаг 7
            EEPROM.put(3710, gradsecv);
            EEPROM.put(3715, gradsecn);           
          }
    }else if(shag == 8)
    {
      if (termoprofily == 2)
       {
          //термопрофиль User 1 шаг 8
          EEPROM.put(3280, gradsecv);
          EEPROM.put(3285, gradsecn);         
         
       } else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 8
          EEPROM.put(3390, gradsecv);
          EEPROM.put(3395, gradsecn);	        
        } else if (termoprofily == 4)
          { 
              //термопрофиль User 3 шаг 8
            EEPROM.put(3500, gradsecv);
            EEPROM.put(3505, gradsecn);          
          }else if (termoprofily == 5)
          { 
              //термопрофиль User 4 шаг 8
            EEPROM.put(3610, gradsecv);
            EEPROM.put(3615, gradsecn);          
          }else if (termoprofily == 6)
          { 
              //термопрофиль User 5 шаг 8
            EEPROM.put(3720, gradsecv);
            EEPROM.put(3725, gradsecn);          
          }
    }else if(shag == 9)
    {
      if (termoprofily == 2)
       {
          //термопрофиль User 1 шаг 9
          EEPROM.put(3290, gradsecv);
          EEPROM.put(3295, gradsecn);         
         
       } else if (termoprofily == 3)
        {
            //термопрофиль User 2 шаг 9
          EEPROM.put(3400, gradsecv);
          EEPROM.put(3405, gradsecn);	        
        } else if (termoprofily == 4)
          { 
              //термопрофиль User 3 шаг 9
            EEPROM.put(3510, gradsecv);
            EEPROM.put(3515, gradsecn);          
          }else if (termoprofily == 5)
          { 
              //термопрофиль User 4 шаг 9
            EEPROM.put(3620, gradsecv);
            EEPROM.put(3625, gradsecn);          
          }else if (termoprofily == 6)
          { 
              //термопрофиль User 5 шаг 9
            EEPROM.put(3730, gradsecv);
            EEPROM.put(3735, gradsecn);          
          }
    }else if(shag == 10)
    {
      if (termoprofily == 2)
       {
          //термопрофиль User 1 шаг 10
          EEPROM.put(3300, gradsecv);
          EEPROM.put(3305, gradsecn);          
         
       } else if (termoprofily == 3)
        {
          //термопрофиль User 2 шаг 10
          EEPROM.put(3410, gradsecv);
          EEPROM.put(3415, gradsecn);	        
        } else if (termoprofily == 4)
          { 
            //термопрофиль User 3 шаг 10
            EEPROM.put(3520, gradsecv);
            EEPROM.put(3525, gradsecn);          
          }else if (termoprofily == 5)
          { 
              //термопрофиль User 4 шаг 10
            EEPROM.put(3630, gradsecv);
            EEPROM.put(3635, gradsecn);          
          }else if (termoprofily == 6)
          { 
              //термопрофиль User 5 шаг 10
            EEPROM.put(3740, gradsecv);
            EEPROM.put(3745, gradsecn);          
          }
    }
  }
  if (incStr.indexOf("opovsave") >= 0) 
  { 
    if (termoprofily == 2)
    {
      // Оповещение температуры User 1
      EEPROM.put(3856,setopov1);
      EEPROM.put(3861,setopov2);
      EEPROM.put(3866,setopov3);
      EEPROM.put(3871,setopov4);
    }else if (termoprofily == 3)
    {
      // Оповещение температуры User 2
      EEPROM.put(3876,setopov1);
      EEPROM.put(3881,setopov2);
      EEPROM.put(3886,setopov3);
      EEPROM.put(3891,setopov4);
    }else if (termoprofily == 4)
    {
      // Оповещение температуры User 3
      EEPROM.put(3896,setopov1);
      EEPROM.put(3901,setopov2);
      EEPROM.put(3906,setopov3);
      EEPROM.put(3911,setopov4);
    }else if (termoprofily == 5)
    {
      // Оповещение температуры User 4
      EEPROM.put(3916,setopov1);
      EEPROM.put(3921,setopov2);
      EEPROM.put(3926,setopov3);
      EEPROM.put(3931,setopov4);
    }else if (termoprofily == 6)
    {
      // Оповещение температуры User 5
      EEPROM.put(3936,setopov1);
      EEPROM.put(3941,setopov2);
      EEPROM.put(3946,setopov3);
      EEPROM.put(3951,setopov4);
    }
  }
  if (incStr.indexOf("hisusave") >= 0) 
  {
     EEPROM.put(3956,hup);
     EEPROM.put(3961,kup);
     EEPROM.put(3966,dtup);
  }
  if (incStr.indexOf("hisdsave") >= 0) 
  {
     EEPROM.put(3971,hdown);
     EEPROM.put(3976,kdown);
     EEPROM.put(3981,dtdown);
  }

  if (incStr.indexOf("b4") >= 0) 
  {
    if (shag < 10)
    {
      shag++;
      outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
      if(shag == 0)
      {
          reley_n1=0;
          reley_v=0;
          analogWrite(nigniy_1, 0);
          analogWrite(verhniy_1, 0);
          
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec

       if (termoprofily == 0)
       {

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
         
        if (reley_n1==1)
        {
          termoprofily1_9 = 1;
        } else
        {
          termoprofily1_9 = 0;
          
        }
          reley_n1=0;
          reley_v=0;
          analogWrite(nigniy_1, 0);
          analogWrite(verhniy_1, 0);
          

      } else if (termoprofily == 1)
      {
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
         
        if (reley_n1==1)
        {
          termoprofily1_9 = 1;
        } else
        {
          termoprofily1_9 = 0;
          
        }
          reley_n1=0;
          reley_v=0;
          analogWrite(nigniy_1, 0);
          analogWrite(verhniy_1, 0);


      } else if (termoprofily == 2)
      {
          EEPROM.get(227, dtv);	   
          EEPROM.get(232, dtn); 
          delay(10);
          EEPROM.get(22, sec);
          EEPROM.get(27, temp1);
          EEPROM.get(32, temp2);
          delay(10);
          //термопрофиль User 1 шаг 0
          EEPROM.get(3200, gradsecv);
          EEPROM.get(3205, gradsecn); 
          delay(10);
          EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
  delay(10);	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
  delay(10);  
          //термопрофиль User 1 шаг 0
          EEPROM.get(3746, pwmv);
          EEPROM.get(3747, pwmn); 
           // Оповещение температуры User 1
          EEPROM.get(3856,setopov1);
          EEPROM.get(3861,setopov2);
          EEPROM.get(3866,setopov3);
          EEPROM.get(3871,setopov4);        
         // shag = 0;
         //sec=0;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          Dtv = dtv;
          String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
          SendData("t57.txt", t57);
          Dtn = dtn;
          String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
          SendData("t58.txt", t58);
          GradSecv = gradsecv;
          String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
          SendData("tgcs3.txt", tgcs3);
          GradSecn = gradsecn;
          String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
          SendData("tgcs4.txt", tgcs4);
          Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
          outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
          pwmust1 = pwmv;
          outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
          pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         
        if (reley_n1==1)
        {
          termoprofily1_9 = 1;
        } else
        {
          termoprofily1_9 = 0;
          
        }
          reley_n1=0;
          reley_v=0;
          analogWrite(nigniy_1, 0);
          analogWrite(verhniy_1, 0); 


       }  else if (termoprofily == 3)
      {
          //термопрофиль User 2
	        //delay(10);
          EEPROM.get(1465, dtv); 
          EEPROM.get(1470, dtn); 
          delay(10);
          //термопрофиль User 2 шаг 0
          EEPROM.get(1300, sec);
          EEPROM.get(1305, temp1);
          EEPROM.get(1310, temp2);
          delay(10);
          //термопрофиль User 2 шаг 0
          EEPROM.get(3310, gradsecv);
          EEPROM.get(3315, gradsecn);
          delay(10);
            EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
  delay(10);	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
  delay(10);
          //термопрофиль User 2 шаг 0
          EEPROM.get(3768, pwmv);
          EEPROM.get(3769, pwmn);
          // Оповещение температуры User 2
          EEPROM.get(3876,setopov1);
          EEPROM.get(3881,setopov2);
          EEPROM.get(3886,setopov3);
          EEPROM.get(3891,setopov4);
         // shag = 0;
         //sec=0;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          Dtv = dtv;
          String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
          SendData("t57.txt", t57);
          Dtn = dtn;
          String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
          SendData("t58.txt", t58);
          GradSecv = gradsecv;
          String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
          SendData("tgcs3.txt", tgcs3);
          GradSecn = gradsecn;
          String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
          SendData("tgcs4.txt", tgcs4);
          Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
          outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
          pwmust1 = pwmv;
          outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
          pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         
        if (reley_n1==1)
        {
          termoprofily1_9 = 1;
        } else
        {
          termoprofily1_9 = 0;
          
        }
          reley_n1=0;
          reley_v=0;
          analogWrite(nigniy_1, 0);
          analogWrite(verhniy_1, 0); 

                
       } else if (termoprofily == 4)
      {
        //термопрофиль User 3
        //delay(10);
          EEPROM.get(1940, dtv); 
          EEPROM.get(1945, dtn);
          delay(10);
        //термопрофиль User 3 шаг 0
          EEPROM.get(1775, sec);
          EEPROM.get(1780, temp1);
          EEPROM.get(1785, temp2);
          delay(10);
            //термопрофиль User 3 шаг 0
          EEPROM.get(3420, gradsecv);
          EEPROM.get(3425, gradsecn); 
          delay(10);
            EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
  delay(10);	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
  delay(10);
          //термопрофиль User 3 шаг 0
          EEPROM.get(3790, pwmv);
          EEPROM.get(3791, pwmn); 
           // Оповещение температуры User 3
          EEPROM.get(3896,setopov1);
          EEPROM.get(3901,setopov2);
          EEPROM.get(3906,setopov3);
          EEPROM.get(3911,setopov4);       
         // shag = 0;
         //sec=0;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          Dtv = dtv;
          String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
          SendData("t57.txt", t57);
          Dtn = dtn;
          String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
          SendData("t58.txt", t58);
          GradSecv = gradsecv;
          String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
          SendData("tgcs3.txt", tgcs3);
          GradSecn = gradsecn;
          String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
          SendData("tgcs4.txt", tgcs4);
          Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        
          outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
          pwmust1 = pwmv;
          outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
          pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         
        if (reley_n1==1)
        {
          termoprofily1_9 = 1;
        } else
        {
          termoprofily1_9 = 0;
          
        }
          reley_n1=0;
          reley_v=0;
          analogWrite(nigniy_1, 0);
          analogWrite(verhniy_1, 0); 
                  
       } else if (termoprofily == 5)
      {
        //термопрофиль User 4
        //delay(10);
          EEPROM.get(2415, dtv); 
          EEPROM.get(2420, dtn); 
          delay(10);
        //термопрофиль User 4 шаг 0
          EEPROM.get(2250, sec);
          EEPROM.get(2255, temp1);
          EEPROM.get(2260, temp2);
          delay(10);
            //термопрофиль User 4 шаг 0
          EEPROM.get(3530, gradsecv);
          EEPROM.get(3535, gradsecn); 
          delay(10);
            EEPROM.get(3956,hup);
  EEPROM.get(3961,kup);
  EEPROM.get(3966,dtup);
  delay(10);	
  EEPROM.get(3971,hdown);
  EEPROM.get(3976,kdown);
  EEPROM.get(3981,dtdown);
  delay(10);
           //термопрофиль User 4 шаг 0
          EEPROM.get(3812, pwmv);
          EEPROM.get(3813, pwmn); 
          // Оповещение температуры User 4
          EEPROM.get(3916,setopov1);
          EEPROM.get(3921,setopov2);
          EEPROM.get(3926,setopov3);
          EEPROM.get(3931,setopov4);       
         // shag = 0;
         //sec=0;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          Dtv = dtv;
          String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
          SendData("t57.txt", t57);
          Dtn = dtn;
          String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
          SendData("t58.txt", t58);
          GradSecv = gradsecv;
          String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
          SendData("tgcs3.txt", tgcs3);
          GradSecn = gradsecn;
          String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
          SendData("tgcs4.txt", tgcs4);
          Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        
          outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
          pwmust1 = pwmv;
          outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
          pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         
         if (reley_n1==1)
        {
          termoprofily1_9 = 1;
        } else
        {
          termoprofily1_9 = 0;
          
        }
          reley_n1=0;
          reley_v=0;
          analogWrite(nigniy_1, 0);
          analogWrite(verhniy_1, 0);


       } else if (termoprofily == 6)
      {
        //термопрофиль User 5
        //delay(10);
          EEPROM.get(2890, dtv); 
          EEPROM.get(2895, dtn); 
          delay(10);
        //термопрофиль User 5 шаг 0
          EEPROM.get(2725, sec);
          EEPROM.get(2730, temp1);
          EEPROM.get(2735, temp2);
          delay(10);
          //термопрофиль User 5 шаг 0
          EEPROM.get(3640, gradsecv);
          EEPROM.get(3645, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 5 шаг 0
          EEPROM.get(3834, pwmv);
          EEPROM.get(3835, pwmn);
          // Оповещение температуры User 5
          EEPROM.get(3936,setopov1);
          EEPROM.get(3941,setopov2);
          EEPROM.get(3946,setopov3);
          EEPROM.get(3951,setopov4);
         // shag = 0;
         //sec=0;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          Dtv = dtv;
          String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
          SendData("t57.txt", t57);
          Dtn = dtn;
          String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
          SendData("t58.txt", t58);
          GradSecv = gradsecv;
          String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
          SendData("tgcs3.txt", tgcs3);
          GradSecn = gradsecn;
          String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
          SendData("tgcs4.txt", tgcs4);
          Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        
          outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
          pwmust1 = pwmv;
          outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
          pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 50 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         
        if (reley_n1==1)
        {
          termoprofily1_9 = 1;
        } else
        {
          termoprofily1_9 = 0;
          
        }
          reley_n1=0;
          reley_v=0;
          analogWrite(nigniy_1, 0);
          analogWrite(verhniy_1, 0); 


       } 

     } else if(shag == 1)
     {
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0)
       {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      } else if (termoprofily == 1)
      {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }        
      } else if (termoprofily == 2)
      {
         EEPROM.get(7, kpv);
         EEPROM.get(12, kiv);
         EEPROM.get(17, kdv);
         delay(10);
         //shag == 1 // termoprofily 2 нижний нагреватель
         EEPROM.get(212, kpn);
         EEPROM.get(217, kin);
         EEPROM.get(222, kdn); 
         delay(10);
         EEPROM.get(37, sec);
         EEPROM.get(42, temp1);
         EEPROM.get(47, temp2);
         delay(10);
         //термопрофиль User 1 шаг 1
          EEPROM.get(3210, gradsecv);
          EEPROM.get(3215, gradsecn); 
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 1 шаг 1
          EEPROM.get(3748, pwmv);
          EEPROM.get(3749, pwmn);        
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }else if (termoprofily == 3)
      {
          //shag == 1 
          EEPROM.get(1475, kpv);
          EEPROM.get(1480, kiv);
          EEPROM.get(1485, kdv);
         delay(10);
         //shag == 1
          EEPROM.get(1625, kpn);
          EEPROM.get(1630, kin);
          EEPROM.get(1635, kdn); 
         delay(10);
         //термопрофиль User 2 шаг 1
         EEPROM.get(1315, sec);
         EEPROM.get(1320, temp1);
         EEPROM.get(1325, temp2);
         delay(10);
            //термопрофиль User 2 шаг 1
          EEPROM.get(3320, gradsecv);
          EEPROM.get(3325, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 2 шаг 1
          EEPROM.get(3770, pwmv);
          EEPROM.get(3771, pwmn);         
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }else if (termoprofily == 4)
      {
        //shag == 1 
          EEPROM.get(1950, kpv);
          EEPROM.get(1955, kiv);
          EEPROM.get(1960, kdv);
         delay(10);
        //shag == 1
          EEPROM.get(2100, kpn);
          EEPROM.get(2105, kin);
          EEPROM.get(2110, kdn); 
         delay(10);
          //термопрофиль User 3 шаг 1
          EEPROM.get(1790, sec);
          EEPROM.get(1795, temp1);
          EEPROM.get(1800, temp2);
          delay(10);
            //термопрофиль User 3 шаг 1
          EEPROM.get(3430, gradsecv);
          EEPROM.get(3435, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 3 шаг 1
          EEPROM.get(3792, pwmv);
          EEPROM.get(3793, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }else if (termoprofily == 5)
      {
        //shag == 1 
          EEPROM.get(2425, kpv);
          EEPROM.get(2430, kiv);
          EEPROM.get(2435, kdv);
         delay(10);
        //shag == 1
          EEPROM.get(2575, kpn);
          EEPROM.get(2580, kin);
          EEPROM.get(2585, kdn); 
         delay(10);
          //термопрофиль User 4 шаг 1
          EEPROM.get(2265, sec);
          EEPROM.get(2270, temp1);
          EEPROM.get(2275, temp2);
          delay(10);
            //термопрофиль User 4 шаг 1
          EEPROM.get(3540, gradsecv);
          EEPROM.get(3545, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
           //термопрофиль User 4 шаг 1
          EEPROM.get(3814, pwmv);
          EEPROM.get(3815, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }else if (termoprofily == 6)
      {
          //shag == 1 
          EEPROM.get(2900, kpv);
          EEPROM.get(2905, kiv);
          EEPROM.get(2910, kdv);
         delay(10);
          //shag == 1
          EEPROM.get(3050, kpn);
          EEPROM.get(3055, kin);
          EEPROM.get(3060, kdn);
         delay(10);
          //термопрофиль User 5 шаг 1
          EEPROM.get(2740, sec);
          EEPROM.get(2745, temp1);
          EEPROM.get(2750, temp2);
          delay(10);
            //термопрофиль User 5 шаг 1
          EEPROM.get(3650, gradsecv);
          EEPROM.get(3655, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 5 шаг 1
          EEPROM.get(3836, pwmv);
          EEPROM.get(3837, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }
    } else if(shag == 2)
    {
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0)
       {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      } else if (termoprofily == 1)
      {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }        
      } else if (termoprofily == 2)
      {
         //shag == 2 
         EEPROM.get(1030, kpv);
         EEPROM.get(1035, kiv);
         EEPROM.get(1040, kdv);
         delay(10);
         //shag == 2
         EEPROM.get(1165, kpn);
         EEPROM.get(1170, kin);
         EEPROM.get(1175, kdn);
         delay(10);
         EEPROM.get(52, sec);
         EEPROM.get(57, temp1);
         EEPROM.get(62, temp2);
         delay(10);
          //термопрофиль User 1 шаг 2
          EEPROM.get(3220, gradsecv);
          EEPROM.get(3225, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
           //термопрофиль User 1 шаг 2
          EEPROM.get(3750, pwmv);
          EEPROM.get(3751, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }else if (termoprofily == 3)
      {
      	  //shag == 2 
          EEPROM.get(1490, kpv);
          EEPROM.get(1495, kiv);
          EEPROM.get(1500, kdv);
         delay(10);
         //shag == 2
          EEPROM.get(1640, kpn);
          EEPROM.get(1645, kin);
          EEPROM.get(1650, kdn);
         delay(10);
          //термопрофиль User 2 шаг 2
          EEPROM.get(1330, sec);
          EEPROM.get(1335, temp1);
          EEPROM.get(1340, temp2);
          delay(10);
            //термопрофиль User 2 шаг 2
          EEPROM.get(3330, gradsecv);
          EEPROM.get(3335, gradsecn); 
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 2 шаг 2
          EEPROM.get(3772, pwmv);
          EEPROM.get(3773, pwmn);         
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }else if (termoprofily == 4)
      {
        //shag == 2 
          EEPROM.get(1965, kpv);
          EEPROM.get(1970, kiv);
          EEPROM.get(1975, kdv);
         delay(10);
          //shag == 2
          EEPROM.get(2115, kpn);
          EEPROM.get(2120, kin);
          EEPROM.get(2125, kdn);
         delay(10);
          //термопрофиль User 3 шаг 2
          EEPROM.get(1805, sec);
          EEPROM.get(1810, temp1);
          EEPROM.get(1815, temp2);
          delay(10);
            //термопрофиль User 3 шаг 2
          EEPROM.get(3440, gradsecv);
          EEPROM.get(3445, gradsecn);         
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 3 шаг 2
          EEPROM.get(3794, pwmv);
          EEPROM.get(3795, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }else if (termoprofily == 5)
      {
        //shag == 2 
          EEPROM.get(2440, kpv);
          EEPROM.get(2445, kiv);
          EEPROM.get(2450, kdv);
         delay(10);
          //shag == 2
          EEPROM.get(2590, kpn);
          EEPROM.get(2595, kin);
          EEPROM.get(2600, kdn);
         delay(10);
          //термопрофиль User 4 шаг 2
          EEPROM.get(2280, sec);
          EEPROM.get(2285, temp1);
          EEPROM.get(2290, temp2);
          delay(10);
            //термопрофиль User 4 шаг 2
          EEPROM.get(3550, gradsecv);
          EEPROM.get(3555, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 4 шаг 2
          EEPROM.get(3816, pwmv);
          EEPROM.get(3817, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }else if (termoprofily == 6)
      {
        //shag == 2 
          EEPROM.get(2915, kpv);
          EEPROM.get(2920, kiv);
          EEPROM.get(2925, kdv);
         delay(10);
          //shag == 2
          EEPROM.get(3065, kpn);
          EEPROM.get(3070, kin);
          EEPROM.get(3075, kdn);     
         delay(10);
          //термопрофиль User 5 шаг 2
          EEPROM.get(2755, sec);
          EEPROM.get(2760, temp1);
          EEPROM.get(2765, temp2);
          delay(10);
            //термопрофиль User 5 шаг 2
          EEPROM.get(3660, gradsecv);
          EEPROM.get(3665, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 5 шаг 2
          EEPROM.get(3838, pwmv);
          EEPROM.get(3839, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }
    } else if(shag == 3)
    {
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0)
       {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1)
      {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }      
      } else if (termoprofily == 2)
      {
        //shag == 3
        EEPROM.get(1045, kpv);
        EEPROM.get(1050, kiv);
        EEPROM.get(1055, kdv);
        delay(10);
        //shag == 3
        EEPROM.get(1180, kpn);
        EEPROM.get(1185, kin);
        EEPROM.get(1190, kdn);
        delay(10);
        EEPROM.get(67, sec);
        EEPROM.get(72, temp1);
        EEPROM.get(77, temp2);
        delay(10);
        //термопрофиль User 1 шаг 3
        EEPROM.get(3230, gradsecv);
        EEPROM.get(3235, gradsecn); 
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 1 шаг 3
        EEPROM.get(3752, pwmv);
        EEPROM.get(3753, pwmn);       
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 120; // Нижний нагреватель Бессвинецовый выбрано 120 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        } else if (termoprofily == 3)
      {
         //shag == 3
        EEPROM.get(1505, kpv);
        EEPROM.get(1510, kiv);
        EEPROM.get(1515, kdv);
        delay(10);
        //shag == 3
        EEPROM.get(1655, kpn);
        EEPROM.get(1660, kin);
        EEPROM.get(1665, kdn); 
        delay(10);
        //термопрофиль User 2 шаг 3
        EEPROM.get(1345, sec);
        EEPROM.get(1350, temp1);
        EEPROM.get(1355, temp2);
        delay(10);
          //термопрофиль User 2 шаг 3
        EEPROM.get(3340, gradsecv);
        EEPROM.get(3345, gradsecn);
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 2 шаг 3
        EEPROM.get(3774, pwmv);
        EEPROM.get(3775, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 120; // Нижний нагреватель Бессвинецовый выбрано 120 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }else if (termoprofily == 4)
      {
         //shag == 3
        EEPROM.get(1980, kpv);
        EEPROM.get(1985, kiv);
        EEPROM.get(1990, kdv);
        delay(10);
        //shag == 3
        EEPROM.get(2130, kpn);
        EEPROM.get(2135, kin);
        EEPROM.get(2140, kdn);
        delay(10);
        //термопрофиль User 3 шаг 3
        EEPROM.get(1820, sec);
        EEPROM.get(1825, temp1);
        EEPROM.get(1830, temp2);
        delay(10);
                //термопрофиль User 3 шаг 3
        EEPROM.get(3450, gradsecv);
        EEPROM.get(3455, gradsecn);
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 3 шаг 3
        EEPROM.get(3796, pwmv);
        EEPROM.get(3797, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 120; // Нижний нагреватель Бессвинецовый выбрано 120 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }else if (termoprofily == 5)
      {
         //shag == 3
          EEPROM.get(2455, kpv);
          EEPROM.get(2460, kiv);
          EEPROM.get(2465, kdv);
        delay(10);
        //shag == 3
          EEPROM.get(2605, kpn);
          EEPROM.get(2610, kin);
          EEPROM.get(2615, kdn);
        delay(10);
          //термопрофиль User 4 шаг 3
          EEPROM.get(2295, sec);
          EEPROM.get(2300, temp1);
          EEPROM.get(2305, temp2);
          delay(10);
            //термопрофиль User 4 шаг 3
          EEPROM.get(3560, gradsecv);
          EEPROM.get(3565, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 4 шаг 3
          EEPROM.get(3818, pwmv);
          EEPROM.get(3819, pwmn);          
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 120; // Нижний нагреватель Бессвинецовый выбрано 120 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }else if (termoprofily == 6)
      {
          //shag == 3
          EEPROM.get(2930, kpv);
          EEPROM.get(2935, kiv);
          EEPROM.get(2940, kdv);
         delay(10);
          //shag == 3
          EEPROM.get(3080, kpn);
          EEPROM.get(3085, kin);
          EEPROM.get(3090, kdn);     
         delay(10);
          //термопрофиль User 5 шаг 3
          EEPROM.get(2770, sec);
          EEPROM.get(2775, temp1);
          EEPROM.get(2780, temp2); 
          delay(10);
            //термопрофиль User 5 шаг 3
          EEPROM.get(3670, gradsecv);
          EEPROM.get(3675, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 5 шаг 3
          EEPROM.get(3840, pwmv);
          EEPROM.get(3841, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }
    } else if(shag == 4)
    {
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0)
       {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1)
      {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }       
      } else if (termoprofily == 2)
      {
         //shag == 4
         EEPROM.get(1060, kpv);
         EEPROM.get(1065, kiv);
         EEPROM.get(1070, kdv);
         delay(10);
         //shag == 4
         EEPROM.get(1195, kpn);
         EEPROM.get(1200, kin);
         EEPROM.get(1205, kdn);
         delay(10);
         EEPROM.get(82, sec);
         EEPROM.get(87, temp1);
         EEPROM.get(92, temp2);
         delay(10);
         //термопрофиль User 1 шаг 4
         EEPROM.get(3240, gradsecv);
         EEPROM.get(3245, gradsecn);
         delay(10);
         EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
         //термопрофиль User 1 шаг 4
         EEPROM.get(3754, pwmv);
         EEPROM.get(3755, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 140; // Нижний нагреватель Бессвинецовый выбрано 140 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }else if (termoprofily == 3)
      {
          //shag == 4
          EEPROM.get(1520, kpv);
          EEPROM.get(1525, kiv);
          EEPROM.get(1530, kdv);
         delay(10);
        //shag == 4
        EEPROM.get(1670, kpn);
        EEPROM.get(1675, kin);
        EEPROM.get(1680, kdn);
         delay(10);
        //термопрофиль User 2 шаг 4
        EEPROM.get(1360, sec);
        EEPROM.get(1365, temp1);
        EEPROM.get(1370, temp2);
        delay(10);
          //термопрофиль User 2 шаг 4
        EEPROM.get(3350, gradsecv);
        EEPROM.get(3355, gradsecn);
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 2 шаг 4
        EEPROM.get(3776, pwmv);
        EEPROM.get(3777, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 140; // Нижний нагреватель Бессвинецовый выбрано 140 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }else if (termoprofily == 4)
      {
          //shag == 4
        EEPROM.get(1995, kpv);
        EEPROM.get(2000, kiv);
        EEPROM.get(2005, kdv);
         delay(10);
        //shag == 4
        EEPROM.get(2145, kpn);
        EEPROM.get(2150, kin);
        EEPROM.get(2155, kdn);
         delay(10);
        //термопрофиль User 3 шаг 4
        EEPROM.get(1835, sec);
        EEPROM.get(1840, temp1);
        EEPROM.get(1845, temp2);
        delay(10);
            //термопрофиль User 3 шаг 4
          EEPROM.get(3460, gradsecv);
          EEPROM.get(3465, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 3 шаг 4
          EEPROM.get(3798, pwmv);
          EEPROM.get(3799, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 140; // Нижний нагреватель Бессвинецовый выбрано 140 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }else if (termoprofily == 5)
      {
          //shag == 4
        EEPROM.get(2470, kpv);
        EEPROM.get(2475, kiv);
        EEPROM.get(2480, kdv);
         delay(10);
        //shag == 4
        EEPROM.get(2620, kpn);
        EEPROM.get(2625, kin);
        EEPROM.get(2630, kdn);
         delay(10);
        //термопрофиль User 4 шаг 4
        EEPROM.get(2310, sec);
        EEPROM.get(2315, temp1);
        EEPROM.get(2320, temp2);
        delay(10);
          //термопрофиль User 4 шаг 4
        EEPROM.get(3570, gradsecv);
        EEPROM.get(3575, gradsecn);
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 4 шаг 4
        EEPROM.get(3820, pwmv);
        EEPROM.get(3821, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 140; // Нижний нагреватель Бессвинецовый выбрано 140 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }else if (termoprofily == 6)
      {
          //shag == 4
          EEPROM.get(2945, kpv);
          EEPROM.get(2950, kiv);
          EEPROM.get(2955, kdv);
         delay(10);
          //shag == 4
          EEPROM.get(3095, kpn);
          EEPROM.get(3100, kin);
          EEPROM.get(3105, kdn);     
         delay(10);
          //термопрофиль User 5 шаг 4
          EEPROM.get(2785, sec);
          EEPROM.get(2790, temp1);
          EEPROM.get(2795, temp2);
          delay(10);
          //термопрофиль User 5 шаг 4
          EEPROM.get(3680, gradsecv);
          EEPROM.get(3685, gradsecn); 
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 5 шаг 4
          EEPROM.get(3842, pwmv);
          EEPROM.get(3843, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }
    } else if(shag == 5)
    {
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0)
       {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1)
      {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }     
      } else if (termoprofily == 2)
      {
          //shag == 5
          EEPROM.get(1075, kpv);
          EEPROM.get(1080, kiv);
          EEPROM.get(1085, kdv);
          delay(10);
          //shag == 5
          EEPROM.get(1210, kpn);
          EEPROM.get(1215, kin);
          EEPROM.get(1220, kdn);
          delay(10);
          EEPROM.get(97, sec);
          EEPROM.get(102, temp1);
          EEPROM.get(107, temp2);
          delay(10);
            //термопрофиль User 1 шаг 5
          EEPROM.get(3250, gradsecv);
          EEPROM.get(3255, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 1 шаг 5
          EEPROM.get(3756, pwmv);
          EEPROM.get(3757, pwmn);          
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 3)
      {
          //shag == 5
          EEPROM.get(1535, kpv);
          EEPROM.get(1540, kiv);
          EEPROM.get(1545, kdv);
          delay(10);
          //shag == 5
          EEPROM.get(1685, kpn);
          EEPROM.get(1690, kin);
          EEPROM.get(1695, kdn);
          delay(10);
          //термопрофиль User 2 шаг 5
          EEPROM.get(1375, sec);
          EEPROM.get(1380, temp1);
          EEPROM.get(1385, temp2);
          delay(10);
            //термопрофиль User 2 шаг 5
          EEPROM.get(3360, gradsecv);
          EEPROM.get(3365, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 2 шаг 5
          EEPROM.get(3778, pwmv);
          EEPROM.get(3779, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 4)
      {
          //shag == 5
          EEPROM.get(2010, kpv);
          EEPROM.get(2015, kiv);
          EEPROM.get(2020, kdv);
          delay(10);
          //shag == 5
          EEPROM.get(2160, kpn);
          EEPROM.get(2165, kin);
          EEPROM.get(2170, kdn);
          delay(10);
            //термопрофиль User 3 шаг 5
          EEPROM.get(1850, sec);
          EEPROM.get(1855, temp1);
          EEPROM.get(1860, temp2);
          delay(10);
            //термопрофиль User 3 шаг 5
          EEPROM.get(3470, gradsecv);
          EEPROM.get(3475, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 3 шаг 5
          EEPROM.get(3800, pwmv);
          EEPROM.get(3801, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 5)
      {
          //shag == 5
          EEPROM.get(2485, kpv);
          EEPROM.get(2490, kiv);
          EEPROM.get(2495, kdv);
          delay(10);
          //shag == 5
          EEPROM.get(2635, kpn);
          EEPROM.get(2640, kin);
          EEPROM.get(2645, kdn);
          delay(10);
          //термопрофиль User 4 шаг 5
          EEPROM.get(2325, sec);
          EEPROM.get(2330, temp1);
          EEPROM.get(2335, temp2);
          delay(10);
            //термопрофиль User 4 шаг 5
          EEPROM.get(3580, gradsecv);
          EEPROM.get(3585, gradsecn); 
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 4 шаг 5
          EEPROM.get(3822, pwmv);
          EEPROM.get(3823, pwmn);         
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 6)
      {
          //shag == 5
          EEPROM.get(2960, kpv);
          EEPROM.get(2965, kiv);
          EEPROM.get(2970, kdv);
         delay(10);
          //shag == 5
          EEPROM.get(3110, kpn);
          EEPROM.get(3115, kin);
          EEPROM.get(3120, kdn);     
         delay(10);
          //термопрофиль User 5 шаг 5
          EEPROM.get(2800, sec);
          EEPROM.get(2805, temp1);
          EEPROM.get(2810, temp2); 
          delay(10);
            //термопрофиль User 5 шаг 5
          EEPROM.get(3690, gradsecv);
          EEPROM.get(3695, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 5 шаг 5
          EEPROM.get(3844, pwmv);
          EEPROM.get(3845, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }
    } else if(shag == 6)
    {
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0)
       {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1)
      {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }        
      } else if (termoprofily == 2)
      {
          //shag == 6
          EEPROM.get(1090, kpv);
          EEPROM.get(1095, kiv);
          EEPROM.get(1100, kdv);
          delay(10);
          //shag == 6
          EEPROM.get(1225, kpn);
          EEPROM.get(1230, kin);
          EEPROM.get(1235, kdn);
          delay(10);
          EEPROM.get(112, sec);
          EEPROM.get(117, temp1);
          EEPROM.get(122, temp2);
          delay(10);
          //термопрофиль User 1 шаг 6
          EEPROM.get(3260, gradsecv);
          EEPROM.get(3265, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 1 шаг 6
          EEPROM.get(3758, pwmv);
          EEPROM.get(3759, pwmn); 
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 3)
      {
          //shag == 6
          EEPROM.get(1550, kpv);
          EEPROM.get(1555, kiv);
          EEPROM.get(1560, kdv);
          delay(10);
          //shag == 6
          EEPROM.get(1700, kpn);
          EEPROM.get(1705, kin);
          EEPROM.get(1710, kdn);
          delay(10);
          //термопрофиль User 2 шаг 6
          EEPROM.get(1390, sec);
          EEPROM.get(1395, temp1);
          EEPROM.get(1400, temp2);
          delay(10);
            //термопрофиль User 2 шаг 6
          EEPROM.get(3370, gradsecv);
          EEPROM.get(3375, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 2 шаг 6
          EEPROM.get(3780, pwmv);
          EEPROM.get(3781, pwmn); 
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 4)
      {
          //shag == 6
          EEPROM.get(2025, kpv);
          EEPROM.get(2030, kiv);
          EEPROM.get(2035, kdv);
          delay(10);
          //shag == 6
          EEPROM.get(2175, kpn);
          EEPROM.get(2180, kin);
          EEPROM.get(2185, kdn);
          delay(10);
          //термопрофиль User 3 шаг 6
          EEPROM.get(1865, sec);
          EEPROM.get(1870, temp1);
          EEPROM.get(1875, temp2);
          delay(10);
          //термопрофиль User 3 шаг 6
          EEPROM.get(3480, gradsecv);
          EEPROM.get(3485, gradsecn); 
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 3 шаг 6
          EEPROM.get(3802, pwmv);
          EEPROM.get(3803, pwmn);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 5)
      {
          //shag == 6
          EEPROM.get(2500, kpv);
          EEPROM.get(2505, kiv);
          EEPROM.get(2510, kdv);
          delay(10);
          //shag == 6
          EEPROM.get(2650, kpn);
          EEPROM.get(2655, kin);
          EEPROM.get(2660, kdn);
          delay(10);
          //термопрофиль User 4 шаг 6
          EEPROM.get(2340, sec);
          EEPROM.get(2345, temp1);
          EEPROM.get(2350, temp2);
          delay(10);
          //термопрофиль User 4 шаг 6
          EEPROM.get(3590, gradsecv);
          EEPROM.get(3595, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 4 шаг 6
          EEPROM.get(3824, pwmv);
          EEPROM.get(3825, pwmn);           
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 6)
      {
          //shag == 6
          EEPROM.get(2975, kpv);
          EEPROM.get(2980, kiv);
          EEPROM.get(2985, kdv);
         delay(10);
          //shag == 6
          EEPROM.get(3125, kpn);
          EEPROM.get(3130, kin);
          EEPROM.get(3135, kdn);     
         delay(10);
          //термопрофиль User 5 шаг 6
          EEPROM.get(2815, sec);
          EEPROM.get(2820, temp1);
          EEPROM.get(2825, temp2); 
          delay(10);
            //термопрофиль User 5 шаг 6
          EEPROM.get(3700, gradsecv);
          EEPROM.get(3705, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 5 шаг 6
          EEPROM.get(3846, pwmv);
          EEPROM.get(3847, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }
       
    } else if(shag == 7)
    {
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0)
       {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1)
      {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }       
      } else if (termoprofily == 2)
      {
         //shag == 7
         EEPROM.get(1105, kpv);
         EEPROM.get(1110, kiv);
         EEPROM.get(1115, kdv);
         delay(10);
         //shag == 7
         EEPROM.get(1240, kpn);
         EEPROM.get(1245, kin);
         EEPROM.get(1250, kdn);
         delay(10);
         EEPROM.get(127, sec);
         EEPROM.get(132, temp1);
         EEPROM.get(137, temp2);
         delay(10);
          //термопрофиль User 1 шаг 7
        EEPROM.get(3270, gradsecv);
        EEPROM.get(3275, gradsecn);
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 1 шаг 7
        EEPROM.get(3760, pwmv);
        EEPROM.get(3761, pwmn);           
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 3)
      {
        //shag == 7
        EEPROM.get(1565, kpv);
        EEPROM.get(1570, kiv);
        EEPROM.get(1575, kdv);
         delay(10);
        //shag == 7
        EEPROM.get(1715, kpn);
        EEPROM.get(1720, kin);
        EEPROM.get(1725, kdn);
         delay(10);
        //термопрофиль User 2 шаг 7
        EEPROM.get(1405, sec);
        EEPROM.get(1410, temp1);
        EEPROM.get(1415, temp2);
        delay(10);
        //термопрофиль User 2 шаг 7
        EEPROM.get(3380, gradsecv);
        EEPROM.get(3385, gradsecn);
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 2 шаг 7
        EEPROM.get(3782, pwmv);
        EEPROM.get(3783, pwmn);  
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 4)
      {
        //shag == 7
        EEPROM.get(2040, kpv);
        EEPROM.get(2045, kiv);
        EEPROM.get(2050, kdv);
         delay(10);
        //shag == 7
        EEPROM.get(2190, kpn);
        EEPROM.get(2195, kin);
        EEPROM.get(2200, kdn);
         delay(10);
          //термопрофиль User 3 шаг 7
          EEPROM.get(1880, sec);
          EEPROM.get(1885, temp1);
          EEPROM.get(1890, temp2);
          delay(10);
            //термопрофиль User 3 шаг 7
          EEPROM.get(3490, gradsecv);
          EEPROM.get(3495, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 3 шаг 7
          EEPROM.get(3804, pwmv);
          EEPROM.get(3805, pwmn);           
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 5)
      {
        //shag == 7
        EEPROM.get(2515, kpv);
        EEPROM.get(2520, kiv);
        EEPROM.get(2525, kdv);
         delay(10);
        //shag == 7
        EEPROM.get(2665, kpn);
        EEPROM.get(2670, kin);
        EEPROM.get(2675, kdn);
         delay(10);
        //термопрофиль User 4 шаг 7
        EEPROM.get(2355, sec);
        EEPROM.get(2360, temp1);
        EEPROM.get(2365, temp2);
        delay(10);
          //термопрофиль User 4 шаг 7
        EEPROM.get(3600, gradsecv);
        EEPROM.get(3605, gradsecn);
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 4 шаг 7
        EEPROM.get(3826, pwmv);
        EEPROM.get(3827, pwmn);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 6)
      {
          //shag == 7
          EEPROM.get(2990, kpv);
          EEPROM.get(2995, kiv);
          EEPROM.get(3000, kdv);
         delay(10);
          //shag == 7
          EEPROM.get(3140, kpn);
          EEPROM.get(3145, kin);
          EEPROM.get(3150, kdn);     
         delay(10);
          //термопрофиль User 5 шаг 7
          EEPROM.get(2830, sec);
          EEPROM.get(2835, temp1);
          EEPROM.get(2840, temp2); 
          delay(10);	
            //термопрофиль User 5 шаг 7
          EEPROM.get(3710, gradsecv);
          EEPROM.get(3715, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 5 шаг 7
          EEPROM.get(3848, pwmv);
          EEPROM.get(3849, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }
       
    } else if(shag == 8)
    {
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0)
       {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1)
      {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }     
      } else if (termoprofily == 2)
      {
         //shag == 8
         EEPROM.get(1120, kpv);
         EEPROM.get(1125, kiv);
         EEPROM.get(1130, kdv);
         delay(10);
         //shag == 8
         EEPROM.get(1255, kpn);
         EEPROM.get(1260, kin);
         EEPROM.get(1265, kdn);
         delay(10);
         EEPROM.get(142, sec);
         EEPROM.get(147, temp1);
         EEPROM.get(152, temp2);
          delay(10);
            //термопрофиль User 1 шаг 8
          EEPROM.get(3280, gradsecv);
          EEPROM.get(3285, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 1 шаг 8
          EEPROM.get(3762, pwmv);
          EEPROM.get(3763, pwmn);         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 3)
      {
        //shag == 8
        EEPROM.get(1580, kpv);
        EEPROM.get(1585, kiv);
        EEPROM.get(1590, kdv);
         delay(10);
        //shag == 8
        EEPROM.get(1730, kpn);
        EEPROM.get(1735, kin);
        EEPROM.get(1740, kdn);
         delay(10);
        //термопрофиль User 2 шаг 8
        EEPROM.get(1420, sec);
        EEPROM.get(1425, temp1);
        EEPROM.get(1430, temp2);
        delay(10);
          //термопрофиль User 2 шаг 8
        EEPROM.get(3390, gradsecv);
        EEPROM.get(3395, gradsecn);
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 2 шаг 8
        EEPROM.get(3784, pwmv);
        EEPROM.get(3785, pwmn);        
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 4)
      {
        //shag == 8
        EEPROM.get(2055, kpv);
        EEPROM.get(2060, kiv);
        EEPROM.get(2065, kdv);
         delay(10);
        //shag == 8
        EEPROM.get(2205, kpn);
        EEPROM.get(2210, kin);
        EEPROM.get(2215, kdn);
         delay(10);
        //термопрофиль User 3 шаг 8
        EEPROM.get(1895, sec);
        EEPROM.get(1900, temp1);
        EEPROM.get(1905, temp2);
        delay(10);
          //термопрофиль User 3 шаг 8
        EEPROM.get(3500, gradsecv);
        EEPROM.get(3505, gradsecn);
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
         //термопрофиль User 3 шаг 8
        EEPROM.get(3806, pwmv);
        EEPROM.get(3807, pwmn);       
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 5)
      {
        //shag == 8
        EEPROM.get(2530, kpv);
        EEPROM.get(2535, kiv);
        EEPROM.get(2540, kdv);
         delay(10);
        //shag == 8
        EEPROM.get(2680, kpn);
        EEPROM.get(2685, kin);
        EEPROM.get(2690, kdn);
         delay(10);
        //термопрофиль User 4 шаг 8
        EEPROM.get(2370, sec);
        EEPROM.get(2375, temp1);
        EEPROM.get(2380, temp2);
        delay(10);
          //термопрофиль User 4 шаг 8
        EEPROM.get(3610, gradsecv);
        EEPROM.get(3615, gradsecn);
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
         //термопрофиль User 4 шаг 8
        EEPROM.get(3828, pwmv);
        EEPROM.get(3829, pwmn);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 6)
      {
          //shag == 8
          EEPROM.get(3005, kpv);
          EEPROM.get(3010, kiv);
          EEPROM.get(3015, kdv);
         delay(10);
          //shag == 8
          EEPROM.get(3155, kpn);
          EEPROM.get(3160, kin);
          EEPROM.get(3165, kdn);     
         delay(10);
          //термопрофиль User 5 шаг 8
          EEPROM.get(2845, sec);
          EEPROM.get(2850, temp1);
          EEPROM.get(2855, temp2);
          delay(10);	
          //термопрофиль User 5 шаг 8
          EEPROM.get(3720, gradsecv);
          EEPROM.get(3725, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 5 шаг 8
          EEPROM.get(3850, pwmv);
          EEPROM.get(3851, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }
       
    } else if(shag == 9)
    {
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0)
       {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
           
         }
      } else if (termoprofily == 1)
      {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }       
      } else if (termoprofily == 2)
      {
         //shag == 9
         EEPROM.get(1135, kpv);
         EEPROM.get(1140, kiv);
         EEPROM.get(1145, kdv);
         delay(10);
         //shag == 9
         EEPROM.get(1270, kpn);
         EEPROM.get(1275, kin);
         EEPROM.get(1280, kdn);
         delay(10);
         EEPROM.get(157, sec);
         EEPROM.get(162, temp1);
         EEPROM.get(167, temp2);
         delay(10);
         //термопрофиль User 1 шаг 9
          EEPROM.get(3290, gradsecv);
          EEPROM.get(3295, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
           //термопрофиль User 1 шаг 9
          EEPROM.get(3764, pwmv);
          EEPROM.get(3765, pwmn);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 3)
      {
        //shag == 9
        EEPROM.get(1595, kpv);
        EEPROM.get(1600, kiv);
        EEPROM.get(1605, kdv);
         delay(10);
        //shag == 9
        EEPROM.get(1745, kpn);
        EEPROM.get(1750, kin);
        EEPROM.get(1755, kdn);
         delay(10);
        //термопрофиль User 2 шаг 9
        EEPROM.get(1435, sec);
        EEPROM.get(1440, temp1);
        EEPROM.get(1445, temp2);
        delay(10);
          //термопрофиль User 2 шаг 9
        EEPROM.get(3400, gradsecv);
        EEPROM.get(3405, gradsecn); 
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 2 шаг 9
        EEPROM.get(3786, pwmv);
        EEPROM.get(3787, pwmn);       
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 4)
      {
        //shag == 9
        EEPROM.get(2070, kpv);
        EEPROM.get(2075, kiv);
        EEPROM.get(2080, kdv);
         delay(10);
        //shag == 9
        EEPROM.get(2220, kpn);
        EEPROM.get(2225, kin);
        EEPROM.get(2230, kdn);
         delay(10);
        //термопрофиль User 3 шаг 9
        EEPROM.get(1910, sec);
        EEPROM.get(1915, temp1);
        EEPROM.get(1920, temp2);
        delay(10);  
          //термопрофиль User 3 шаг 9
        EEPROM.get(3510, gradsecv);
        EEPROM.get(3515, gradsecn);
        delay(10); 
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 3 шаг 9
        EEPROM.get(3808, pwmv);
        EEPROM.get(3809, pwmn);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       }else if (termoprofily == 5)
      {
        //shag == 9
        EEPROM.get(2545, kpv);
        EEPROM.get(2550, kiv);
        EEPROM.get(2555, kdv);
         delay(10);
        //shag == 9
        EEPROM.get(2695, kpn);
        EEPROM.get(2700, kin);
        EEPROM.get(2705, kdn);
         delay(10);
        //термопрофиль User 4 шаг 9
        EEPROM.get(2385, sec);
        EEPROM.get(2390, temp1);
        EEPROM.get(2395, temp2);
        delay(10);
          //термопрофиль User 4 шаг 9
        EEPROM.get(3620, gradsecv);
        EEPROM.get(3625, gradsecn); 
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 4 шаг 9
        EEPROM.get(3830, pwmv);
        EEPROM.get(3831, pwmn);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 6)
      {
          //shag == 9
          EEPROM.get(3020, kpv);
          EEPROM.get(3025, kiv);
          EEPROM.get(3030, kdv);
         delay(10);
          //shag == 9
          EEPROM.get(3170, kpn);
          EEPROM.get(3175, kin);
          EEPROM.get(3180, kdn);     
         delay(10);
          //термопрофиль User 5 шаг 9
          EEPROM.get(2860, sec);
          EEPROM.get(2865, temp1);
          EEPROM.get(2870, temp2); 
          delay(10);
            //термопрофиль User 5 шаг 9
          EEPROM.get(3730, gradsecv);
          EEPROM.get(3735, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 5 шаг 9
          EEPROM.get(3852, pwmv);
          EEPROM.get(3853, pwmn);
         //sec=80;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      }
    } else if(shag == 10)
    {
       //sec=80;
       outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
       outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0)
       {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
      } else if (termoprofily == 1)
      {
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
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }    
      } else if (termoprofily == 2)
      {
         //shag == 10
         EEPROM.get(1150, kpv);
         EEPROM.get(1155, kiv);
         EEPROM.get(1160, kdv);
         delay(10);
         //shag == 10
         EEPROM.get(1285, kpn);
         EEPROM.get(1290, kin);
         EEPROM.get(1295, kdn);
         delay(10);
         EEPROM.get(172, sec);
         EEPROM.get(177, temp1);
         EEPROM.get(182, temp2);
         delay(10);
         //термопрофиль User 1 шаг 10
          EEPROM.get(3300, gradsecv);
          EEPROM.get(3305, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 1 шаг 10
        EEPROM.get(3766, pwmv);
        EEPROM.get(3767, pwmn);         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 3)
      {
         //shag == 10
         EEPROM.get(1610, kpv);
         EEPROM.get(1615, kiv);
         EEPROM.get(1620, kdv);
         delay(10);
         //shag == 10
         EEPROM.get(1760, kpn);
         EEPROM.get(1765, kin);
         EEPROM.get(1770, kdn);
         delay(10);
         //термопрофиль User 2 шаг 10
         EEPROM.get(1450, sec);
         EEPROM.get(1455, temp1);
         EEPROM.get(1460, temp2);
         delay(10);
          //термопрофиль User 2 шаг 10
        EEPROM.get(3410, gradsecv);
        EEPROM.get(3415, gradsecn);
        delay(10);
        EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
        //термопрофиль User 2 шаг 10
        EEPROM.get(3788, pwmv);
        EEPROM.get(3789, pwmn);         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 4)
      {
         //shag == 10
          EEPROM.get(2085, kpv);
          EEPROM.get(2090, kiv);
          EEPROM.get(2095, kdv);
         delay(10);
         //shag == 10
          EEPROM.get(2235, kpn);
          EEPROM.get(2240, kin);
          EEPROM.get(2245, kdn);
         delay(10);
          //термопрофиль User 3 шаг 10
          EEPROM.get(1925, sec);
          EEPROM.get(1930, temp1);
          EEPROM.get(1935, temp2);
          delay(10);
            //термопрофиль User 3 шаг 10
          EEPROM.get(3520, gradsecv);
          EEPROM.get(3525, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 3 шаг 10
          EEPROM.get(3810, pwmv);
          EEPROM.get(3811, pwmn);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 5)
      {
         //shag == 10
          EEPROM.get(2560, kpv);
          EEPROM.get(2565, kiv);
          EEPROM.get(2570, kdv);
         delay(10);
         //shag == 10
          EEPROM.get(2710, kpn);
          EEPROM.get(2715, kin);
          EEPROM.get(2720, kdn);
         delay(10);
          //термопрофиль User 4 шаг 10
          EEPROM.get(2400, sec);
          EEPROM.get(2405, temp1);
          EEPROM.get(2410, temp2);
          delay(10);
            //термопрофиль User 4 шаг 10
          EEPROM.get(3630, gradsecv);
          EEPROM.get(3635, gradsecn); 
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 4 шаг 10
          EEPROM.get(3832, pwmv);
          EEPROM.get(3833, pwmn);         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
       } else if (termoprofily == 6)
        {
          //shag == 10
          EEPROM.get(3035, kpv);
          EEPROM.get(3040, kiv);
          EEPROM.get(3045, kdv);
         delay(10);
          //shag == 10
          EEPROM.get(3185, kpn);
          EEPROM.get(3190, kin);
          EEPROM.get(3195, kdn);      
         delay(10);
          //термопрофиль User 5 шаг 10
          EEPROM.get(2875, sec);
          EEPROM.get(2880, temp1);
          EEPROM.get(2885, temp2);
          delay(10);
            //термопрофиль User 5 шаг 10
          EEPROM.get(3740, gradsecv);
          EEPROM.get(3745, gradsecn);
          delay(10);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);
          delay(10);
          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown); 
          delay(10);
          //термопрофиль User 5 шаг 10
          EEPROM.get(3854, pwmv);
          EEPROM.get(3855, pwmn);
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Text();
         //temp1 = 0; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 100; // Нижний нагреватель Бессвинецовый выбрано 100 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily1_9 = 1;
         }
        }
       }
    } else {
         shag = 0;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
       if (termoprofily == 0)
       {
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
         if (reley_n==1)
         {
           termoprofily10 = 1;
           reley_n=0;
           reley_n1=0;
           reley_v=0;
           analogWrite(nigniy_1, 0);
           analogWrite(verhniy_1, 0);
           
         }
         
      } else if (termoprofily == 1)
      {
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
         if (reley_n==1)
         {
           termoprofily10 = 1;
           reley_n=0;
           reley_n1=0;
           reley_v=0;
           analogWrite(nigniy_1, 0);
           analogWrite(verhniy_1, 0);
           
         }       
      } else if (termoprofily == 2)
      {
         EEPROM.get(227, dtv);	   
         EEPROM.get(232, dtn);
         delay(10); 
         EEPROM.get(22, sec);
         EEPROM.get(27, temp1);
         EEPROM.get(32, temp2);
         delay(10);
         //термопрофиль User 1 шаг 0
          EEPROM.get(3200, gradsecv);
          EEPROM.get(3205, gradsecn);
          delay(10);
          //термопрофиль User 1 шаг 0
          EEPROM.get(3746, pwmv);
          EEPROM.get(3747, pwmn);
          // Оповещение температуры User 1
          EEPROM.get(3856,setopov1);
          EEPROM.get(3861,setopov2);
          EEPROM.get(3866,setopov3);
          EEPROM.get(3871,setopov4);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);

          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown);          
         shag = 0;
         //sec=0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          Dtv = dtv;
          String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
          SendData("t57.txt", t57);
          Dtn = dtn;
          String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
          SendData("t58.txt", t58);
          GradSecv = gradsecv;
          String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
          SendData("tgcs3.txt", tgcs3);
          GradSecn = gradsecn;
          String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
          SendData("tgcs4.txt", tgcs4);
          Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
          outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
          pwmust1 = pwmv;
          outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
          pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
        // temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 0 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily10 = 1;
           reley_n=0;
           reley_n1=0;
           reley_v=0;
           analogWrite(nigniy_1, 0);
           analogWrite(verhniy_1, 0);
          
         }
       }else if (termoprofily == 3)
      {
         //термопрофиль User 2
	       //delay(10);
         EEPROM.get(1465, dtv); 
         EEPROM.get(1470, dtn);
         delay(10); 
         //термопрофиль User 2 шаг 0
         EEPROM.get(1300, sec);
         EEPROM.get(1305, temp1);
         EEPROM.get(1310, temp2);
         delay(10); 
         //термопрофиль User 2 шаг 0
          EEPROM.get(3310, gradsecv);
          EEPROM.get(3315, gradsecn);
          delay(10);
          //термопрофиль User 2 шаг 0
          EEPROM.get(3768, pwmv);
          EEPROM.get(3769, pwmn);
          // Оповещение температуры User 2
          EEPROM.get(3876,setopov1);
          EEPROM.get(3881,setopov2);
          EEPROM.get(3886,setopov3);
          EEPROM.get(3891,setopov4);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);

          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown);          
         shag = 0;
         //sec=0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          Dtv = dtv;
          String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
          SendData("t57.txt", t57);
          Dtn = dtn;
          String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
          SendData("t58.txt", t58);
          GradSecv = gradsecv;
          String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
          SendData("tgcs3.txt", tgcs3);
          GradSecn = gradsecn;
          String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
          SendData("tgcs4.txt", tgcs4);
           Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        
          outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
          pwmust1 = pwmv;
          outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
          pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
        // temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 0 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily10 = 1;
           reley_n=0;
           reley_n1=0;
           reley_v=0;
           analogWrite(nigniy_1, 0);
           analogWrite(verhniy_1, 0);
           
         }
       }else if (termoprofily == 4)
      {
        //термопрофиль User 3
        //delay(10);
          EEPROM.get(1940, dtv); 
          EEPROM.get(1945, dtn); 
         delay(10); 
        //термопрофиль User 3 шаг 0
          EEPROM.get(1775, sec);
          EEPROM.get(1780, temp1);
          EEPROM.get(1785, temp2);
          delay(10); 
            //термопрофиль User 3 шаг 0
          EEPROM.get(3420, gradsecv);
          EEPROM.get(3425, gradsecn);
          delay(10); 
          //термопрофиль User 3 шаг 0
          EEPROM.get(3790, pwmv);
          EEPROM.get(3791, pwmn);
          // Оповещение температуры User 3
          EEPROM.get(3896,setopov1);
          EEPROM.get(3901,setopov2);
          EEPROM.get(3906,setopov3);
          EEPROM.get(3911,setopov4);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);

          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown);
         shag = 0;
         //sec=0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          Dtv = dtv;
          String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
          SendData("t57.txt", t57);
          Dtn = dtn;
          String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
          SendData("t58.txt", t58);
          GradSecv = gradsecv;
          String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
          SendData("tgcs3.txt", tgcs3);
          GradSecn = gradsecn;
          String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
          SendData("tgcs4.txt", tgcs4);
          Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        
          outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
          pwmust1 = pwmv;
          outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
          pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
        // temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 0 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily10 = 1;
           reley_n=0;
           reley_n1=0;
           reley_v=0;
           analogWrite(nigniy_1, 0);
           analogWrite(verhniy_1, 0);
           
         }
       }else if (termoprofily == 5)
      {
        //термопрофиль User 4
        //delay(10);
          EEPROM.get(2415, dtv); 
          EEPROM.get(2420, dtn);
         delay(10); 
        //термопрофиль User 4 шаг 0
          EEPROM.get(2250, sec);
          EEPROM.get(2255, temp1);
          EEPROM.get(2260, temp2);
          delay(10); 
            //термопрофиль User 4 шаг 0
          EEPROM.get(3530, gradsecv);
          EEPROM.get(3535, gradsecn);
          delay(10);
          //термопрофиль User 4 шаг 0
          EEPROM.get(3812, pwmv);
          EEPROM.get(3813, pwmn);
          // Оповещение температуры User 4
          EEPROM.get(3916,setopov1);
          EEPROM.get(3921,setopov2);
          EEPROM.get(3926,setopov3);
          EEPROM.get(3931,setopov4);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);

          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown);
         shag = 0;
         //sec=0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          Dtv = dtv;
          String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
          SendData("t57.txt", t57);
          Dtn = dtn;
          String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
          SendData("t58.txt", t58);
          GradSecv = gradsecv;
          String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
          SendData("tgcs3.txt", tgcs3);
          GradSecn = gradsecn;
          String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
          SendData("tgcs4.txt", tgcs4);
          Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        
          outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
          pwmust1 = pwmv;
          outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
          pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
        // temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 0 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
           termoprofily10 = 1;
           reley_n=0;
           reley_n1=0;
           reley_v=0;
           analogWrite(nigniy_1, 0);
           analogWrite(verhniy_1, 0);
           
         }
       }else if (termoprofily == 6)
      {
        //термопрофиль User 5
        //delay(10);
          EEPROM.get(2890, dtv); 
          EEPROM.get(2895, dtn); 
         delay(10); 
        //термопрофиль User 5 шаг 0
          EEPROM.get(2725, sec);
          EEPROM.get(2730, temp1);
          EEPROM.get(2735, temp2);
          delay(10);	
            //термопрофиль User 5 шаг 0
          EEPROM.get(3640, gradsecv);
          EEPROM.get(3645, gradsecn);
          delay(10);
          //термопрофиль User 5 шаг 0
          EEPROM.get(3834, pwmv);
          EEPROM.get(3835, pwmn);
          // Оповещение температуры User 5
          EEPROM.get(3936,setopov1);
          EEPROM.get(3941,setopov2);
          EEPROM.get(3946,setopov3);
          EEPROM.get(3951,setopov4);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);

          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown);          
         shag = 0;
         //sec=0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
          Dtv = dtv;
          String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
          SendData("t57.txt", t57);
          Dtn = dtn;
          String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
          SendData("t58.txt", t58);
          GradSecv = gradsecv;
          String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
          SendData("tgcs3.txt", tgcs3);
          GradSecn = gradsecn;
          String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
          SendData("tgcs4.txt", tgcs4);
          Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
          outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
          pwmust1 = pwmv;
          outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
          pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 0 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
        // temp2 = 160; // Нижний нагреватель Бессвинецовый выбрано 0 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         if (reley_n==1)
         {
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
    if (incStr.indexOf("bs20") >= 0) 
    {
      if (sec < 100000)
      {
        sec = sec + rtemp;
        outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
      } else if(sec == 100000)
      {
         sec = 0;
        outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
      }
    
  }
    if (incStr.indexOf("bs21") >= 0) 
    {
      if (sec > 0 )
      {
        sec = sec - rtemp;
        outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
      } else if (sec == 0)
      {
        sec = 100000;
        outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec
      }
    
    }
  if (incStr.indexOf("b6") >= 0) 
  {
    if (temp1 < 450)
    {
      temp1=temp1+rtemp;
      outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
      tempust1 = temp1;
    } else if(temp1 == 450)
    {
        temp1 = 0;
       outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
       tempust1 = temp1;
    }
    
  }
    if (incStr.indexOf("b7") >= 0) 
    {
      if (temp1 > 0 )
      {
        temp1=temp1-rtemp;
        outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
        tempust1 = temp1;
      } else if (temp1 == 0)
      {
        temp1 = 450;
        outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
        tempust1 = temp1;
      }
    
    }
    if (incStr.indexOf("b8") >= 0) 
    {
      if (temp2 < 450)
      {
        temp2=temp2+rtemp;
        outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
        tempust2 = temp2;
      } else if (temp2 == 450)
      {
        temp2 = 0;
        outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
        tempust2 = temp2;
      }
    
    }
    if (incStr.indexOf("b9") >= 0) 
    {
     if (temp2 > 0)
     {
       temp2=temp2-rtemp;
       outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
       tempust2 = temp2;
     } else if (temp2 == 0)
     {
       temp2 = 450;
       outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp2
       tempust2 = temp2;
     }
    
   }
    if (incStr.indexOf("watt10") >= 0) 
    {
      if (pwmv < 255){
        pwmv=pwmv+rtemp;
        outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
      } else if(pwmv == 255)
      {
        pwmv = 0;
        outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
      }
    
    }
    if (incStr.indexOf("watt11") >= 0) 
    {
      if (pwmv > 0 )
      {
        pwmv=pwmv-rtemp;
        outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
      } else if (pwmv == 0)
      {
        pwmv = 255;
        outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
      }
    
    }
    if (incStr.indexOf("watt12") >= 0) 
    {
      if (pwmn < 255)
      {
        pwmn=pwmn+rtemp;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
        pwmust2 = pwmn;
      } else if(pwmn == 255)
      {
         pwmn = 0;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
        pwmust2 = pwmn;
      }
    
    }
    if (incStr.indexOf("watt13") >= 0) 
    {
      if (pwmn > 0 )
      {
         pwmn=pwmn-rtemp;
         outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
         pwmust2 = pwmn;
      } else if (pwmn == 0)
      {
        pwmn = 255;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmn
        pwmust2 = pwmn;
      }
    
    }  
    if (incStr.indexOf("pid14") >= 0) 
    {
      if (kpv < 10000.0)
      {
         kpv=kpv+rtemp;
         Kpv = kpv;
         String t24 = "\"" + String(kpv,5) + "\"";  // Отображение kp
         SendData("t24.txt", t24);
      
      } else if(kpv == 10000.0)
      {
        kpv = 0.00;
        Kpv = kpv;
        String t24 = "\"" + String(kpv,5) + "\"";  // Отображение kp
        SendData("t24.txt", t24);
      
      }
    
    }
    if (incStr.indexOf("pid15") >= 0) 
    {
      if (kpv > -10000.00 )
      {
        kpv=kpv-rtemp; 
        Kpv = kpv; 
        String t24 = "\"" + String(kpv,5) + "\"";  // Отображение kp
        SendData("t24.txt", t24);
      
      } else if (kpv == -10000.00)
      {
        kpv = 0.00;
        Kpv = kpv;
        String t24 = "\"" + String(kpv,5) + "\"";  // Отображение kp
        SendData("t24.txt", t24);
        
      }
    
    }   
    if (incStr.indexOf("pid16") >= 0) 
    {
      if (kiv < 10000.0)
      {
        kiv=kiv+rtemp;
        Kiv = kiv;
        String t25 = "\"" + String(kiv,5) + "\"";  // Отображение ki
        SendData("t25.txt", t25);
      
      } else if(kiv == 10000.0)
      {
        kiv = 0.00;
        Kiv = kiv;
        String t25 = "\"" + String(kiv,5) + "\"";  // Отображение ki
        SendData("t25.txt", t25);
        
      }
    
    }
    if (incStr.indexOf("pid17") >= 0) 
    {
      if (kiv > -10000.00 )
      {
        kiv=kiv-rtemp;
        Kiv = kiv;
        String t25 = "\"" + String(kiv,5) + "\"";  // Отображение ki
        SendData("t25.txt", t25);
      
      } else if (kiv == -10000.00)
      {
        kiv = 0.00;
        Kiv = kiv;
        String t25 = "\"" + String(kiv,5) + "\"";  // Отображение ki
        SendData("t25.txt", t25);
       
      }
    
    }    
  if (incStr.indexOf("pid18") >= 0) 
  {
    if (kdv < 10000.0)
    {
      kdv=kdv+rtemp;
      Kdv = kdv;
      String t26 = "\"" + String(kdv,5) + "\"";  // Отображение kd
      SendData("t26.txt", t26);
    } else if(kdv == 10000.0)
    {
        kdv = 0.00;
        Kdv = kdv;
        String t26 = "\"" + String(kdv,5) + "\"";  // Отображение kd
        SendData("t26.txt", t26);
        
    }
    
  }
    if (incStr.indexOf("pid19") >= 0) 
    {
      if (kdv > -10000.00 )
      {
        kdv=kdv-rtemp;
        Kdv = kdv;
        String t26 = "\"" + String(kdv,5) + "\"";  // Отображение kd
        SendData("t26.txt", t26);
      } else if (kdv == -10000.00)
      {
         kdv = 0.00;
         Kdv = kdv;
         String t26 = "\"" + String(kdv,5) + "\"";  // Отображение kd
         SendData("t26.txt", t26);
       
      }
    } 
    if (incStr.indexOf("pid20") >= 0) 
    {
      if (kpn < 10000.0)
      {
        kpn=kpn+rtemp;
        Kpn = kpn;
        String t54 = "\"" + String(kpn,5) + "\"";  // Отображение kp
       SendData("t54.txt", t54);
      
      } else if(kpn == 10000.0)
      {
        kpn = 0.00;
        Kpn = kpn;
        String t54 = "\"" + String(kpn,5) + "\"";  // Отображение kp
       SendData("t54.txt", t54);
      }
    }
    if (incStr.indexOf("pid21") >= 0) 
    {
      if (kpn > -10000.00 )
      {
        kpn=kpn-rtemp; 
        Kpn = kpn; 
        String t54 = "\"" + String(kpn,5) + "\"";  // Отображение kp
        SendData("t54.txt", t54);
      } else if (kpn == -10000.00)
      {
        kpn = 0.00;
        Kpn = kpn;
        String t54 = "\"" + String(kpn,5) + "\"";  // Отображение kp
        SendData("t54.txt", t54);
      }
    }   
    if (incStr.indexOf("pid22") >= 0) 
    {
      if (kin < 10000.0)
      {
        kin=kin+rtemp;
        Kin = kin;
        String t55 = "\"" + String(kin,5) + "\"";  // Отображение ki
        SendData("t55.txt", t55);
      } else if(kin == 10000.0)
      {
        kin = 0.00;
        Kin = kin;
        String t55 = "\"" + String(kin,5) + "\"";  // Отображение ki
        SendData("t55.txt", t55);
      }
    }
    if (incStr.indexOf("pid23") >= 0) 
    {
      if (kin > -10000.00 )
      {
        kin=kin-rtemp;
        Kin = kin;
        String t55 = "\"" + String(kin,5) + "\"";  // Отображение ki
        SendData("t55.txt", t55);
      } else if (kin == -10000.00)
      {
        kin = 0.00;
        Kin = kin;
        String t55 = "\"" + String(kin,5) + "\"";  // Отображение ki
        SendData("t55.txt", t55);
      }
    }    
    if (incStr.indexOf("pid24") >= 0) 
    {
      if (kdn < 10000.0)
      {
        kdn=kdn+rtemp;
        Kdn = kdn;
        String t56 = "\"" + String(kdn,5) + "\"";  // Отображение kd
        SendData("t56.txt", t56);
      } else if(kdn == 10000.0)
      {
        kdn = 0.00;
        Kdn = kdn;
        String t56 = "\"" + String(kdn,5) + "\"";  // Отображение kd
        SendData("t56.txt", t56);
      }
    }
    if (incStr.indexOf("pid25") >= 0) 
    {
      if (kdn > -10000.00 )
      {
        kdn=kdn-rtemp;
        Kdn = kdn;
        String t56 = "\"" + String(kdn,5) + "\"";  // Отображение kd
        SendData("t56.txt", t56);
      } else if (kdn == -10000.00)
      {
         kdn = 0.00;
         Kdn = kdn;
         String t56 = "\"" + String(kdn,5) + "\"";  // Отображение kd
         SendData("t56.txt", t56);
      }
    }     
    if (incStr.indexOf("pid26") >= 0) 
    {
      if (dtv < 10.0)
      {
        dtv=dtv+rtemp;
        Dtv = dtv;
        String t57 = "\"" + String(dtv) + "\"";  // Отображение kd
        SendData("t57.txt", t57);
      } else if(dtv == 10.0)
      {
        dtv = 0.00;
        Dtv = dtv;
        String t57 = "\"" + String(dtv) + "\"";  // Отображение kd
        SendData("t57.txt", t57);
      }
    }
    if (incStr.indexOf("pid27") >= 0) 
    {
      if (dtv > 0.00 )
      {
        dtv=dtv-rtemp;
        Dtv = dtv;
        String t57 = "\"" + String(dtv) + "\"";  // Отображение kd
        SendData("t57.txt", t57);
      } else if (dtv == 0.00)
      {
         dtv = 10.0;
         Dtv = dtv;
         String t57 = "\"" + String(dtv) + "\"";  // Отображение kd
         SendData("t57.txt", t57);
      }
    }     
    if (incStr.indexOf("pid28") >= 0) 
    {
      if (dtn < 10.0)
      {
        dtn=dtn+rtemp;
        Dtn = dtn;
        String t58 = "\"" + String(dtn) + "\"";  // Отображение kd
        SendData("t58.txt", t58);
      } else if(dtn == 10.0)
      {
        dtn = 0.00;
        Dtn = dtn;
        String t58 = "\"" + String(dtn) + "\"";  // Отображение kd
        SendData("t58.txt", t58);
      }
    }
    if (incStr.indexOf("pid29") >= 0) 
    {
      if (dtn > 0.00 )
      {
        dtn=dtn-rtemp;
        Dtn = dtn;
        String t58 = "\"" + String(dtn) + "\"";  // Отображение kd
        SendData("t58.txt", t58);
      } else if (dtn == 0.00)
      {
         dtn = 10.0;
         Dtn = dtn;
         String t58 = "\"" + String(dtn) + "\"";  // Отображение kd
         SendData("t58.txt", t58);
      }
    }     

  if (incStr.indexOf("bcv") >= 0) 
  {
    if (coolervh < 255)
    {
      coolervh=coolervh+rtemp;
      outNumber("coolervh.val", coolervh);  // Отображение числа в числовом компоненте coolervh
      coolvust1 = coolervh;
    } else if(coolervh == 255)
    {
        coolervh = 0;
       outNumber("coolervh.val", coolervh);  // Отображение числа в числовом компоненте coolervh
       coolvust1 = coolervh;
    }
    
  }
    if (incStr.indexOf("bv2") >= 0) 
    {
    if (coolervh > 0 )
    {
      coolervh=coolervh-rtemp;
      outNumber("coolervh.val", coolervh);  // Отображение числа в числовом компоненте coolervh
      coolvust1 = coolervh;
    } else if (coolervh == 0)
    {
       coolervh = 255;
       outNumber("coolervh.val", coolervh);  // Отображение числа в числовом компоненте coolervh
       coolvust1 = coolervh;
    }
    
  }

  if (incStr.indexOf("bcp") >= 0) 
  {
    if (coolerp < 255)
    {
      coolerp=coolerp+rtemp;
      outNumber("coolerp.val", coolerp);  // Отображение числа в числовом компоненте coolerp
      coolpust2 = coolerp;
    } else if(coolerp == 255)
    {
        coolerp = 0;
       outNumber("coolerp.val", coolerp);  // Отображение числа в числовом компоненте coolerp
       coolpust2 = coolerp;
    }
    
  }
    if (incStr.indexOf("bp2") >= 0) 
    {
      if (coolerp > 0 )
      {
        coolerp=coolerp-rtemp;
        outNumber("coolerp.val", coolerp);  // Отображение числа в числовом компоненте coolerp
        coolpust2 = coolerp;
      } else if (coolerp == 0)
      {
        coolerp = 255;
        outNumber("coolerp.val", coolerp);  // Отображение числа в числовом компоненте coolerp
         coolpust2 = coolerp;
      }
    
    }

    if (incStr.indexOf("ct0") >= 0) 
    {
      if (comptempt1 < 100.0)
      {
        comptempt1=comptempt1+rtemp;
        //Comptempt1 = comptempt1;
        String t61 = "\"" + String(znak1) + String(comptempt1) + "\"";  
        SendData("t61.txt", t61);
      } else if(comptempt1 == 100.0)
      {
        comptempt1 = 0.00;
        //Comptempt1 = comptempt1;
        String t61 = "\"" + String(znak1) + String(comptempt1) + "\"";  
        SendData("t61.txt", t61);
      }
    }
    if (incStr.indexOf("ct1") >= 0) 
    {
      if (comptempt1 > 0.00 )
      {
        comptempt1=comptempt1-rtemp;
        //Comptempt1 = comptempt1;
        String t61 = "\"" + String(znak1) + String(comptempt1) + "\"";  
        SendData("t61.txt", t61);
      } else if (comptempt1 == 0.00)
      {
         comptempt1 = 100.0;
         //Comptempt1 = comptempt1;
         String t61 = "\"" + String(znak1) + String(comptempt1) + "\"";  
         SendData("t61.txt", t61);
      }
    }     
	
    if (incStr.indexOf("ct2") >= 0) 
    {
      if (comptempt2 < 100.0)
      {
        comptempt2=comptempt2+rtemp;
        //Comptempt2 = comptempt2;
        String t62 = "\"" + String(znak2) + String(comptempt2) + "\"";  
        SendData("t62.txt", t62);
      } else if(comptempt2 == 100.0)
      {
        comptempt2 = 0.00;
        //Comptempt2 = comptempt2;
        String t62 = "\"" + String(znak2) + String(comptempt2) + "\"";  
        SendData("t62.txt", t62);
      }
    }
    if (incStr.indexOf("ct3") >= 0) 
    {
      if (comptempt2 > 0.00 )
      {
        comptempt2=comptempt2-rtemp;
        //Comptempt2 = comptempt2;
        String t62 = "\"" + String(znak2) + String(comptempt2) + "\"";  
        SendData("t62.txt", t62);
      } else if (comptempt2 == 0.00)
      {
         comptempt2 = 100.0;
         //Comptempt2 = comptempt2;
         String t62 = "\"" + String(znak2) + String(comptempt2) + "\"";  
         SendData("t62.txt", t62);
      }
    }    
    if (incStr.indexOf("ct4") >= 0) 
    {
      znak1 = '+';
      znak2 = '+';
      if (incStr.indexOf("comptemp1") >= 0) 
      { 
             
             String t61 = "\"" + String(znak1) + String(comptempt1) + "\"";  
             SendData("t61.txt", t61);  
          
      } else if (incStr.indexOf("comptemp2") >= 0) 
      {    
             
             String t62 = "\"" + String(znak2) + String(comptempt2) + "\"";  
             SendData("t62.txt", t62);
          
      }
    }
    if (incStr.indexOf("ct5") >= 0) 
    {
		
      znak1 = '-';
      znak2 = '-';      
      if (incStr.indexOf("comptemp1") >= 0)
      { 
            
            String t61 = "\"" + String(znak1) + String(comptempt1) + "\"";  
            SendData("t61.txt", t61);    
          
          
      } else if (incStr.indexOf("comptemp2") >= 0)
      {    
             
             String t62 = "\"" + String(znak2) + String(comptempt2) + "\"";  
             SendData("t62.txt", t62);
          
      }
    } 
  
  if (incStr.indexOf("degrees1") >= 0) 
    {
      if (gradsecv < 100.0)
      {
       gradsecv=gradsecv+rtemp;
        GradSecv = gradsecv;
        String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
        SendData("tgcs3.txt", tgcs3);
      } else if(gradsecv == 100.0)
      {
        gradsecv = 0.00;
        GradSecv = gradsecv;
        String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
        SendData("tgcs3.txt", tgcs3);
      }
    }
    if (incStr.indexOf("degrees2") >= 0) 
    {
      if (gradsecv > 0.00 )
      {
        gradsecv=gradsecv-rtemp;
        GradSecv = gradsecv;
        String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
        SendData("tgcs3.txt", tgcs3);
      } else if (gradsecv == 0.00)
      {
         gradsecv = 100.0;
         GradSecv = gradsecv;
         String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
         SendData("tgcs3.txt", tgcs3);
      }
    }     

    //**************************

    if (incStr.indexOf("degrees3") >= 0) 
    {
      if (gradsecn < 100.0)
      {
       gradsecn=gradsecn+rtemp;
        GradSecn = gradsecn;
        String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
        SendData("tgcs4.txt", tgcs4);
      } else if(gradsecn == 100.0)
      {
        gradsecn = 0.00;
        GradSecn = gradsecn;
        String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
        SendData("tgcs4.txt", tgcs4);
      }
    }
    if (incStr.indexOf("degrees4") >= 0) 
    {
      if (gradsecn > 0.00 )
      {
        gradsecn=gradsecn-rtemp;
        GradSecn = gradsecn;
        String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
        SendData("tgcs4.txt", tgcs4);
      } else if (gradsecn == 0.00)
      {
         gradsecn = 100.0;
         GradSecn = gradsecn;
         String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
         SendData("tgcs4.txt", tgcs4);
      }
    }     

  //**************************

    if (incStr.indexOf("setopzvyk1") >= 0) 
    {
      if (setopov1 < 450)
      {
        setopov1=setopov1+itemp;
        SetOpov1 = setopov1;
        String setop1 = "\"" + String(setopov1) + "\"";  // Отображение kd
        SendData("setop1.txt", setop1);
        
      } else if(setopov1 == 450)
      {
        setopov1 = 0;
        SetOpov1 = setopov1;
        String setop1 = "\"" + String(setopov1) + "\"";  // Отображение kd
        SendData("setop1.txt", setop1);
        
      }
    }
    if (incStr.indexOf("setopzvyk2") >= 0) 
    {
      if (setopov2 < 450)
      {
        setopov2=setopov2+itemp;
        SetOpov2 = setopov2;
        String setop2 = "\"" + String(setopov2) + "\"";  // Отображение kd
        SendData("setop2.txt", setop2);
        
      } else if(setopov2 == 450)
      {
        setopov2 = 0;
        SetOpov2 = setopov2;
        String setop2 = "\"" + String(setopov2) + "\"";  // Отображение kd
        SendData("setop2.txt", setop2);
        
      }
    }

  //**************************

    if (incStr.indexOf("setopzvyk3") >= 0) 
    {
      if (setopov3 < 450)
      {
        setopov3=setopov3+itemp;
        SetOpov3 = setopov3;
        String setop3 = "\"" + String(setopov3) + "\"";  // Отображение 
        SendData("setop3.txt", setop3);
        
      } else if(setopov3 == 450)
      {
        setopov3 = 0;
        SetOpov3 = setopov3;
        String setop3 = "\"" + String(setopov3) + "\"";  // Отображение 
        SendData("setop3.txt", setop3);
        
      }
    }
    if (incStr.indexOf("setopzvyk4") >= 0) 
    {
      if (setopov4 < 450)
      {
        setopov4=setopov4+itemp;
        SetOpov4 = setopov4;
        String setop4 = "\"" + String(setopov4) + "\"";  // Отображение 
        SendData("setop4.txt", setop4);
        
      } else if(setopov4 == 450)
      {
        setopov4 = 0;
        SetOpov4 = setopov4;
        String setop4 = "\"" + String(setopov4) + "\"";  // Отображение 
        SendData("setop4.txt", setop4);
        
      }
    }

  if (incStr.indexOf("ri1") >= 0)
  {
    ri1=1;
    itemp=ri1;
  }   
  if (incStr.indexOf("ri10") >= 0) 
  {
    ri10=10;
    itemp=ri10;
  }  
  if (incStr.indexOf("rio10") >= 0) 
  {
    ri100=100;
    itemp=ri100;
  }    

  if (incStr.indexOf("r1") >= 0)
  {
    r1=1.0;
    rtemp=r1;
  }   
  if (incStr.indexOf("r10") >= 0) 
  {
    r10=10.0;
    rtemp=r10;
  }  
  if (incStr.indexOf("ro10") >= 0) 
  {
    r100=100.0;
    rtemp=r100;
  }    
  if (incStr.indexOf("r01") >= 0) 
  {
    r01=0.1;
    rtemp=r01;
  }   
  if (incStr.indexOf("ro01") >= 0) 
  {
    r001=0.01;
    rtemp=r001;
  } 
  if (incStr.indexOf("rt1") >= 0) 
  {
    r0001=0.001;
    rtemp=r0001;
  }       
  if (incStr.indexOf("rdt1") >= 0) 
  {
    r00001=0.0001;
    rtemp=r00001;
  }  
  if (incStr.indexOf("rst1") >= 0) 
  {
    r000001=0.00001;
    rtemp=r000001;
  }   
   
  if (incStr.indexOf("b2") >= 0) 
  {
    if (termoprofily < 6)
    {
      termoprofily++;
      outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
      termoprofili();

    } else if(termoprofily == 6)
    {
       termoprofily = 0;
       outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
       termoprofili();
    }
  }
   if (incStr.indexOf("b3") >= 0) 
   {
    if (termoprofily > 0)
    {
      termoprofily--;
      outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
      termoprofili();

    } else if(termoprofily == 0)
    {
       termoprofily = 6;
       outNumber("n2.val", termoprofily);  // Отображение числа в числовом компоненте n2
       termoprofili();
    }
  }
  if (incStr.indexOf("hp3") >= 0) 
    {
      if (hup < 10000.0)
      {
       hup=hup+rtemp;
        Hup = hup;
        String thup2 = "\"" + String(hup,2) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
      } else if(hup == 10000.0)
      {
        hup = 0.00;
        Hup = hup;
        String thup2 = "\"" + String(hup,2) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
      }
    }
    if (incStr.indexOf("hm3") >= 0) 
    {
      if (hup > 0.00 )
      {
        hup=hup-rtemp;
        Hup = hup;
        String thup2 = "\"" + String(hup,2) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
      } else if (hup == 0.00)
      {
         hup = 10000.0;
         Hup = hup;
         String thup2 = "\"" + String(hup,2) + "\"";  // Отображение 
         SendData("thup2.txt", thup2);
      }
    }  
    if (incStr.indexOf("kp3") >= 0) 
    {
      if (kup < 10000.0)
      {
        kup=kup+rtemp;
        Kup = kup;
        String tkup2 = "\"" + String(kup,2) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
      } else if(kup == 10000.0)
      {
        kup = 0.00;
        Kup = kup;
        String tkup2 = "\"" + String(kup,2) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
      }
    }
    if (incStr.indexOf("km3") >= 0) 
    {
      if (kup > 0.00 )
      {
        kup=kup-rtemp;
        Kup = kup;
        String tkup2 = "\"" + String(kup,2) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
      } else if (kup == 0.00)
      {
         kup = 10000.0;
         Kup = kup;
         String tkup2 = "\"" + String(kup,2) + "\"";  // Отображение 
         SendData("tkup2.txt", tkup2);
      }
    }  
    if (incStr.indexOf("dtp3") >= 0) 
    {
      if (dtup < 10000)
      {
        dtup=dtup+rtemp;
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
		SendData("tdtup2.txt", tdtup2);
      } else if(dtup == 10000)
      {
        dtup = 0;
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
		SendData("tdtup2.txt", tdtup2);
      }
    }
    if (incStr.indexOf("dtm3") >= 0) 
    {
      if (dtup > 0 )
      {
        dtup=dtup-rtemp;
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
		SendData("tdtup2.txt", tdtup2);
      } else if (dtup == 0)
      {
         dtup = 10000;
         DTup = dtup;
         String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
		 SendData("tdtup2.txt", tdtup2);
      }
    } 
    if (incStr.indexOf("hp4") >= 0) 
    {
      if (hdown < 10000.0)
      {
        hdown=hdown+rtemp;
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
		SendData("thdown2.txt", thdown2);
      } else if(hdown == 10000.0)
      {
        hdown = 0.00;
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
    SendData("thdown2.txt", thdown2);
      }
    }
    if (incStr.indexOf("hm4") >= 0) 
    {
      if (hdown > 0.00 )
      {
        hdown=hdown-rtemp;
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
    SendData("thdown2.txt", thdown2);
      } else if (hdown == 0.00)
      {
         hdown = 10000.0;
         Hdown = hdown;
         String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
		 SendData("thdown2.txt", thdown2);
      }
    }  
    if (incStr.indexOf("kp4") >= 0) 
    {
      if (kdown < 10000.0)
      {
        kdown=kdown+rtemp;
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
      } else if(kdown == 10000.0)
      {
        kdown = 0.00;
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
      }
    }
    if (incStr.indexOf("km4") >= 0) 
    {
      if (kdown > 0.00 )
      {
        kdown=kdown-rtemp;
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
      } else if (kdown == 0.00)
      {
         kdown = 10000.0;
         Kdown = kdown;
         String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
         SendData("tkdown2.txt", tkdown2);
      }
    }  
    if (incStr.indexOf("dtp4") >= 0) 
    {
      if (dtdown < 10000)
      {
        dtdown=dtdown+rtemp;
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
		SendData("tdtdown2.txt", tdtdown2);
      } else if(dtdown == 10000)
      {
        dtdown = 0;
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
		SendData("tdtdown2.txt", tdtdown2);
      }
    }
    if (incStr.indexOf("dtm4") >= 0) 
    {
      if (dtdown > 0 )
      {
        dtdown=dtdown-rtemp;
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
		SendData("tdtdown2.txt", tdtdown2);
      } else if (dtdown == 0)
      {
         dtdown = 10000;
         DTdown = dtdown;
         String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
		 SendData("tdtdown2.txt", tdtdown2);
      }
    }    
}

void termoprofili()
{
      if (termoprofily == 0)
      {
         shag = 0;
         sec = 3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
         profily="Lead-free"; // Термопрофиль Бессвинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
          
      } else if (termoprofily == 1)
      {
         shag = 0;
         sec = 3;
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec        
         profily="Lead";  // Термопрофиль Свинец
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         temp1 = 195; // Верхний нагреватель Бессвинцовый выбрано 195 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
          
      }else if (termoprofily == 2)
      {
         EEPROM.get(227, dtv);	   
         EEPROM.get(232, dtn); 
         EEPROM.get(22, sec);
         EEPROM.get(27, temp1);
         EEPROM.get(32, temp2);
             //термопрофиль User 1 шаг 0
        EEPROM.get(3200, gradsecv);
        EEPROM.get(3205, gradsecn);
        //термопрофиль User 1 шаг 0
        EEPROM.get(3746, pwmv);
        EEPROM.get(3747, pwmn);
        // Оповещение температуры User 1
        EEPROM.get(3856,setopov1);
        EEPROM.get(3861,setopov2);
        EEPROM.get(3866,setopov3);
        EEPROM.get(3871,setopov4);
        EEPROM.get(3956,hup);
        EEPROM.get(3961,kup);
        EEPROM.get(3966,dtup);

        EEPROM.get(3971,hdown);
        EEPROM.get(3976,kdown);
        EEPROM.get(3981,dtdown);
         shag = 0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
         profily="User 1";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Dtv = dtv;
         String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
         SendData("t57.txt", t57);
         Dtn = dtn;
         String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
         SendData("t58.txt", t58);
         GradSecv = gradsecv;
         String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
         SendData("tgcs3.txt", tgcs3);
         GradSecn = gradsecn;
         String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
         SendData("tgcs4.txt", tgcs4);
         String setop1 = "\"" + String(setopov1) + "\"";  // Отображение kd
        SendData("setop1.txt", setop1);
        String setop2 = "\"" + String(setopov2) + "\"";  // Отображение kd
        SendData("setop2.txt", setop2);
        String setop3 = "\"" + String(setopov3) + "\"";  // Отображение kd
        SendData("setop3.txt", setop3);
        String setop4 = "\"" + String(setopov4) + "\"";  // Отображение kd
        SendData("setop4.txt", setop4);
        Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
         outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
        pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         
          
      }else if (termoprofily == 3)
      {
         	//термопрофиль User 2
         EEPROM.get(1465, dtv); 
         EEPROM.get(1470, dtn); 
         //термопрофиль User 2 шаг 0
         EEPROM.get(1300, sec);
         EEPROM.get(1305, temp1);
         EEPROM.get(1310, temp2);
             //термопрофиль User 2 шаг 0
          EEPROM.get(3310, gradsecv);
          EEPROM.get(3315, gradsecn);
          //термопрофиль User 2 шаг 0
          EEPROM.get(3768, pwmv);
          EEPROM.get(3769, pwmn);
          // Оповещение температуры User 2
          EEPROM.get(3876,setopov1);
          EEPROM.get(3881,setopov2);
          EEPROM.get(3886,setopov3);
          EEPROM.get(3891,setopov4);
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);

          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown);
         shag = 0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
         profily="User 2";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Dtv = dtv;
         String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
         SendData("t57.txt", t57);
         Dtn = dtn;
         String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
         SendData("t58.txt", t58);
         GradSecv = gradsecv;
         String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
         SendData("tgcs3.txt", tgcs3);
         GradSecn = gradsecn;
         String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
         SendData("tgcs4.txt", tgcs4);
        String setop1 = "\"" + String(setopov1) + "\"";  // Отображение kd
        SendData("setop1.txt", setop1);
        String setop2 = "\"" + String(setopov2) + "\"";  // Отображение kd
        SendData("setop2.txt", setop2);
        String setop3 = "\"" + String(setopov3) + "\"";  // Отображение kd
        SendData("setop3.txt", setop3);
        String setop4 = "\"" + String(setopov4) + "\"";  // Отображение kd
        SendData("setop4.txt", setop4);
        Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
         outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
        pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         
          
      }else if (termoprofily == 4)
      {
        //термопрофиль User 3
        //delay(10);
          EEPROM.get(1940, dtv); 
          EEPROM.get(1945, dtn); 
        //термопрофиль User 3 шаг 0
          EEPROM.get(1775, sec);
          EEPROM.get(1780, temp1);
          EEPROM.get(1785, temp2);
            //термопрофиль User 3 шаг 0
          EEPROM.get(3420, gradsecv);
          EEPROM.get(3425, gradsecn); 
          //термопрофиль User 3 шаг 0
          EEPROM.get(3790, pwmv);
          EEPROM.get(3791, pwmn);
          // Оповещение температуры User 3
          EEPROM.get(3896,setopov1);
          EEPROM.get(3901,setopov2);
          EEPROM.get(3906,setopov3);
          EEPROM.get(3911,setopov4); 
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);

          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown);        
         shag = 0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
         profily="User 3";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Dtv = dtv;
         String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
         SendData("t57.txt", t57);
         Dtn = dtn;
         String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
         SendData("t58.txt", t58);
         GradSecv = gradsecv;
         String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
         SendData("tgcs3.txt", tgcs3);
         GradSecn = gradsecn;
         String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
         SendData("tgcs4.txt", tgcs4);
          String setop1 = "\"" + String(setopov1) + "\"";  // Отображение kd
        SendData("setop1.txt", setop1);
        String setop2 = "\"" + String(setopov2) + "\"";  // Отображение kd
        SendData("setop2.txt", setop2);
        String setop3 = "\"" + String(setopov3) + "\"";  // Отображение kd
        SendData("setop3.txt", setop3);
        String setop4 = "\"" + String(setopov4) + "\"";  // Отображение kd
        SendData("setop4.txt", setop4);
        Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
         outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
        pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
         
          
      }else if (termoprofily == 5)
      {
        //термопрофиль User 4
        //delay(10);
          EEPROM.get(2415, dtv); 
          EEPROM.get(2420, dtn); 
        //термопрофиль User 4 шаг 0
          EEPROM.get(2250, sec);
          EEPROM.get(2255, temp1);
          EEPROM.get(2260, temp2);
            //термопрофиль User 4 шаг 0
          EEPROM.get(3530, gradsecv);
          EEPROM.get(3535, gradsecn); 
          //термопрофиль User 4 шаг 0
          EEPROM.get(3812, pwmv);
          EEPROM.get(3813, pwmn); 
          // Оповещение температуры User 4
          EEPROM.get(3916,setopov1);
          EEPROM.get(3921,setopov2);
          EEPROM.get(3926,setopov3);
          EEPROM.get(3931,setopov4); 
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);

          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown);      
         shag = 0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
         profily="User 4";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Dtv = dtv;
         String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
         SendData("t57.txt", t57);
         Dtn = dtn;
         String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
         SendData("t58.txt", t58);
         GradSecv = gradsecv;
         String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
         SendData("tgcs3.txt", tgcs3);
         GradSecn = gradsecn;
         String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
         SendData("tgcs4.txt", tgcs4);
          String setop1 = "\"" + String(setopov1) + "\"";  // Отображение kd
        SendData("setop1.txt", setop1);
        String setop2 = "\"" + String(setopov2) + "\"";  // Отображение kd
        SendData("setop2.txt", setop2);
        String setop3 = "\"" + String(setopov3) + "\"";  // Отображение kd
        SendData("setop3.txt", setop3);
        String setop4 = "\"" + String(setopov4) + "\"";  // Отображение kd
        SendData("setop4.txt", setop4);
        Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        
         outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
        pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      }else if (termoprofily == 6)
      {
        //термопрофиль User 5
        //delay(10);
          EEPROM.get(2890, dtv); 
          EEPROM.get(2895, dtn); 
        //термопрофиль User 5 шаг 0
          EEPROM.get(2725, sec);
          EEPROM.get(2730, temp1);
          EEPROM.get(2735, temp2);
            //термопрофиль User 5 шаг 0
          EEPROM.get(3640, gradsecv);
          EEPROM.get(3645, gradsecn); 
          //термопрофиль User 5 шаг 0
          EEPROM.get(3834, pwmv);
          EEPROM.get(3835, pwmn);
          // Оповещение температуры User 5
          EEPROM.get(3936,setopov1);
          EEPROM.get(3941,setopov2);
          EEPROM.get(3946,setopov3);
          EEPROM.get(3951,setopov4); 
          EEPROM.get(3956,hup);
          EEPROM.get(3961,kup);
          EEPROM.get(3966,dtup);

          EEPROM.get(3971,hdown);
          EEPROM.get(3976,kdown);
          EEPROM.get(3981,dtdown);       
         shag = 0;         
         outNumber("shag.val", shag);  // Отображение числа в числовом компоненте shag
         outNumber("sec.val", sec);  // Отображение числа в числовом компоненте sec         
         profily="User 5";
         String t13= "\"" + String(profily) + "\"";  // Отображение 
         SendData("t13.txt", t13);
         Dtv = dtv;
         String t57= "\"" + String(Dtv) + "\"";  // выводим dt в мксекундах
         SendData("t57.txt", t57);
         Dtn = dtn;
         String t58= "\"" + String(Dtn) + "\"";  // выводим dt в мксекундах
         SendData("t58.txt", t58);
         GradSecv = gradsecv;
         String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
         SendData("tgcs3.txt", tgcs3);
         GradSecn = gradsecn;
         String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
         SendData("tgcs4.txt", tgcs4);
          String setop1 = "\"" + String(setopov1) + "\"";  // Отображение kd
        SendData("setop1.txt", setop1);
        String setop2 = "\"" + String(setopov2) + "\"";  // Отображение kd
        SendData("setop2.txt", setop2);
        String setop3 = "\"" + String(setopov3) + "\"";  // Отображение kd
        SendData("setop3.txt", setop3);
        String setop4 = "\"" + String(setopov4) + "\"";  // Отображение kd
        SendData("setop4.txt", setop4);
        Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	    
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        
         outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
        pwmust2 = pwmn;
         //temp1 = 225; // Верхний нагреватель Бессвинцовый выбрано 225 'C градусов
         outNumber("temp1.val", temp1);  // Отображение числа в числовом компоненте temp1
         tempust1 = temp1;
         //temp2 = 160; // Нижний нагреватель Бессвинцовый выбрано 160 'C градусов
         outNumber("temp2.val", temp2);  // Отображение числа в числовом компоненте temp1
         tempust2 = temp2;
      }
    
}


void Text()
{
         // Пид Верхнего нагревателя
	      Kpv = kpv;
        String t24 = "\"" + String(Kpv,5) + "\"";  // выводим пропорциональное
        SendData("t24.txt", t24);
	      Kiv = kiv;
        String t25 = "\"" + String(Kiv,5) + "\"";  // выводим интегральное
        SendData("t25.txt", t25);
	      Kdv = kdv;
        String t26= "\"" + String(Kdv,5) + "\"";  // выводим дефференциальное
        SendData("t26.txt", t26);
	      // Пид Нижнего нагревателя
        delay(10);
	      Kpn = kpn;
	      String t54 = "\"" + String(Kpn,5) + "\"";  // выводим пропорциональное
        SendData("t54.txt", t54);
	      Kin = kin;
        String t55 = "\"" + String(Kin,5) + "\"";  // выводим интегральное
        SendData("t55.txt", t55);
	      Kdn = kdn;
        String t56= "\"" + String(Kdn,5) + "\"";  // выводим дефференциальное
        SendData("t56.txt", t56);
        delay(10);
        
        // Градус в секунду верхний и нижний нагреватели
        GradSecv = gradsecv;
        String tgcs3 = "\"" + String(gradsecv) + "\"";  // Отображение kd
        SendData("tgcs3.txt", tgcs3);
        GradSecn = gradsecn;
        String tgcs4 = "\"" + String(gradsecn) + "\"";  // Отображение kd
        SendData("tgcs4.txt", tgcs4);
        delay(10);
        Hup = hup;
        String thup2 = "\"" + String(hup) + "\"";  // Отображение 
        SendData("thup2.txt", thup2);
        Kup = kup;
        String tkup2 = "\"" + String(kup) + "\"";  // Отображение 
        SendData("tkup2.txt", tkup2);
        DTup = dtup;
        String tdtup2 = "\"" + String(dtup) + "\"";  // Отображение 
        SendData("tdtup2.txt", tdtup2);   
	      delay(10);
        Hdown = hdown;
        String thdown2 = "\"" + String(hdown) + "\"";  // Отображение 
        SendData("thdown2.txt", thdown2);
        Kdown = kdown;
        String tkdown2 = "\"" + String(kdown) + "\"";  // Отображение 
        SendData("tkdown2.txt", tkdown2);
        DTdown = dtdown;
        String tdtdown2 = "\"" + String(dtdown) + "\"";  // Отображение 
        SendData("tdtdown2.txt", tdtdown2);
        delay(10);
        outNumber("pwmv.val", pwmv);  // Отображение числа в числовом компоненте pwmv
        pwmust1 = pwmv;
        outNumber("pwmn.val", pwmn);  // Отображение числа в числовом компоненте pwmv
        pwmust2 = pwmn;
	      
}

void SendData(String dev, String data)
{
  nexSerial.print(dev);
  nexSerial.print("=");
  nexSerial.print(data);
  nexSerial.write(0xff);  // 3 байта 0xFF отправляем в конце подтверждение дисплею Nextion 
  nexSerial.write(0xff);
  nexSerial.write(0xff);
}
/**
void compSendData(String dev, String data)
{
  compSerial.print(dev);
  compSerial.print("=");
  compSerial.print(data);
  compSerial.print("\n");
}
**/

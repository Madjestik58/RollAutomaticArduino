//version without RCSwitch library. For EV1527 Reciever
#include <SoftwareSerial.h>
#include <Arduino.h>

//количество граничных оборотов для шторы
#define rulonCOUNT 100
#define calibrovkaCOUNT 3


//входы

#define sensorSMK 5 // если нетмагнита, то логическая 1, если есть то 0 Датчик калибровки
#define sensorHALL 6 // если есть магнит, то логический 0, если нет то 1 Датчик оборотов

#define l298nIN1 9
#define l298nIN2 8
#define STBY 10

#define ev1527D0  2 // задание для пина приемника выход 0 ВНИЗ
#define ev1527D1  3 // задание для пина приемника выход 1 ВВЕРХ
#define ev1527D2  4 // задание для пина приемника выход 2 СТОП
#define ev1527D3  7 // задание для пина приемника выход 2 СТОП

//-------------Таймеры
unsigned long currentTime = 0;
unsigned long AutoStop = 0;
#define TIMEOUT_STOP 60000  //время отключения


//------------хранение переменных
int hallSTATUS = 0; //флаг Холл верхнего цифрового
int smkSTATUS = 1; //флаг микрика  1 - нажат, 0 - не нажат
int currentHallPush = 0; // - фактическое значение оборотов намомент остановки

byte ev1527CURRENTD0 = 0; //текущее состояние пина D0 приемника
byte ev1527CURRENTD1 = 0; //текущее состояние пина D1 приемника
byte ev1527CURRENTD2 = 0; //текущее состояние пина D2 приемника
byte ev1527CURRENTD3 = 0; //текущее состояние пина D3 приемника
boolean ev1527flagD0 = false; //флаг EV1527 D0  ВНИЗ
boolean ev1527flagD1 = false; //флаг EV1527 D1  ВВЕРХ
boolean ev1527flagD2 = false; //флаг EV1527 D2  СТОП
boolean ev1527flagD3 = false; //флаг EV1527 D3  Калибровка

boolean calibrovkaFLAG = false; //статус старта режима калибровки
boolean calibrovkaFLAGSEC = false; //второй этап калибровки
boolean calibrovkaOK = false; //подтверждение успешной калибровки


//-----переменные для счетчика
int hallPushCounter = 0;   // счетчик для количества срабатываний датчика Холла
int hallState = 0;         // текущее состояние датчика Холла
int lastHallState = 0;     // предыдущее состояние датчика Холла

int smkState = 0;         // текущее состояние датчика Холла




void setup() {
  Serial.begin(57600);
  pinMode(l298nIN1,OUTPUT);  digitalWrite(l298nIN1,LOW); // движок спит
  pinMode(l298nIN2,OUTPUT); digitalWrite(l298nIN2,LOW); // движок спит
  pinMode(STBY, OUTPUT);// включить микросхему

  pinMode(sensorSMK, INPUT); pinMode(sensorHALL, INPUT);
  pinMode(ev1527D0, INPUT); pinMode(ev1527D1, INPUT); pinMode(ev1527D2, INPUT); pinMode(ev1527D3, INPUT);
  digitalWrite(STBY, LOW); // выключить микросхему 
 }
    

void loop()
{
ev1527read();
counterDOWN();
motor();
delay(10);

}

void counterDOWN()
{
hallState = digitalRead(sensorHALL);  // считываем данные с датчика холла

  if (ev1527flagD0 == true) {
hallState = digitalRead(sensorHALL);  // считываем данные с датчика холла
      // сравниваем текущее состояние (buttonState) с предыдущим:
   if (hallState != lastHallState) {
   // если состояние изменилось, увеличиваем значение счетчика:
   
    if (hallState == LOW) {
      // если текущее состояние равно LOW,
      // переводим кнопку из состояния нет магнит в есть магнит
      hallPushCounter++;
      Serial.print("number of hall pushes:  ");
      Serial.println(hallPushCounter);
     
    }
   
  }
lastHallState = hallState;}

 if (ev1527flagD1 == true) {
hallState = digitalRead(sensorHALL);  // считываем данные с датчика холла
      // сравниваем текущее состояние (buttonState) с предыдущим:
   if (hallState != lastHallState) {
   // если состояние изменилось, увеличиваем значение счетчика:
   
    if (hallState == LOW) {
      // если текущее состояние равно LOW,
      // переводим кнопку из состояния нет магнит в есть магнит
      hallPushCounter--;
      Serial.print("number of hall pushes:  ");
      Serial.println(hallPushCounter);
       
    }
      }
lastHallState = hallState;}

 if (ev1527flagD3 == true) {
hallState = digitalRead(sensorHALL);  // считываем данные с датчика холла
      // сравниваем текущее состояние (buttonState) с предыдущим:
         if (hallState != lastHallState) {
   // если состояние изменилось, увеличиваем значение счетчика:
   
    if (hallState == LOW) {
      // если текущее состояние равно LOW,
      // переводим кнопку из состояния нет магнит в есть магнит
      hallPushCounter++;
      Serial.print("number of hall pushes:  ");
      Serial.println(hallPushCounter);
    }
   
  }
lastHallState = hallState;}

  
}

void motor()
{
 // процедура опускания
if (ev1527flagD0 == true && calibrovkaOK == true) { digitalWrite(STBY, HIGH); 
  if(hallPushCounter < rulonCOUNT && hallPushCounter >= -1 ) {digitalWrite(l298nIN1,LOW); digitalWrite(l298nIN2,HIGH); }
  if(hallPushCounter >= rulonCOUNT) {digitalWrite(l298nIN1,LOW); digitalWrite(l298nIN2,LOW); delay(10); digitalWrite(STBY, LOW); }
     } 

// процедура подъема
if (ev1527flagD1 == true && calibrovkaOK == true) { digitalWrite(STBY, HIGH); 
  if(hallPushCounter <= rulonCOUNT+3 && hallPushCounter > 0 ) {digitalWrite(l298nIN1,HIGH); digitalWrite(l298nIN2,LOW);}
  smkState = digitalRead(sensorSMK);  // считываем данные с микрика
  if (smkState == 0) {digitalWrite(l298nIN1,LOW); digitalWrite(l298nIN2,LOW); delay(10); digitalWrite(STBY, LOW); hallPushCounter = 0;} // если отжалост СМК, то отсанавливаем. Верхнее положение
  if(hallPushCounter <= -3) {   digitalWrite(l298nIN1,LOW); digitalWrite(l298nIN2,LOW); delay(10); digitalWrite(STBY, LOW); hallPushCounter = 0; }
 
  }
 // процедура остановки
if (ev1527flagD2 == true) {
  
  if(hallPushCounter != hallPushCounter-1) { digitalWrite(l298nIN1,LOW); digitalWrite(l298nIN2,LOW); delay(10);digitalWrite(STBY, LOW); } 
  if(hallPushCounter != hallPushCounter+1) { digitalWrite(l298nIN1,LOW); digitalWrite(l298nIN2,LOW); delay(10); digitalWrite(STBY, LOW); }}

   //цепь безопасности по времени 
if ((currentTime + TIMEOUT_STOP )<millis()) {digitalWrite(l298nIN1,LOW); digitalWrite(l298nIN2,LOW); currentTime = millis(); ev1527flagD0 = false; ev1527flagD1 = false; ev1527flagD2 = false; ev1527flagD3 = false;}  
 
 //процедура калибровки
if (ev1527flagD3 == true) { digitalWrite(STBY, HIGH); 
   if(hallPushCounter < calibrovkaCOUNT && hallPushCounter >= 0 ) {digitalWrite(l298nIN1,LOW); digitalWrite(l298nIN2,HIGH);}
   if(hallPushCounter >= calibrovkaCOUNT) {digitalWrite(l298nIN1,LOW); digitalWrite(l298nIN2,LOW); calibrovkaFLAGSEC = true; Serial.print("Опущено вниз на: "); Serial.println(calibrovkaCOUNT);}
   if(calibrovkaFLAGSEC == true)
   {smkState = digitalRead(sensorSMK);  // считываем данные с микрика
        if (smkState == 1) {digitalWrite(l298nIN1,HIGH); digitalWrite(l298nIN2,LOW); Serial.println("Ищем ноль");} // если нажата СМК, то поднимаем штору
        if (smkState == 0) {digitalWrite(l298nIN1,LOW); digitalWrite(l298nIN2,LOW);calibrovkaOK = true; calibrovkaFLAGSEC = false; ev1527flagD3 =false; hallPushCounter = 0; Serial.println("НОЛЬ найден"); delay(10); digitalWrite(STBY, LOW);} // если отжалост СМК, то отсанавливаем. Верхнее положение
    
   }
   
   }
   // Serial.print("hallPushCounter:"); Serial.println(hallPushCounter);
   
 
 } 
 

void ev1527read()
{
ev1527CURRENTD0 = digitalRead(ev1527D0); 
ev1527CURRENTD1 = digitalRead(ev1527D1); 
ev1527CURRENTD2 = digitalRead(ev1527D2);
ev1527CURRENTD3 = digitalRead(ev1527D3);
//Serial.print("D0:");Serial.print(ev1527CURRENTD0); Serial.print(" D1:");Serial.print(ev1527CURRENTD1);Serial.print(" D2:");Serial.print(ev1527CURRENTD2);Serial.print(" D3:");Serial.println(ev1527CURRENTD3);
if (ev1527CURRENTD0 == 1 && calibrovkaOK == true) {ev1527flagD0 = true; ev1527flagD1 = false; ev1527flagD2 = false; ev1527flagD3 = false; currentTime=millis();} //ВНИЗ
if (ev1527CURRENTD1 == 1 && calibrovkaOK == true) {ev1527flagD1 = true; ev1527flagD0 = false; ev1527flagD2 = false; ev1527flagD3 = false; currentTime=millis();} //ВВЕРХ
if (ev1527CURRENTD2 == 1) {ev1527flagD2 = true; ev1527flagD0 = false; ev1527flagD1 = false; ev1527flagD3 = false; calibrovkaFLAG = false;} //СТОП
if (ev1527CURRENTD3 == 1) {ev1527flagD3 = true; ev1527flagD0 = false; ev1527flagD1 = false; ev1527flagD2 = false; hallPushCounter = 0; calibrovkaFLAG = true; calibrovkaOK = false; Serial.println("Калибровка старт");} //Калибровка
}


  



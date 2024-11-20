// Черновик.  ХардБАСС, устройство для передачи данных на расстояние 1.0
// Эта нода: Клиент C1
// Основано на протоколе LoRa, представляет собой сеть из последовательно соединенных роутеров
// Топология сети: Клиент C1 <-> Сервер S1 <-> Сервер S2 <-> Сервер S3 <-> Сервер S4
// Клиент C1 и  сервер S4 не имеют прямой связи между собой, нужны "прослойки"  S1 - S3
// Передача не работает, если отключен хотя бы один из серверов, будьте внимательны 
// Разработано для работы с SERVER* скетчами
// Большинство Serial.print() можно смело удалять, если вы настроили всё верно, они несут лишь debug-функцию

#include <RHSoftwareSPI.h>
RHSoftwareSPI spi;
#include <RHRouter.h>
#include <RH_RF95.h>
#include <SPI.h>

// LED пины для индикации передачи (могут быть любыми, как удобно)
int led_g = 3;
int led_r = 12;

// В этой небольшой искусственной сети из 5 узлов сообщения направляются 
// через промежуточные узлы к узлу назначения. 
// Все узлы могут выступать в роли маршрутизаторов
// CLIENT_ADDRESS <-> SERVER1_ADDRESS <-> SERVER2_ADDRESS <-> SERVER3_ADDRESS <-> SERVER4_ADDRESS
#define CLIENT_ADDRESS 1
#define SERVER1_ADDRESS 2
#define SERVER2_ADDRESS 3
#define SERVER3_ADDRESS 4
#define SERVER4_ADDRESS 5

/*// Для Arduino
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2 //*/

// Для ESP32 
#define RFM95_CS 5
#define RFM95_RST 14
#define RFM95_INT 2 //*/

// Переключитесь на 434.0 или другую частоту, она должна совпадать с частотой RX!
#define RF95_FREQ 915.0
// Экземпляр радио-драйвера, нужен для назначения выводов NSS, INIT
RH_RF95 driver(RFM95_CS, RFM95_INT);
// Класс для управления доставкой и получением сообщений, использующий драйвер, объявленный выше
RHRouter *manager;
// Простой счётчик для посыла сообщений
int count = 0;

void setup() 
{  
  pinMode(led_g, OUTPUT); 
  pinMode(led_r, OUTPUT); 

  Serial.begin(9600);
  // Настройте пины MOSI, MISO, SCK под вашу плату. В этом примере плата ESP32-S2-LOLIN-MINI
  spi.setPins(11, 9, 7);

  manager = new RHRouter(driver, CLIENT_ADDRESS);
  // Инициализация
  // Стандартные значения после инициализации: 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  if (!manager->init()){
    Serial.println("init failed");
    digitalWrite(led_r, HIGH);
    delay(10);}
    else{
    led_G();
    }

  // Назначаем свои параметры для RFM95W
  driver.setTxPower(23, false);
  driver.setFrequency(RF95_FREQ);
  driver.setCADTimeout(0);
  driver.setSpreadingFactor(9);
  driver.setSignalBandwidth(250E3);

  // Ручное определение маршрутов для этой сети
    manager->addRouteTo(SERVER1_ADDRESS, SERVER1_ADDRESS);  
    manager->addRouteTo(SERVER4_ADDRESS, SERVER1_ADDRESS);

  Serial.println("Нода Клиент C1: работает, ок.");
  led_G();
}

uint8_t data[] = "Hello From Client!";
// Dont put this on the stack:
uint8_t buf[RH_ROUTER_MAX_MESSAGE_LEN];

void loop()
{
  Serial.println("Посылка для SERVER_4 - ушла");
  
  if (count >= 1000) while(1);
  // Отправляем сообщение на SERVER_N
  // Оно будет направлено промежуточными
  // узлами до узла назначения, в соответствии с
  // таблицей маршрутизации каждого узла
  int errorLog = manager->sendtoWait(data, sizeof(data), SERVER4_ADDRESS);
  Serial.println("Debug::loop()::errorLog = " + (String) errorLog);
  
  if (errorLog == RH_ROUTER_ERROR_NONE)
  {
    // Сообщение было надежно доставлено на следующий узел.
    // Теперь ожидайте ответа от конечного сервера
    uint8_t len = sizeof(buf);
    uint8_t from;    
    count++;
    Serial.println("Count Sent: " + (String) count + "/1000");
    
    if (manager->recvfromAckTimeout(buf, &len, 3000, &from))
    {
      Serial.print(" Получен ответ от : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
      Serial.println("DEBUG::manager->recvfromAckTimeout");
      led_G();
      
    }
    else
    {
      Serial.println("Нет ответа, запущены ли SERVER_1, SERVER_2, SERVER_3?");
      Serial.println("DEBUG::manager->recvfromAckTimeout else");
      led_R2();
    }
  }
  else{
    Serial.println("Ошибка sendtoWait. Работают ли промежуточные серверы?");
    led_R();
  }
}
// Функции для индикации с помощью LED
void led_G(){
    digitalWrite(led_g, HIGH);
    delay(100);
    digitalWrite(led_g, LOW);
    delay(100);
}

void led_R(){
    digitalWrite(led_r, HIGH);
    delay(100);
    digitalWrite(led_r, LOW);
    delay(100);
}

void led_R2(){
    digitalWrite(led_r, HIGH);
    delay(500);
    digitalWrite(led_r, LOW);
    delay(500);
}
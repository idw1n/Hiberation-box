#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ESP8266WebServer.h>
#include "DHT.h"

#define lamp_pin D7


LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DHTPIN 14
//#define DHTTYPE           DHT11     // DHT 11 
#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

// Replace with your network credentials
const char* ssid = "Keenetic";  // SSID
const char* password = "TMKiwGPy"; // пароль

const long interval_time = 1000;
const long interval_dht = 10000;
const long interval_day = 3600000;
unsigned long millis_time = 0;
unsigned long millis_dht = 0;
unsigned long millis_day = 0;
unsigned long millis_led = 0;

IPAddress ip(192,168,1,17);  //IP adres dlia prosmotra grafika web-brayzerom
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
ESP8266WebServer server(80);

String SendHTML(float Temperaturestat, float Humiditystat, int GetTime_hour, int GetTime_minute, boolean lamp_status)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Hibernation Box</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>Hibernation Box. Status</h1>\n";

  
  ptr +="<p>Time: ";
  ptr +=(int)GetTime_hour;
  ptr +=":";
  ptr +=(int)GetTime_minute;
  ptr +="<p>Temperature: ";
  ptr +=(int)Temperaturestat;
  ptr +=" C</p>";
  ptr +="<p>Humidity: ";
  ptr +=(int)Humiditystat;
  ptr +="%</p>";
  ptr +="<p>Lamp: ";
  (lamp_status==1) ? ptr +="ON" : ptr +="OFF";
  
  
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

const long utcOffsetInSeconds = 25200;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Определение NTP-клиента для получения времени
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

boolean flag_lamp();
void setup()
{
  pinMode(lamp_pin, OUTPUT); // see #define lamp_pin 
  digitalWrite(lamp_pin, 0);
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  timeClient.begin();
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
  dht.begin();

// connect lcd display
Wire.begin(D2, D1);   //Use predefined PINS consts
lcd.begin(20,4);      // The begin call takes the width and height. This
                      // Should match the number provided to the constructor.

lcd.backlight();      // Turn on the backlight.

lcd.home();
}

void handleRoot() {
  float Temperature = dht.readTemperature(); // получить значение температуры
  float Humidity = dht.readHumidity();       // получить значение влажности
  int Now_hour = timeClient.getHours();
  int Now_minutes = timeClient.getMinutes();
  server.send(200, "text/html", SendHTML(Temperature, Humidity, Now_hour, Now_minutes, flag_lamp())); 
}

void loop() 
{
  timeClient.update();

  server.handleClient();

  
  //  on/off Lamp
  if (millis() - millis_time >= 10000) {
    digitalWrite(lamp_pin, flag_lamp());
    millis_time = millis();
  } 
  
  print_day();
  print_time();
  print_dht();
  print_led();  
}

 boolean flag_5sec() {  //set time working lamp//Flag on/off lamp  
    return (timeClient.getSeconds()%10 >= 5)
    ? 1 : 0;
  }
void print_day() {
  lcd.setCursor(5, 0);  // Move the cursor at origin
  lcd.print(daysOfTheWeek[timeClient.getDay()]);
}
void print_time() {
lcd.setCursor(6, 1);  // Move the cursor at origin
  lcd.print(timeClient.getFormattedTime());
}
void print_dht() {
  lcd.setCursor(1, 2);  // Move the cursor at origin
  lcd.print("H=");
  lcd.setCursor(3, 2);  // Move the cursor at origin
  lcd.print(dht.readHumidity());
  lcd.setCursor(11, 2);  // Move the cursor at origin
  lcd.print("t=");
  lcd.setCursor(13, 2);  // Move the cursor at origin
  lcd.print(dht.readTemperature());
}
void print_led() {
    lcd.setCursor(9, 3);  // Move the cursor at origin
    (flag_lamp() == 1) ? lcd.print("led.on") : lcd.print("led.off");
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

boolean flag_lamp() {  //set time working lamp//Flag on/off lamp  
    char _start = 6;
    char _finish = 22;
    return (timeClient.getHours()>=_start && timeClient.getHours()<=_finish) ? 1 : 0;
  }
  

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ESP8266WebServer.h>
#include "DHT.h"

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



const long utcOffsetInSeconds = 25200;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Определение NTP-клиента для получения времени
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup()
{
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
void loop() 
{
  timeClient.update();

  server.handleClient();
  
  /*
  if (millis() - millis_time >= interval_time) {
    millis_time = millis();
    print_time();
  }*/
  print_day();
  print_time();
  print_dht();
  print_led();  
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
    if (timeClient.getHours()>6 && timeClient.getHours()<22) {
  lcd.print("led.on");
  } else {
    lcd.print("led.off");
  }
}

void handleRoot() {
  const uint16_t lengt=1440; //max kol tochek trenda
uint16_t tick=0;
String date_write = timeClient.getFormattedTime();
float h[lengt], t[lengt];
  String trendstr;
  
  trendstr = F("<html>\
  <head>\
    <script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>\
    <meta http-equiv='refresh' content='1000'/>\
    <title>Daily temperature</title>\
    <script type='text/javascript'>\
      google.charts.load('current', {'packages':['corechart']});\
      google.charts.setOnLoadCallback(drawChart);\
\
      function drawChart() {\
        var data = new google.visualization.DataTable();\
      data.addColumn('datetime', 'Time');\
      data.addColumn('number', 'Temp, C');\
      data.addColumn('number', 'Humidity, %');\
\
      data.addRows([");
  uint16_t k, y=0;
  for (int i=1; i <= lengt; i++){
    k = tick-1 + i;
    if (h[k]>0){
      if (y>0) trendstr += ",";
      y ++;
      if (k>lengt-1) k = k - lengt;
      trendstr += "[new Date(";
      trendstr += String(date_write[k]-(7*3600)); //2- chasovoy poyas
      trendstr += "*1000), ";
      trendstr += t[k];
      trendstr += ", ";
      trendstr += h[k];
      trendstr += "]";
    }
  }
    trendstr += F("]);\
\
        var options = {width: '100%',\
          title: 'Hibernation Box',\
          curveType: 'function',\
          legend: { position: 'bottom' },\
          hAxis: {format: 'HH:mm',\
          gridlines: {\
            count: 10,\
          },\
        }\
        };\
\
        var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));\
        var formatter = new google.visualization.DateFormat({pattern: 'HH:mm'});\
        formatter.format(data, 0);\
        chart.draw(data, options);\
      }\
    </script>\
  </head>\
  <body>\
    <div id='curve_chart' style='width: 100%; height: 600px'></div>\
  </body>\
</html>");
  server.send ( 200, F("text/html"), trendstr );
}

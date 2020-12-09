#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//wifi module
#include <ESP8266WiFi.h>

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = ""; //TODO REMOVE before push to github
const char* password = ""; //TODO REMOVE before push to github

const String API_KEY = ""; //add your own accuweather API KEY
String city = "Authezat"; // add your prefered city
String units = "metric"; // (options: metric/imperial )

long weatherDataTimer = 0;


void setup() {
  Serial.begin(74880);

  Wire.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed\n"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();
  Serial.print("Starting ..\n");
  scrolltext("Starting ..");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting..\n");
    scrolltext("Connecting..");
  }

  scrolltext("Connected");
  Serial.print("Connected\n");
  String ip = WiFi.localIP().toString().c_str();
  Serial.print("IP Address: \n");
  Serial.println(ip);
  displaytext("IP Address:\n" + ip);
  delay(4000);
  
  display.clearDisplay();
  
  Serial.println("initWeather\n");
  initWeather();

}

void loop() {
    if (millis() - weatherDataTimer > 60000) {
    initWeather();
    weatherDataTimer = millis();
  }

}


void initWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    scrolltext("Disconnected");
  } else {
    Serial.println("getWeatherData\n");
    getWeatherData();
  }
}

void getWeatherData() {
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&units=" + units + "&APPID=" + API_KEY;

  HTTPClient http;  //Declare an object of class HTTPClient
  http.begin(url);  //Specify request destination
  int httpCode = http.GET();//Send the request
  if (httpCode > 0) { //Check t he returning code
    String payload = http.getString();   //Get the request response payload
    //parse data
    parseWeatherData(payload);
  }
  http.end();//Close connection
}


void parseWeatherData(String payload) {
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(11) + 500;
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.parseObject(payload);

  float coord_lon = root["coord"]["lon"]; // 25.61
  float coord_lat = root["coord"]["lat"]; // 45.65

  JsonObject& weather_0 = root["weather"][0];
  int weather_0_id = weather_0["id"]; // 803
  const char* weather_0_main = weather_0["main"]; // "Clouds"
  const char* weather_0_description = weather_0["description"]; // "broken clouds"
  const char* weather_0_icon = weather_0["icon"]; // "04d"

  const char* base = root["base"]; // "stations"

  JsonObject& main = root["main"];
  float main_temp = main["temp"]; // -6.04
  float main_pressure = main["pressure"]; // 1036.21
  int main_humidity = main["humidity"]; // 65
  float main_temp_min = main["temp_min"]; // -6.04
  float main_temp_max = main["temp_max"]; // -6.04
  float main_sea_level = main["sea_level"]; // 1036.21
  float main_grnd_level = main["grnd_level"]; // 922.42

  float wind_speed = root["wind"]["speed"]; // 1.21
  float wind_deg = root["wind"]["deg"]; // 344.501

  int clouds_all = root["clouds"]["all"]; // 68

  long dt = root["dt"]; // 1551023165

  JsonObject& sys = root["sys"];
  float sys_message = sys["message"]; // 0.0077
  const char* sys_country = sys["country"]; // COUNTRY
  long sys_sunrise = sys["sunrise"]; // 1550984672
  long sys_sunset = sys["sunset"]; // 1551023855

  long id = root["id"]; // 683844
  const char* cityName = root["name"]; // CITY
  int cod = root["cod"]; // 200

  display.clearDisplay();


  getCurrentTimeRequest(coord_lat, coord_lon);

  displayTemperature(main_temp);
  displayDescription(weather_0_description);
  //displaySunriseTime(sys_sunrise);
  displaySunsetTime(sys_sunset);
  displayLocation(cityName);

  delay(5000);
}


void getCurrentTimeRequest(float latitude, float longitude) {
  String url = "http://api.geonames.org/timezoneJSON?lat=" + String(latitude) + "&lng=" + String(longitude) + "&username=kode";

  HTTPClient http;  //Declare an object of class HTTPClient
  http.begin(url);  //Specify request destination
  int httpCode = http.GET();//Send the request
  if (httpCode > 0) { //Check the returning code
    String payload = http.getString();   //Get the request response payload
    //parse data
    parseTimeData(payload);
  }
  http.end();   //Close connection
}

void parseTimeData(String payload) {
  const size_t capacity = JSON_OBJECT_SIZE(11) + 220;
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.parseObject(payload);

  const char* sunrise = root["sunrise"]; // "2019-03-08 06:44"
  float lng = root["lng"]; // 25.61
  const char* countryCode = root["countryCode"]; // "RO"
  int gmtOffset = root["gmtOffset"]; // 2
  int rawOffset = root["rawOffset"]; // 2
  const char* sunset = root["sunset"]; // "2019-03-08 18:13"
  const char* timezoneId = root["timezoneId"]; // "Europe/Bucharest"
  int dstOffset = root["dstOffset"]; // 3
  const char* countryName = root["countryName"]; // "Romania"
  const char* currentTime = root["time"]; // "2019-03-08 01:11"
  float lat = root["lat"]; // 45.65

  Serial.print(currentTime);

  displayCurrentTime(currentTime);
}


// TIME
void displayCurrentTime(String currentTime) {
  display.setTextSize(0);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  String timeOnly = currentTime.substring(10);
  display.println(timeOnly);
  display.display(); 
}

// TEMPERATURE
void displayTemperature(float main_temp) {
  display.setTextSize(0); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(80, 0);

  String temperatureValue = String((int)main_temp) + (char)247 + "C";
  display.println(temperatureValue);
  display.display(); 
}



// WEATHER DESCRIPTION
void displayDescription(String weatherDescription) {
  display.setTextSize(0); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 8);
  if(weatherDescription.length() > 18){
   weatherDescription = weatherDescription.substring(0, 15) + "...";
  }
  String description = String(weatherDescription);
  display.println(description);
  display.display(); 
}

// SUNRISE
void displaySunriseTime(long sys_sunrise) {

  sys_sunrise = sys_sunrise  + 3600;//ADD 2 Hours (For GMT+2)

  int hr = (sys_sunrise  % 86400L) / 3600;
  int minute = (sys_sunrise % 3600) / 60;
  int sec = (sys_sunrise % 60);

  String sunriseHour;
  String sunriseMinute;

  if (hr < 10) {
    sunriseHour = "0" + String(hr);
  } else {
    sunriseHour = String(hr);
  }

  if (minute < 10) {
    sunriseMinute = "0" + String(minute);
  } else {
    sunriseMinute = String(minute);
  }

  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 9);

  String sunrise = "Sunrise: " + sunriseHour + " : " + sunriseMinute;
  display.println(sunrise);
  display.display(); 
}


// SUNSET
void displaySunsetTime(long sys_sunset) {

  sys_sunset = sys_sunset + 3600;
  int sunset_hr = (sys_sunset  % 86400L) / 3600;
  int sunset_minute = (sys_sunset % 3600) / 60;
  int sunset_sec = (sys_sunset % 60);

  String sunsetHour;
  String sunsetMinute;

  if (sunset_hr < 10) {
    sunsetHour = "0" + String(sunset_hr);
  } else {
    sunsetHour = String(sunset_hr);
  }

  if (sunset_minute < 10) {
    sunsetMinute = "0" + String(sunset_minute);
  } else {
    sunsetMinute = String(sunset_minute);
  }

  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 16);

  String sunset = "Sunset: " + sunsetHour + " : " + sunsetMinute;
  display.println(sunset);
  display.display(); 
}

// LOCATION
void displayLocation(String cityName) {
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 24);
  String loc = "City: " + String(cityName);
  display.println(loc);
  display.display(); 
}

void scrolltext(String todisplay) {
  display.clearDisplay();

  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(todisplay);
  display.display();      // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(4000);

}

void displaytext(String todisplay) {
  display.clearDisplay();

  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(6, 10);
  display.stopscroll();
  display.println(todisplay);
  display.display();      // Show initial text
  delay(100);

}


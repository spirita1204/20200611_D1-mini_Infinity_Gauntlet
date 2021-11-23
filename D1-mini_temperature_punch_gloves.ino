#include <ESP8266WiFi.h>
#include <FlagHTTPClient.h>
#include "MLX90615.h"
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <U8g2lib.h>
#include <ThingSpeak.h>
#include <FlagHTTPClient.h>
//
#define DEVICE_ADDR 0x5B //MLX90615
#define Field_Number_1  1
#define Field_Number_2  2
#define trigPin   12
#define echoPin   13
//----------- Channel details ----------------//
unsigned long Channel_ID = 1073960; // Your Channel ID
#define myWriteAPIKey  "51T9WWFZ95PDWNBI" //Your write API key
//------------SMTP details----------------//
const String ssid = "iPhone";//router 
const String pass = "oooooooo";//router
const String server = "msa.hinet.net";//SMTP SERVER
int port = 25;
String myIP = "192.168.1.119"; 
String username = "aWhzdW4uY2hlbjg5QG1zYS5oaW5ldC5uZXQ="; //郵件帳號經過BASE64 加密
String password = "cTEyMzQ1c2su";  //郵件密碼經過BASE64 加密
String sender = "<ihsun.chen89@msa.hinet.net>";  //發送者
String reciver = "<s0935959625@gmail.com>";  //收件者
float temp;
int loopCount = 0;//連線太久，自動TIMEOUT用
unsigned long duration; //
int distance;
int _E8_8E_8A_E6_B3_B0_E9_A6_AC;

WiFiClient client;
MLX90615 mlx90615(DEVICE_ADDR, &Wire);
Adafruit_NeoPixel   ring = Adafruit_NeoPixel(16, 2);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C  u8g2(U8G2_R0,U8X8_PIN_NONE);
FlagHTTPClient _httpClient;

void setup() {
ThingSpeak.begin(client);
 // Wifi Connect
  while (WiFi.status() != WL_CONNECTED) {
    colorWipe(random(0,256),random(0,256)    
    ,random(0,256),100);
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    delay(10000);
  }
  Serial.println("Connected to wifi");
  Serial.println("\nStarting send email...");
    pinMode(trigPin, OUTPUT); //設定超音波腳位為輸出
  pinMode(15,OUTPUT); //設定電磁鐵腳位為輸出(GPIO15)
  digitalWrite(15,HIGH); //給定電磁鐵初始高電壓
  u8g2.begin(); //開始構建U8G2
  Wire.begin(); //加入I2C BUS
  Serial.begin(9600);
  ring.begin(); //燈條                   
  ring.setBrightness(32);//亮度設定
}

byte sendEmail()
{
  while(client.connect(server,port) != 1) {
    Serial.println("Connecting to SMTP server...");
    if(++loopCount > 2000){
      Serial.println("Failed to connect to SMTP  server ");
    }
  }//確認是否連線到SMTP server

  if(loopCount < 2000){
    Serial.println("Connected to SMTP server");
    Serial.println(F("Sending hello"));
    // replace 1.2.3.4 with your Arduino's ip
    client.println((String)"EHLO "+myIP);
    Serial.println(F("Sending auth login"));
    client.println("auth login");
    
    Serial.println(F("Sending User"));
    // Change to your base64 encoded user
    client.println(username);
     
    Serial.println(F("Sending Password"));
    // change to your base64 encoded password
    client.println(password);        

    // change to your email address (sender)
    Serial.println(F("Sending From"));
    client.println((String)"MAIL From: "+sender);
    if(!eRcv()) return 0;
   
    // change to recipient address
    Serial.println(F("Sending To"));
    client.println((String)"RCPT To: "+reciver);
    if(!eRcv()) return 0;
   
    Serial.println(F("Sending DATA"));
    client.println("DATA");
    if(!eRcv()) return 0;
   
    // Mail content here!!!!!!!!!
    Serial.println(F("Sending email"));
    client.println((String)"To: "+reciver);
    client.println((String)"From: "+sender);
    client.println("Subject: Arduino email\r\n");
    client.print("Temperature is too high :");
    client.print(temp);
    client.println("°C");
    client.println(".");
    if(!eRcv()) return 0;
  }else{
    return 0;
  }
    return 1;
}

byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;
 
  while(!client.available()) {
    delay(1);
    loopCount++;
 
    // if nothing received for 10 seconds, timeout
    if(loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }
  respCode = client.peek();
  while(client.available()){  
    thisByte = client.read();    
    Serial.write(thisByte);
  }
  if(respCode >= '4'){
    efail();
    return 0;  
  }
  return 1;
}

void efail()
{
  byte thisByte = 0;
  int loopCount = 0;
 
  client.println(F("QUIT"));
 
  while(!client.available()) {
    delay(1);
    loopCount++;
 
    // if nothing received for 10 seconds, timeout
    if(loopCount > 10000) {
      client.stop();
      
      Serial.println(F("\r\nTimeout"));
    }
  }
 
  while(client.available())
  {  
    thisByte = client.read();    
    Serial.write(thisByte);
  }

  client.stop();
  Serial.println(F("disconnected"));
}


void colorWipe(int r, int g, int b, int waitMs) {
  for (int i = 0; i < ring.numPixels(); i++) {
    ring.setPixelColor(i, r, g, b);
    ring.show();
    delay(waitMs);
  }
}


int measure_distance(){
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin,
  //returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculating the distance
  distance= duration*0.034/2;
  
  Serial.print("Distance: ");
  Serial.println(distance);
  return distance;
}

int _httpGET(String url) {
  _httpClient.end();
  _httpClient.begin(url);
  _httpClient.setTimeout(30000);
  return _httpClient.GET();
}

void loop() {
   for (int i = 0; i < ring.numPixels(); i++) { //隨機點亮作為閒置狀態
    ring.setPixelColor(i,random(0,256), random(0,256), random(0,256));
    ring.show();
  }
  while(measure_distance() > 5){  
    temp = mlx90615.getTemperature(MLX90615_OBJECT_TEMPERATURE);
  }
  Serial.println(temp);
  u8g2.clearBuffer();
  // u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.setCursor(0, 15);
  u8g2.print("temp: ");
  u8g2.print(temp);
  u8g2.sendBuffer();
  if(temp > 31.3){//超過固定溫度，燈條亮紅燈//電磁鐵摩擦摩擦//OLED顯示叉叉
    digitalWrite(15,LOW);
    delay(500);
    digitalWrite(15,HIGH);
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    for(int i = 15;i < 105;i+=15){
      u8g2.drawGlyph(i,40, 0x2716);
    }
    u8g2.sendBuffer();
    for (int i = 0; i < ring.numPixels(); i++) {
      ring.setPixelColor(i, 255, 0, 0);
    }
    ring.show();
    if(sendEmail()){
      Serial.println(F("Email sent"));      
    } else {
      Serial.println(F("Email failed"));
    }
    delay(500);
  }else{ //溫度正常則顯示綠色
    for (int i = 0; i < ring.numPixels(); i++) {
      ring.setPixelColor(i, 0, 255, 0);
    }
    ring.show();
  } 
  ThingSpeak.writeField(Channel_ID, Field_Number_1, (String)temp, myWriteAPIKey); //上傳THINGSPEAK
  _E8_8E_8A_E6_B3_B0_E9_A6_AC =      _httpGET((String(u8"https://script.google.com/macros/s/AKfycbxQjYIXuEtOFvXIOLtQ8zcYiPlYuZCYe24-nc57ceSLzunXtmw/exec") + String(u8"?t=") + String(temp))); 
  delay(1000);
}

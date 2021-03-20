#include <BME280I2C.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "MEMS Packaging lab";
const char* password = "@pressure";

const char* host = "www.cense.iisc.ac.in";
const char* streamId = "cnssrv/upload1.php";
const int httpPort = 80;
const char* mqtt_server = "m11.cloudmqtt.com";

long now = 0;
long Pt = 0;
long Tt = 0;
long Ht = 0;

long lastMsg = 0;
const long int time1 = 60000; // 5->min*sec*1000


WiFiClient espclient;
WiFiClient espclient1;
PubSubClient pubclient(espclient1);
long lastMsg2 = 0;
char msg[150];

float pmax = 925;
float pmin = 895;
float tmax = 35;
float tmin = 20;
float hmax = 80;
float hmin = 10;


BME280I2C bme;

char data[100];
char data1[100];

void setup() {
  Serial.begin(115200);
  Wire.begin();
  while(!bme.begin()){
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }
  
  setup_wifi(); 
  pubclient.setServer(mqtt_server,11224);
  pubclient.setCallback(callback);  
}

void loop() {

 
  float temp(NAN), hum(NAN), pres(NAN);
  float P,T,H;
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_bar);
  P = float(int(bme.pres())/100.00); // the unit in mbar and two decimal point aproximation  
  T = float(int(bme.temp()*100)/100.00); // two decimal point aproximation
  H = float(int(bme.hum()*100)/100.00); // two decimal point aproximation
  
  StaticJsonBuffer <150> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();  
  root["T"] = T;
  root["P"] = P;
  root["H"] = H;
  root.printTo(data,sizeof(data));

  DynamicJsonBuffer jsonBuffer2;
  JsonObject& root_2 = jsonBuffer2.createObject();  
  JsonArray& result = root_2.createNestedArray("result");
  JsonObject& root__2 = result.createNestedObject();
  root__2["T"] = T;
  root__2["P"] = P;
  root__2["H"] = H;
  root_2.printTo(data1,sizeof(data1));
  
  long now = millis();
  if (now - lastMsg > 30000) {
    lastMsg = now;
    
    if (!espclient.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }
    
    // We now create a URI for the request
    String url = "http://";
    url += host;
    url += "/";
    url += streamId;
    url += "?Json=";
    String temp_data(data);
    url += temp_data;
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
    // This will send the request to the server
    espclient.print(String("GET ") + url +" HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");  
    unsigned long timeout = millis();
    while (espclient.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> espClient Timeout !");
        espclient.stop();
        return;
      }
    }
    
    // Read all the lines of the reply from server and print them to Serial
    while(espclient.available()){
      String line = espclient.readStringUntil('\r');
      Serial.print(line);
    }
    
    Serial.println();
    //Serial.println("closing connection");
  }

  if (!pubclient.connected()) {
    reconnect();
    pubclient.subscribe("testp/data");
  }
  pubclient.loop();

  long now2 = millis();
  if (P<=pmin || P>=pmax){
    //Serial.println(now);
    if (now2 - Pt >= time1){
      char outstr[10];
      dtostrf(P,7, 3, outstr);
      snprintf (msg, 150, "Pressure outof range %s",outstr);
      pubclient.publish("testp/Noti", msg);
      Pt = now2;  
      Serial.println("Pressure published");    
    }
  }
  //Serial.println("P->T");
  
  if (T<=tmin || T>=tmax){
    if(now2-Tt > time1){
      char outstr[10];
      dtostrf(T,7, 3, outstr);
      snprintf (msg, 150, "Temperature outof range %s",outstr);
      pubclient.publish("testp/Noti", msg);
      Tt = now2;
      Serial.println("Tempreture published");      
    }
  }
  //Serial.println("T->H");
  if (H<=hmin || H>=hmax){
    if(now2 - Ht > time1){
      char outstr[10];
      dtostrf(H,7, 3, outstr);
      snprintf (msg, 150, "Humidity outof range %s",outstr);
      pubclient.publish("testp/Noti", msg);
      Ht = now2;  
      Serial.println("Humidity published");   
    }
  }
}


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char data2[10] = "Send Data";
  int count = 0;
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    if(data2[i] == (char)payload[i]) {
      count = count + 1;
    }
    if (count == 9){
      Serial.println("Sending Status");
      
      pubclient.publish("testp/response",data1);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!pubclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random pubclient ID
    // Attempt to connect
    if (pubclient.connect("PKLNODE","pfluekdn","KIrRRhrEKjJX")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

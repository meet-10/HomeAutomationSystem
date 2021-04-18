#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#error "Board not found"
#endif


/*--------Libraries To Be Included--------*/
// WebsocketsServer is fullDuplex communication(both server
// and client send data at particular time).Bi-directional
// communication pattern.Clients are connected to each other.
// install websocket library by Markus Sattler from library manager. 
#include <WebSocketsServer.h>
// install arduino json by benoit blanchon from library manager.
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include "DHT.h" //DHT and Adafruit Sensor library(https://github.com/adafruit/Adafruit_Sensor)




/*--------Defining Variables--------*/

#define DHTPIN 4 //dht data pin
#define LED1 2 //bulb 1
#define LED2 18 //bulb 2
#define LED3 15 //bulb 3
#define PIRPIN 5 //Pir data pin
#define DHTTYPE   DHT11
#define GREEN_LED 22 //Green led pin
#define RED_LED 19 //Red led pin
#define PIN 36 // current sensor pin
#define Lswitch 21 // limit switch pin
float resolution  = 3.3 / 4095;                 // Input Voltage Range is 1V to 3.3V 2.198
                                                  // ESP32 ADC resolution is 10-bit. 2^12 = 4095

uint32_t period = 1000000 / 60;                   // One period of a periodic waveform
uint32_t t_start = 0;                                 

// setup
float zero_ADC_Value = 0, zero_voltageValue = 0;   

// loop in current sensor
float ADC = 0, Vrms = 0, Current = 0, Q = 0.0147;
float sensitivity = 0.100;                        // 185 mV/A, 100 mV/A and 0.66 mV/A for ±5A, ±20A and ±30A current range respectively. 
float diff = 0;
int flag = 0;
int lmt_swch = 0;

/*--------WiFi Credentials--------*/
const char* ssid = "OPPO Reno3 Pro";
const char* password =  "meetshah10@";

DHT dht(DHTPIN, DHTTYPE);


// ipaddress/led1/on
//ipaddress/led1/off

// ipaddress/led2/on
//ipaddress/led2/off
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80); // server port 80
WebSocketsServer websockets(81);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Page Not found");
}
/**
 * num - number assigned to a connected client
 * payload - recive the data from client side
 */
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) 
  {
     // incase any client gets disconnected then this case will be called
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
     // when client gets connected then case will be called
    // it will saves its ip address and prints its ip address 
    // then it print as connected 
    case WStype_CONNECTED: {
        IPAddress ip = websockets.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client then he is connected. It is sent from server side.
        websockets.sendTXT(num, "Connected from server");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char*)( payload)); // payload converted to string format
      Serial.println(message);
      Serial.println(message);

      
     DynamicJsonDocument doc(200);
    // deserialize the data
    DeserializationError error = deserializeJson(doc, message);// doc - saving json based data
    // parse the parameters we expect to receive (TO-DO: error handling)
      // Test if parsing succeeds.
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  int LED1_status = doc["LED1"];// fetch the value of LED1 object and saved it into variable LED1_STATUS
  int LED2_status = doc["LED2"];// fetch the value of LED2 object and saved it into variable LED2_STATUS
  digitalWrite(LED1,LED1_status);
  digitalWrite(LED2,LED2_status);
  }
}

void setup(void)
{
  
  Serial.begin(115200); // Initialize Serial communication
  dht.begin(); // Initialize DHT communication
  pinMode(PIRPIN,INPUT); // Set pin D5 as read.
  pinMode(PIN, INPUT); // Set pin 36 as read.
  pinMode(Lswitch, INPUT); // Set pin 21 as read.
  pinMode(LED1,OUTPUT); // Set pin D2 as output.
  pinMode(LED2,OUTPUT); // Set pin D18 as output.
  pinMode(LED3,OUTPUT); // Set pin D15 as output.
  pinMode(GREEN_LED, OUTPUT); // Set pin D22 as output.
  pinMode(RED_LED, OUTPUT); // Set pin D19 as output.
 

 

 WiFi.begin(ssid, password);
  
   if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    delay(500);
    Serial.println(WiFi.localIP());
  }
  
    WiFi.softAP("SMART HOME", "");
    Serial.println("softap");
    Serial.println("");
    Serial.println(WiFi.softAPIP());
     
   if(MDNS.begin("ESP")){//esp.local/
    Serial.println("MDNS responder started");
  }


  /*--------ESP32-CurrentSensor-ACS712--------*/
  t_start = micros();                             
  uint32_t ADC_SUM = 0, n = 0;
  while(micros() - t_start < period) {            
    ADC_SUM += analogRead(PIN);                   // reading the analog value from pin 36. 
    n++;                                          // counter to be used for avg. 
  }
  zero_ADC_Value = (ADC_SUM / n);                 // The avg analog value when no current pass throught the ACS712 sensor.
  zero_voltageValue = zero_ADC_Value * resolution;    // The ACS712 output voltage value when no current pass trough the sensor (i = 0)

  
  server.on("/", [](AsyncWebServerRequest * request)
  { 
   
  //request->send_P(200, "text/html", html);
  request->redirect("http://127.0.0.1:8000/");
  });


  server.on("/led1/on", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    digitalWrite(LED1,HIGH);
    //request->send_P(200, "text/html", html);
    request->redirect("http://127.0.0.1:8000/");
    
  });

  server.on("/led1/off", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    digitalWrite(LED1,LOW);
    //request->send_P(200, "text/html", html);
    request->redirect("http://127.0.0.1:8000/");
  });

  server.on("/led2/on", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    digitalWrite(LED2,HIGH);
    //request->send_P(200, "text/html", html);
    request->redirect("http://127.0.0.1:8000/");
  });

  server.on("/led2/off", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    digitalWrite(LED2,LOW);
    //request->send_P(200, "text/html", html);
    request->redirect("http://127.0.0.1:8000/");
  });

   
  server.onNotFound(notFound);

  server.begin();  // it will start webserver
  websockets.begin();// it will start websocket server
  websockets.onEvent(webSocketEvent);// recieving the data from the client
}


void loop(void)
{
 websockets.loop(); // Websocket function
 send_sensor(); // temperature-humidity function
 pir_sensor(); // PIR Sensor function
 current_sensor(); // Current Sensor Function
 limit_switch(); // Limit Switch Function
}

/* TEMPERATURE-HUMIDITY SENSOR FUNCTION */
void send_sensor()
{

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
   
//    JSON_Data = {"temp":t,"hum":h}
   //JSON_DATA = {"stat":pirStat, "str":"text"} 
  String JSON_Data = "{\"temp\":";
         JSON_Data += t;
         JSON_Data += ",\"hum\":";
         JSON_Data += h;
         JSON_Data += "}";
   Serial.println(JSON_Data);     
  websockets.broadcastTXT(JSON_Data);
  delay(300);
}

/* PIR SENSOR FUNCTION */
void pir_sensor(){
  int pirStat = digitalRead(PIRPIN);
  String JSON_Data = "{\"stat\":";
         JSON_Data += pirStat;
         JSON_Data += "}";
  Serial.println(JSON_Data);     
  websockets.broadcastTXT(JSON_Data);
  delay(250);
 
  if(pirStat == HIGH){
  digitalWrite(LED3,LOW);
 }
 else{
  digitalWrite(LED3,HIGH);
 }

 
}

/* CURRENT SENSOR FUNCTION */
void current_sensor(){
/*----Vrms & Irms Calculation----*/
  t_start = micros();                             
  uint32_t ADC_Dif = 0, ADC_SUM = 0, m = 0;        
  while(micros() - t_start < period) {            // Defining one period of the waveform. US frequency(f) is 60Hz. Period = 1/f = 0.016 seg = 16,666 microsec
    ADC_Dif = zero_ADC_Value - analogRead(PIN);   // To start from 0V we need to subtracting our initial value when no current passes through the current sensor, (i.e. 750 or 2.5V).
    ADC_SUM += ADC_Dif * ADC_Dif;                 // SUM of the square
    m++;                                          // counter to be used for avg.
  }
  ADC = sqrt(ADC_SUM / m);                        // The root-mean-square ADC value. 
  Vrms = ADC * resolution ;                       // The root-mean-square analog voltage value.   
  Current = (Vrms  / sensitivity) - Q;        // The root-mean-square analog current value. Note: Q
  diff = Current - 0.245;
  //------------------------------//
  
  //Serial.print("analogRead = ");
  //Serial.println(analogRead(PIN));
  
  //Serial.print("Vrms = ");                        
  //Serial.print(Vrms, 6);
  //Serial.println(" V");        

  //Serial.print("Irms = ");                       
  //Serial.print(Current, 6);
  //Serial.print(" A");
  //Serial.print(diff, 3);
  //Serial.println(" A");
  //Serial.print("\n");
  
  


   
   String JSON_Data = "{\"curr_stat\":";
         JSON_Data += diff;
         JSON_Data += "}";
  Serial.println(JSON_Data);     
  websockets.broadcastTXT(JSON_Data);
  delay(300);

  
}

/* LIMIT SWITCH FUNCTION */
void limit_switch(){
  lmt_swch = digitalRead(Lswitch);
if( (lmt_swch == LOW) && (flag == 0) ) 
  {
    Serial.println("door is closed"); 
    flag = 1; 
    delay(20); 
  }

if( (lmt_swch == HIGH) && (flag == 1) ) 
  {
    Serial.println("door is opened"); 
    flag = 0;
    delay(20); 
  }

  String JSON_Data = "{\"swch_stat\":";
         JSON_Data += lmt_swch;
         JSON_Data += "}";
  Serial.println(JSON_Data);     
  websockets.broadcastTXT(JSON_Data);
  delay(250);
  
  if ( flag == 0 ) 
  {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);  
     
  }
  
    if ( flag == 1 ) 
  {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW); 
  }
  
  digitalWrite(Lswitch, HIGH);
 
}

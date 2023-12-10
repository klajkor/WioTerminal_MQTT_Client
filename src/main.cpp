/*
 
*/

#include <Arduino.h>
#include <rpcWiFi.h>
#include <PubSubClient.h>
#include "secrets.h" 
#include "Free_Fonts.h"
#include <ArduinoJson.h>

#include "app_messages.h"
#include "display.h"
#include "main.h"
#include "main_defs.h"
#include "serial_print.h"
#include "wio_gpio.h"
#include "wio_battery.h"


char value[7] = "      "; //initial values
char value2[7] = "      ";
char stamp[40] = "2021-09-01T13:04:34"; 


// Update these with values suitable for your network.
char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// Update these with values suitable for your MQTT server.
const char mqtt_user[] = SECRET_MQTT_USER;
const char mqtt_pass[] = SECRET_MQTT_PASS;

// MQTT server info
const char* mqttServer = "192.168.1.13";
const int mqttPort = 1883;

// Timer variables
static TimerHandle_t battery_status_update_timer;
bool                 do_battery_status_update = false;

// JSON document variables
StaticJsonDocument<256> doc1;
StaticJsonDocument<256> doc2;
StaticJsonDocument<256> doc3;

int data1 = 0;  //data1 of MQTT json message
int data2 = 0;  //data2 of MQTT json message
int data3 = 0;  //data3 of MQTT json message
int data4 = 0;  //data3 of MQTT json message
int msg = 0;
const char* timestamp = "dummy data";
String recv_payload;
const char* subtopic1 = "tele/ggbase_ttgo/HEARTBEAT";    
const char* subtopic2 = "tele/ggbase_ttgo/garage02/SENSOR";
const char* subtopic3 = "tele/test_sensor/SENSOR"; 

// wio terminal wifi 
WiFiClient wclient;
PubSubClient mqtt_client(wclient); // Setup MQTT client

void setup_wifi(void) 
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  wio_wifi_status_update(ssid);
}

// mqtt message callback
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    const char* heartbeat_timestamp = "2020-01-01T11:22:33";
    float garage_temp = 0.0;
    float garage_hum = 0.0;
    //print out message topic and payload  
    Serial.print("New message from Subscribe topic: ");
    Serial.println(topic);
    Serial.print("Subscribe JSON payload: ");
    for (uint32_t i = 0; i < length; i++) {
        Serial.print((char)payload[i]);     // print mqtt payload
    }
    Serial.println();

    msg = 1;  //set message flag = 1 when new subscribe message received

    //check subscribe topic for message received and decode    
    
    if (strcmp(topic, subtopic1)== 0) {  
        
        Serial.print("TTGO Heartbeat:");
        Serial.println(topic);
        deserializeJson(doc1, (const byte*)payload, length);
        heartbeat_timestamp = doc1["Time"];
        Serial.println(heartbeat_timestamp);
        wio_heartbeat_update(heartbeat_timestamp);
    }
    //********************************//
    //if message from topic subtopic2
    //********************************//
    if (strcmp(topic, subtopic2)== 0) {   
        Serial.print("Payload from topic: ");
        Serial.println(topic);
        deserializeJson(doc2, (const byte*)payload, length);
        garage_temp = doc2["SI7021"]["Temperature"];
        garage_hum = doc2["SI7021"]["Humidity"];
        
        // itoa(data2,value2,10); 
        Serial.print("Garage Temp=");
        Serial.println(garage_temp);
        Serial.print("Garage Humidity=");
        Serial.println(garage_hum);
        
    }
        
    //********************************//
    //if message from topic subtopic3
    //********************************//
    if (strcmp(topic, subtopic3)== 0) {   
        Serial.print("decode payload from topic ");
        Serial.println(topic);
        deserializeJson(doc3, (const byte*)payload, length);   //parse MQTT message
        data3 = doc3["SI7021"]["Temperature"];
        data4 = doc3["SI7021"]["Humidity"];
        Serial.print("data3 Temp = ");
        Serial.println(data3);
        Serial.print("data4 Hum = ");
        Serial.println(data4);
        itoa(data1,value,10);  //convert data integer to character "value"
        
        timestamp = doc3["Time"];    //mqtt message timestamp
        strcpy (stamp,timestamp);
        stamp[19] = 0;   // terminate string after seconds         
    }
}

//connect to mqtt broker 
void mqtt_reconnect(void) 
{
    // Loop until we're reconnected
    while (!mqtt_client.connected()) 
    {
        Serial.print("Attempting MQTT connection...");
        // Create a unique client ID using the Wio Terminal MAC address
        String MACadd = WiFi.macAddress();
        MACadd = "WioT" + MACadd;  
        String clientID = MACadd;

        // Attempt to connect
        if (mqtt_client.connect(clientID.c_str(), mqtt_user, mqtt_pass)) 
        {
            Serial.println("MQTT connected");
            // set up MQTT topic subscription
            Serial.println("subscribing to topics:");
            Serial.println(subtopic1);
            if (mqtt_client.subscribe(subtopic1))
            {
                Serial.println(" - subscribed");
            }
            else
            {
                Serial.println(" - Failed");
            }Serial.print(subtopic2);
            if (mqtt_client.subscribe(subtopic2))
            {
                Serial.println(" - subscribed");
            }
            else
            {
                Serial.println(" - Failed");
            }
            Serial.print(subtopic3);
            if (mqtt_client.subscribe(subtopic3))
            {
                Serial.println(" - subscribed");
            }
            else
            {
                Serial.println(" - Failed");
            }      
        } 
        else 
        {
            Serial.print("failed, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 3 seconds");
            delay(3000);
        }
    }
}


void battery_status_update_callback(TimerHandle_t xTimer)
{
    do_battery_status_update = true;
}

void setup() 
{
    Serial.begin(115200);
    delay(200);
    serial_println(MSG_SETUP_STARTED);
    battery_init();
    wio_gpio_init();
    wio_display_init(Display_Rotation_Portrait);
    delay(2000);
    wio_set_background();
    delay(1000);
    wio_battery_status_update();
    delay(5000);
    serial_println(MSG_DISPLAY_INIT_FINISHED);
    delay(1000);
    battery_status_update_timer = xTimerCreate("battery_status_update_timer", PERIOD_BATTERY_STATUS_UPDATE, pdTRUE,
                                               (void *)0, battery_status_update_callback);
    if (battery_status_update_timer == NULL)
    {
        serial_println(MSG_TIMER_CREATE_ERROR);
    }
    else
    {
        serial_println(MSG_TIMER_CREATED);
        xTimerStart(battery_status_update_timer, 0);
    }
    serial_println(MSG_CONNECTING_TO_WIFI);
    setup_wifi();
    mqtt_client.setServer(mqttServer, 1883);  //set mqtt server
    mqtt_client.setCallback(mqtt_callback);
}

void loop() {
    // check if connected to mqtt
    if (!mqtt_client.connected()) {
        mqtt_reconnect();
    }
    if (msg == 1) 
    {                         // check if new callback message
        msg = 0;                             // reset message flag
        /*
        Serial.print("decoded timestamp = ");
        Serial.println(stamp);
        Serial.print("decoded data1 = ");
        Serial.println(data1);
        Serial.print("decoded data2 = ");
        Serial.println(data2);    
        */
    }
    if (do_battery_status_update)
    {
        do_battery_status_update = false;
        wio_battery_status_update();
    }
    Serial.println("debug - in main loop");
    delay(2000);
    mqtt_client.loop();
}
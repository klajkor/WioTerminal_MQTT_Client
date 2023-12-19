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

uint32_t delay_counter = 0;

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
StaticJsonDocument<256> heartbeat_json;
StaticJsonDocument<256> garage_sensor_json;

int msg = 0;
const char* timestamp = "dummy data";
String recv_payload;
const char* subtopic_heartbeat = "tele/ggbase_ttgo/HEARTBEAT";    
const char* subtopic_sensor = "tele/ggbase_ttgo/garage02/SENSOR";


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
    char temperature_str[8] = "100.0 C";
    char humidity_str[5] = "100%";    
    const char* heartbeat_timestamp = "2020-01-01T11:22:33";
    const char* garage_timestamp = "2020-01-01T11:22:33";
    float garage_temp = 0.0;
    uint8_t garage_hum = 0;
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
    
    if (strcmp(topic, subtopic_heartbeat)== 0) {  
        
        deserializeJson(heartbeat_json, (const byte*)payload, length);
        heartbeat_timestamp = heartbeat_json["Time"];
        Serial.print("TTGO Heartbeat:");
        Serial.println(heartbeat_timestamp);
        wio_heartbeat_update(heartbeat_timestamp);
    }
    
    if (strcmp(topic, subtopic_sensor)== 0) {   
        deserializeJson(garage_sensor_json, (const byte*)payload, length);
        garage_timestamp = heartbeat_json["Time"];
        garage_temp = garage_sensor_json["SI7021"]["Temperature"].as<float>();
        garage_hum = garage_sensor_json["SI7021"]["Humidity"];
        snprintf(temperature_str, 8, "%.1f C", garage_temp);
        snprintf(humidity_str, 5, "%3d%%", garage_hum);
        
        if ((garage_temp * garage_hum) != 0)
        {
            Serial.print(garage_timestamp);
            Serial.print(" Garage Temp=");
            Serial.print(temperature_str);
            Serial.print(" Garage Humidity=");
            Serial.println(humidity_str);
        }
        
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
            Serial.println(subtopic_heartbeat);
            if (mqtt_client.subscribe(subtopic_heartbeat))
            {
                Serial.println(" - subscribed");
            }
            else
            {
                Serial.println(" - Failed");
            }Serial.print(subtopic_sensor);
            if (mqtt_client.subscribe(subtopic_sensor))
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
    delay_counter = millis();
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
    {                         // check if new callback message        msg = 0;                             
    }
    if (do_battery_status_update)
    {
        do_battery_status_update = false;
        wio_battery_status_update();
    }
    if (millis() > (delay_counter + DELAY_MAIN_LOOP_MILLISEC))
    {
        Serial.println("debug - in main loop");
        delay_counter = millis();
    }
    delay(100);
    mqtt_client.loop();
}
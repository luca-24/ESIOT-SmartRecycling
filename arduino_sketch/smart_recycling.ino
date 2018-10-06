#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <string.h>

const char* ssid = "TISCALI-016D20";
const char* password = "GPON00016D202";
const char* mqtt_broker = "a3niqqc065ygmt.iot.eu-west-1.amazonaws.com";

const char* cert_path = "/cert.der";
const char* key_path = "/private.der";
const char* cert_ca = "VeriSign-Class 3-Public-Primary-Certification-Authority-G5.pem";

String GARBAGE_TYPE = "Paper"; //Can be Paper, Plastic, Glass, Organic or Inorganic
const int project_mode = 1; //0 -> Camera, 1 -> Bot

int shadow_status = 0;
const size_t buffer_size = 5*JSON_OBJECT_SIZE(1) + 4*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 240;

WiFiClientSecure espClient;
PubSubClient client(espClient);

int red_led = 0;
int yellow_led = 4;
int green_led = 5;
int button = 16;
int relay = 2;

int button_state = 0;
int blinking = 0;
int waiting = 0;

int response = 0;

void setup() {
  Serial.begin(115200);
  pinMode(red_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(button, INPUT);
  pinMode(relay, OUTPUT);

  digitalWrite(red_led, LOW);
  digitalWrite(yellow_led, LOW);
  digitalWrite(green_led, LOW);
  digitalWrite(relay, LOW);

  setup_wifi();
  client.setServer(mqtt_broker, 8883);
  client.setCallback(callback);

  bool ok = SPIFFS.begin();
  if (ok) {
    File cert = SPIFFS.open(cert_path, "r");
  
    if (!cert) Serial.println("Failed to open cert file");
    else Serial.println("Success to open cert file");
  
    delay(1000);
  
    if (espClient.loadCertificate(cert)) Serial.println("cert loaded");
    else Serial.println("cert not loaded");
  
    File private_key = SPIFFS.open(key_path, "r"); //replace private eith your uploaded file name
    
    if (!private_key) Serial.println("Failed to open private cert file");
    else Serial.println("Success to open private cert file");
  
    delay(1000);
  
    if (espClient.loadPrivateKey(private_key)) Serial.println("private key loaded");
    else Serial.println("private key not loaded");
  }
}

void setup_wifi(){
  delay(10);
  Serial.println("\nConnecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println();
}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char payload_string[length];

  for (int i=0; i<length; i++){
    payload_string[i] = (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (project_mode == 0){
    blinking = 0;
  
    switch((char)payload[0]){
      case '+':
        response = 1;
        digitalWrite(green_led, HIGH);
        client.publish("$aws/things/esiot_test/shadow/update", "{\"state\": {\"reported\": {\"action\": \"ACCEPTED\"}}}");
        break;
      case '-':
        response = -1;
        digitalWrite(red_led, HIGH);
        client.publish("$aws/things/esiot_test/shadow/update", "{\"state\": {\"reported\": {\"action\": \"REJECTED\"}}}");
        break;
      default:
        response = 0;
        blinking = 1;
    }
  }
  else{
    DynamicJsonBuffer jsonBuffer(buffer_size);
    JsonObject& root = jsonBuffer.parseObject(payload_string);
    const char* action = root["state"]["reported"]["action"];

    if(strcmp(action, "WAITING")==0){
      shadow_status = 0;
    }
    else if(strcmp(action, "PROCESSING")==0){
      shadow_status = 1;
    }
    else if(strcmp(action, "ACCEPTED")==0){
      shadow_status = 2;
    }
    else if(strcmp(action, "REJECTED")==0){
      shadow_status = 3;
    }
    else{
      shadow_status = 0;
    }

  }
}

void reconnect(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");

    if(client.connect("ESP8266Client")){
      Serial.println("connected");
    }
    else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();

  if (project_mode == 0){
    if (waiting){
      button_state = 0;
    }
    else{
      button_state = digitalRead(button);
    }
  
    switch(button_state){
      case 0:
        break;
      case 1:
        blinking = blinking ? 0 : 1;
        client.subscribe("esiot_out");
        char message[60];
        String json_mex = "{\"message\":\"banana\",\"name\":\"Glass\",\"type\":\"" + GARBAGE_TYPE + "\"}";
        json_mex.toCharArray(message, json_mex.length()+1);
        client.publish("esiot_in", message);
        client.publish("$aws/things/esiot_test/shadow/update", "{\"state\": {\"reported\": {\"action\": \"PROCESSING\"}}}");
        digitalWrite(relay, HIGH);
        delay(200);
    }
  
    if (response != 0){
      delay(10000);
      if (response == 1){
        digitalWrite(green_led, LOW);
      }
      else{
        digitalWrite(red_led, LOW);
      }
      response = 0;
      client.publish("$aws/things/esiot_test/shadow/update", "{\"state\": {\"reported\": {\"action\": \"WAITING\"}}}");
    }
  }
  else{
    client.subscribe("$aws/things/esiot_test/shadow/get/accepted");
    delay(200);
    client.publish("$aws/things/esiot_test/shadow/get", "{}");

    switch (shadow_status){
      case 0:
        Serial.println("WAITING");
        blinking = 0;
        digitalWrite(green_led, LOW);
        digitalWrite(red_led, LOW);
        break;
      case 1:
        Serial.println("PROCESSING");
        blinking = 1;
        break;
      case 2:
        Serial.println("ACCEPTED");
        blinking = 0;
        digitalWrite(green_led, HIGH);
        digitalWrite(red_led, LOW);
        break;
      case 3:
        Serial.println("REJECTED");
        blinking = 0;
        digitalWrite(red_led, HIGH);
        digitalWrite(green_led, LOW);
    }
  }
  
  if (blinking){
    digitalWrite(yellow_led, HIGH);
    delay(200);
    digitalWrite(yellow_led, LOW);
  }
  delay(200);
}

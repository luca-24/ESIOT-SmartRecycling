#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <string.h>
//#include <ArduCAM.h>
// #include <SPI.h>
// #include "memorysaver.h"
// #include <Wire.h>

// //This demo can only work on OV2640_MINI_2MP or ARDUCAM_SHIELD_V2 platform.
// #if !(defined (OV2640_MINI_2MP)||(defined (ARDUCAM_SHIELD_V2) && defined (OV2640_CAM)))
// #error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
// #endif

// // set GPIO16 as the slave select :
// const int CS = 14;

// String errMsg = "";

// ArduCAM myCAM(OV2640, CS);

// int fileTotalKB = 0;
// int fileUsedKB = 0; int fileCount = 0;
// String errMsg = "";
// int imgMode = 1; // 0: stream  1: capture
// int resolution = 3;
// // resolutions:
// // 0 = 160x120
// // 1 = 176x144
// // 2 = 320x240
// // 3 = 352x288
// // 4 = 640x480
// // 5 = 800x600
// // 6 = 1024x768
// // 7 = 1280x1024
// // 8 = 1600x1200

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

  //   // check for properties file
  //   File f = SPIFFS.open(fName, "r");

  //   if (!f) {
  //     // no file exists so lets format and create a properties file
  //     Serial.println("Please wait 30 secs for SPIFFS to be formatted");

  //     SPIFFS.format();

  //     Serial.println("Spiffs formatted");

  //     f = SPIFFS.open(fName, "w");
  //     if (!f) {
  //     Serial.println("properties file open failed");
  //     }
  //     else
  //     {
  //     // write the defaults to the properties file
  //     Serial.println("====== Writing to properties file =========");

  //     f.println(resolution);

  //     f.close();
  //     }

  //   }
  //   else
  //   {
  //     // if the properties file exists on startup,  read it and set the defaults
  //     Serial.println("Properties file exists. Reading.");

  //     while (f.available()) {

  //     // read line by line from the file
  //     String str = f.readStringUntil('\n');

  //     Serial.println(str);

  //     resolution = str.toInt();

  //     }

  //     f.close();
  //   }


  //   uint8_t vid, pid;
  //   uint8_t temp;

  //   #if defined(__SAM3X8E__)
  //     Wire1.begin();
  //   #else
  //     Wire.begin();
  //   #endif
  }

  // // set the CS as an output:
	// pinMode(CS, OUTPUT);

  // // initialize SPI:
	// SPI.begin();
	// SPI.setFrequency(4000000); //4MHz

	// //Check if the ArduCAM SPI bus is OK
	// myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
	// temp = myCAM.read_reg(ARDUCHIP_TEST1);
	// if (temp != 0x55) {
	// 	Serial.println("SPI1 interface Error!");
	// }

	// //Check if the camera module type is OV2640
	// myCAM.wrSensorReg8_8(0xff, 0x01);
	// myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
	// myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
	// if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 )))
	// 	Serial.println("Can't find OV2640 module! pid: " + String(pid));
	// else
	// 	Serial.println("OV2640 detected.");


	// //Change to JPEG capture mode and initialize the OV2640 module
	// myCAM.set_format(JPEG);
	// myCAM.InitCAM();

	// setCamResolution(resolution);

	// myCAM.clear_fifo_flag();

  // Dir dir = SPIFFS.openDir("/pics");
	// while (dir.next()) {
	// 	fileCount++;
	// }

	// FSInfo fs_info;
	// SPIFFS.info(fs_info);

	// fileTotalKB = (int)fs_info.totalBytes;
	// fileUsedKB = (int)fs_info.usedBytes;
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


// /////////////////////////////////////////////////
// //   Updates Properties file with resolution  ///
// /////////////////////////////////////////////////
// void updateDataFile()
// {

// 	File f = SPIFFS.open(fName, "w");
// 	if (!f) {
// 		Serial.println("prop file open failed");
// 	}
// 	else
// 	{
// 		Serial.println("====== Writing to prop file =========");

// 		f.println(resolution);
// 		Serial.println("Data file updated");
// 		f.close();
// 	}

// }

// ///////////////////////////////////////////
// //    Saves captured image to memory     //
// ///////////////////////////////////////////
// void myCAMSaveToSPIFFS() {

//   // as file space is used, capturing images will get slower. At a certain point, the images will become distored
//   // or they will not save at all due to lack of space. To avoid this we set a limit and allow some free space to remain
// 	if ((fileTotalKB - fileUsedKB) < fileSpaceOffset)
// 	{
// 		String maxStr = "====== Maximum Data Storage Reached =========";
// 		Serial.println(maxStr);
// 		errMsg = maxStr;
// 		return;
// 	}

// 	String str;
// 	byte buf[256];
// 	static int i = 0;

// 	static int n = 0;
// 	uint8_t temp, temp_last;

// 	//  File file;
// 	//Flush the FIFO
// 	myCAM.flush_fifo();
// 	//Clear the capture done flag
// 	myCAM.clear_fifo_flag();
// 	//Start capture
// 	myCAM.start_capture();

// 	while (!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
// 	Serial.println("File Capture Done!");

// 	fileCount++;

// 	str = "/pics/" + String(fileCount)  + ".jpg";

// 	File f = SPIFFS.open(str, "w");
// 	if (!f) {
// 		Serial.println("prop file open failed");
// 	}
// 	else
// 	{
// 		Serial.println(str);
// 	}


// 	i = 0;
// 	myCAM.CS_LOW();
// 	myCAM.set_fifo_burst();
// 	#if !(defined (ARDUCAM_SHIELD_V2) && defined (OV2640_CAM))
// 		SPI.transfer(0xFF);
// 	#endif
// 	//Read JPEG data from FIFO
// 	while ( (temp != 0xD9) | (temp_last != 0xFF)) {
// 		temp_last = temp;
// 		temp = SPI.transfer(0x00);

// 		//Write image data to buffer if not full
// 		if ( i < 256)
// 		buf[i++] = temp;
// 		else {
// 		//Write 256 bytes image data to file
// 		myCAM.CS_HIGH();
// 		f.write(buf , 256);
// 		i = 0;
// 		buf[i++] = temp;
// 		myCAM.CS_LOW();
// 		myCAM.set_fifo_burst();
// 		}
// 		//delay(0);
// 	}

// 	//Write the remain bytes in the buffer
// 	if (i > 0) {
// 		myCAM.CS_HIGH();
// 		f.write(buf, i);
// 	}
// 	//Close the file
// 	f.close();
// 	Serial.println("CAM Save Done!");
// }

// ///////////////////////////////////////////////////////
// //   deletes all files in the /pics directory        //
// ///////////////////////////////////////////////////////
// void clearData(){
// 	errMsg = "======  Data Storage Cleared =========";
// 	Dir dir = SPIFFS.openDir("/pics");
// 	while (dir.next()) {
// 		SPIFFS.remove(dir.fileName());
// 	}

// 	fileCount = 0;
// 	fileTotalKB = 0;
// 	fileUsedKB = 0;

// 	handleNotFound();
// }


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
        //myCAMSaveToSPIFFS();
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
        digitalWrite(relay, LOW);
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
        digitalWrite(relay, HIGH);
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

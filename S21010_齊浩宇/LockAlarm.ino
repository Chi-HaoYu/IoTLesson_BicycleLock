#include <WiFi.h>
#include <PubSubClient.h>

void MqttWork(void);
bool checkKeyOn(void);
bool checkLineOk(void);
void alarmMode(void);
void buzzer(void);
void killed(void);
void bibi(void);

// WiFi Option
const char ssid[] = "Buffalo-G-A330";
const char passwd[] = "s4hcec6nk3x77";
WiFiClient wifiClient;

// Mqtt Option
const char* mqttHost = "sunlab.kic.ac.jp";
const int mqttPort = 1883;
PubSubClient mqttClient(wifiClient);

//Set topic
const char topic[]  = "s21010/LockAlarm";


//Set Lock Alarm Pin
const int keyPin = 25;
const int beePin = 26;
const int linePin = 27;

bool keystate = 1;
bool keystate0 = 1;

bool linestate = 1;
bool linestate0 = 1;
bool alarmOn = 0;

int timeCount = 0;
bool publishTime = 0;

bool stopButton = 0;
bool downButton = 0;
bool bibiButton = 0;
char* words = {"Got_it_its_ok"};
char* esc = {"Maybe_its_broken_turn_off_all"};
char* where = {"Im_here"};

//Set Callback Method
void callback(char* topic, byte* payload, unsigned int length) {
  int stopCount=0, downCount=0, bibiCount=0;
  Serial.print("Message arrived On [ ");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    //Check Sign from Android side and stop the buzzer
    if(payload[i] == words[i])
      stopCount++;
    
    //Check Sign from Android side and stop all
    if(payload[i] == esc[i])
      downCount++;

    //Check Sign from Android side and stop trigger buzzer twice
    if(payload[i] == where[i])
      bibiCount++;
    
  }
  Serial.println("");

  if(stopCount == length){
    stopButton = 1;
  }
  else{
    stopButton = 0;
  }
  if(downCount == length){
    downButton = 1;
   }
  else{
    downButton = 0;
  }
  if(bibiCount == length){
    bibiButton = 1;
   }
  else{
    bibiButton = 0;
  }
}

void setup() {
  Serial.begin(115200);

  //Set Pin Mode
  pinMode(keyPin, INPUT);
  pinMode(beePin, OUTPUT);
  pinMode(linePin, INPUT);
  
  //WiFi Connect
  WiFi.begin(ssid, passwd);
  Serial.print("WiFi connecting...");
  //WiFi Exception
  unsigned int waitTime = 0;
  while(WiFi.status() != WL_CONNECTED) {
      if(waitTime++ > 0 && !(waitTime%50)){
        bibi();
      }
      Serial.print(".");
      delay(100);
  }
  Serial.print(" connected. ");
  Serial.println(WiFi.localIP());
  //Set Mqtt Option
  mqttClient.setServer(mqttHost, mqttPort);
  mqttClient.setCallback(callback);
}


void loop() {
  //Let Mqtt work
  MqttWork();

  //Check Alarm On
  if(checkKeyOn()){
    if(!checkLineOk()){
      alarmOn = 1;
    }
  }

  //Make Buzzer Up
  if(alarmOn){
      alarmMode();
  }else{
    digitalWrite(beePin, LOW);
  }

  if(stopButton){
    alarmOn = 0;
    publishTime = 0;
  }
  if(downButton){
    killed();
  }
  if(bibiButton){
    bibi();
    bibiButton = 0;
  }
}

//Make Mqtt work
void MqttWork(void){
  //Check Mqtt connect or not
  if(!mqttClient.connected()) {
    //Connect Mqtt to topic, and also check its successed
    if (mqttClient.connect(topic)) {
      mqttClient.publish(topic, "LockAlarm Online!!");
      Serial.println("Connected.");    
      mqttClient.subscribe(topic);
      Serial.println("Subscribed.");
    }
  }
  mqttClient.loop();
}

//When lock return true, unlock return false
bool checkKeyOn(void){
  keystate0 = keystate;
  keystate = digitalRead(keyPin);
  if(keystate0 == 0 && keystate == 1)
      mqttClient.publish(topic, "Lock on");
  if(keystate0 == 1&& keystate == 0)
      mqttClient.publish(topic, "UnLock");
  if(keystate && keystate0){
    return true;
  }
  else{
    return false;
  }
}

//When Line is not broken return true, broken return false
bool checkLineOk(void){
  linestate0 = linestate;
  digitalWrite(linePin, LOW);
  linestate = digitalRead(linePin);
  if(linestate && linestate0)
    return true;
  else
    return false;
}

//Alarm action
void alarmMode(void){
  if(!publishTime){
    Serial.println("Line got break");
    mqttClient.publish(topic, "Your Lock broken");
    publishTime = 1;
  }
  buzzer();
  delay(10);
}

//Buzzer work in Alarm Mode
void buzzer(void){
  for(timeCount=0; timeCount<60; timeCount++){
    digitalWrite(beePin, HIGH);
    delay(10);
  }
  for(timeCount=0; timeCount<20; timeCount++){
    digitalWrite(beePin, LOW);
    delay(10);
  }
}

//Stop whole system
void killed(void){
  digitalWrite(beePin, LOW);
  mqttClient.publish(topic, "Get into UNWORK_MODE, Press RESET_BUTTON can work again.");
  while(1);
}

//Buzzer work in Im_here Mode
void bibi(void){
  digitalWrite(beePin, HIGH);
  delay(150);
  digitalWrite(beePin, LOW);
  delay(60);
  digitalWrite(beePin, HIGH);
  delay(150);
  digitalWrite(beePin, LOW);
}

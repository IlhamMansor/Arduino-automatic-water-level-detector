#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

//thingspeak API setup
String host= "api.thingspeak.com";
String url = "/apps/thingtweet/1/statuses/update";
int httpPort = 80;

//pin setup
int TRIGGER = D3;
int ECHO   = D2;  //ultrasonic sensor pin
int dataUltrasonic;
int FloatSensor = D5; //float sensor pin
float floatSensorData;
int relay = D6; //relay pin

//wifi setup
HTTPClient http;
const char* ssid = "/*Put your wifi ssid*/";
const char* password = "/*Put your wifi password*/";

//For timer post twitter
unsigned long myTime;
unsigned long postDelay = 60000; //delay post twitter 1000 = 1s
unsigned long lastPostTime;
bool firstPost = true;

//hashtag for put hashtag in post
String hashTags[] = {"#fyp","#flood","#waterleveldetector","#TamanSeriRaia"};
const size_t n = sizeof(hashTags) / sizeof(hashTags[0]);

//wifi connect and autoconnect function
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  //The ESP8266 tries to reconnect automatically when the connection is lost
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
} 
String randHashtag(){
  String tags = "";
  int k = 0;
  
  for(size_t i = 0; i < n-1; i++){
    size_t j = random(0, n - i);

    String t = hashTags[i];
    hashTags[i] = hashTags[j];
    hashTags[j] = t;
  }

  while(k < n){
    tags += " "+hashTags[k]+"";
    k++;
  }
  return tags;
}

//function for what to post on twitter
void postTwitter(float heightValue){
 int httpCode = 400;
 String value = String(heightValue);
 String hashTag = randHashtag ();
 http.begin(host,httpPort,url);
 String RequestBody = "api_key=S1IH3NI36YVMYHVK"; //change the API key to yours get from thingspeak API
        RequestBody += "&status=Caution! Water high is "+value+"!";
 
 if(firstPost){
  firstPost = false;
  Serial.println("First post sending to twitter");
  while(httpCode != 200){
   Serial.println("Fail to post. retrying");
   httpCode = http.POST(RequestBody);
   hashTag = randHashtag ();
   RequestBody += " "+hashTag+"";
   delay(30000);
  }
  lastPostTime= myTime;
 }else{
  if(myTime >= lastPostTime+postDelay){
    Serial.println("Sending to twitter");
   while(httpCode != 200){
    Serial.println("Fail to post. retrying");
    httpCode = http.POST(RequestBody);
    hashTag = randHashtag ();
    RequestBody += " "+hashTag+"";
    delay(30000);
   }
   lastPostTime= myTime;
  }
 }
 
  Serial.print("Last post time :");
  Serial.print(lastPostTime);
}

void setup() {
 pinMode(TRIGGER, OUTPUT); 
 pinMode(ECHO, INPUT); 
 pinMode(FloatSensor, INPUT_PULLUP); //Arduino Internal Resistor 10K
 pinMode(relay,OUTPUT);
 digitalWrite (relay,LOW);
 randomSeed(analogRead(A0));
 
  delay(1000);
  Serial.begin(74880);
  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  delay(4000);
}

void ultrasonicData(){
  digitalWrite(TRIGGER, LOW);  
  delayMicroseconds(2); 
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10); 
  digitalWrite(TRIGGER, LOW);
  long duration = pulseIn(ECHO, HIGH);
  dataUltrasonic = 18.5 - ((duration/2) / 33.24);

  if (dataUltrasonic < 8){
    digitalWrite(relay,HIGH);
  }else{
    digitalWrite(relay,LOW);
  }
}

void loop() {
  floatSensorData = digitalRead(FloatSensor);
  if(floatSensorData == HIGH){
    Serial.println("Float sensor - ON");
    ultrasonicData();
    postTwitter(dataUltrasonic);
  }else{
    Serial.println("Float sensor - OFF");
  }
  Serial.println(floatSensorData);
  delay(2000);

  Serial.print("Time: ");
  myTime = millis();

  Serial.println(myTime); // prints time since program started
  delay(1000);            // wait a second so as not to send massive amounts of data
}

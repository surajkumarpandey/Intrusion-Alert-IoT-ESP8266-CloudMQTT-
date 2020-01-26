#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>


const char* mqttServer = "********";                                     //add the cloudmqtt credential
const int   mqttPort = ********;                                         //add the cloudmqtt credential
const char* mqttUser = "********";                                       //add the cloudmqtt credential
const char* mqttPassword = "********";                                   //add the cloudmqtt credential

const char* ssid = "********";                                           //add WiFi credential
const char* password = "********";                                       //add WiFi credential
  
WiFiClient espClient;
PubSubClient client(espClient);

const int trigPin = 2;  //D4                                             //ultrasonic sensor's trigger pin
const int echoPin = 0;  //D3                                             //ultrasonic sensor's echo pin
long duration;                                
int distance;                                                            //distance of door from ultrasonic sensor

int frequency=350;                                                       //buzzer frequency specified in Hz        
int buzzPin=D1;                                                          //buzzer pin
int timeOn=70;                                                           //duration of buzzer in milliseconds

String opened = "Door opened: ";                                         //alert for android app
String closed = "Door closed: ";                                         //alert for android app
time_t now;


void setup()
{
  pinMode(trigPin, OUTPUT);                                              // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);                                               // Sets the echoPin as an Input
  pinMode(buzzPin, OUTPUT);
  Serial.begin(9600);   

  WiFi.begin(ssid, password);                                            //wifi connection initiated
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.println("Connecting to WiFi.."+(String)ssid);
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);    
  while (!client.connected()) 
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword ))        //mqtt connection initiated
    {
      Serial.println("connected");
    } 
    else 
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }  

  configTime(0, 0, "pool.ntp.org");                                     //configuration to fetch current time
  setenv("TZ", "UTC-05:30", 1);
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) 
  {
    Serial.print(".");
    delay(1000);
  }
  

}
void loop()
{
  digitalWrite(trigPin, LOW);                                          //sonic blast process initiation for measuring ultrasonic sensor's distance
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance= duration*0.034/2;                                         //ultrasonic sensor's distance from door calculated
  Serial.println(distance);      
  if(distance>10)                                                     //if door opened
    {
    opened = "Door opened, Distance: ";
    opened = opened + distance;
    now = time(nullptr);
    opened = opened + (String)ctime(&now)+" ";
    tone(buzzPin, frequency);                                         //buzzer activated
    delay(timeOn);
    noTone(buzzPin);
    delay(1800);
    Serial.println(ctime(&now));
    client.publish("door", (opened).c_str());                         //message sent to cloud using "door" topic. The same topic is to be added in the the android app as well
    }
    else                                                              //if door is closed
    {
    now = time(nullptr);
    closed= "Door closed, Distance: ";
    closed= closed + distance + " at " + (String)ctime(&now);
    client.publish("door", (closed).c_str());                         //message sent to cloud
    delay(1000);
    }
  
}

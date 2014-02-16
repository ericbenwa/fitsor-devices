#include <SPI.h>
#include <WiFi.h>
#include <Pitches.h>

/*
Pull Up Device
by Eric Benoit
*/

int active = 0;
int configPin = 2;                // choose the pin for the LED
int val = 0;                    // variable for reading the pin status

String postURLstart = "GET /api/exercises/test.php?";
String user_id = "user_id=1&";
String device_udid = "device_udid=1&";
String exercise_type_id = "exercise_type_id=1&";
String repetitions = "repetitions=";
String postURLend = " HTTP/1.1";
String postURL;


int pullups_total_arduino;      // total number of pull ups for the day
String pullup_string = "pullups_total=";
String pullups_total;
long timeBetween;               // using a timer between pull ups to identify inactivity
long timestampOfLastPullUp;     // when my last pullup started
long elapsedTimeSinceLastPullUp;// time between pull ups, used to determine active status
int activePerson = 0;

// Wifi Setup
char ssid[] = "flux capacitor"; //  your network SSID (name) 
char password[] = "applepie";   // your network password
int status = WL_IDLE_STATUS;
WiFiClient client;

// notes in the melody:
int melody[] = { NOTE_A3 };
int noteDurations[] = { 4 };

// Server address
char server[] = "www.fitsor.com";


void setup() {
  pinMode(configPin,OUTPUT);       //make Calibration pin output
  Serial.begin(9600);           // opens serial port, sets data rate to 9600 bps
}


void loop(){
digitalWrite(configPin,HIGH);    //raise the reset pin high
delay(50);                      //start of calibration ring
float sensorvalue = analogRead(A1); //get analog sensor value from pin 0
float inchvalue = (254.0/1024.0) *2.0* sensorvalue; //convert to inches
//Serial.print("Sensed a-d value:"); //print a-d text
//Serial.println(sensorvalue);       //print a-d value
///Serial.print("Inch value=");      //print inch text
//Serial.println(inchvalue);        //print inch value
delay(50);                      //optional delay 1 second
digitalWrite(configPin,LOW);      //turn off Calibration ring and sensor
delay(50);                     //delay 1 second

  if (inchvalue < 20 && inchvalue > 0 && active == 0) {
    pullUp();
    Serial.print("---- PULLUP -----");
    active = 1;
    Serial.print("i see you: Active = ");
    Serial.println(activePerson);
    //Serial.println(cm);
  } else if (inchvalue > 20 || inchvalue == 0) {
    active = 0;
//    Serial.print("nothing in front: Active = ");
//    Serial.println(activePerson);
  }
  
  sendData();

}


void pullUp(){                             // actions performed for each pull up
  pullups_total_arduino++;                 // adding 1 to todays pull up total
  long currentTime = millis();
  activePerson = 1;
  timeBetween = currentTime - timestampOfLastPullUp;     
  timestampOfLastPullUp = currentTime;  
  Serial.print("Time since last pullup ");
  Serial.println(timeBetween);
  Serial.print(pullups_total_arduino);
  Serial.println(" = pull up total");
  beep();
}

void sendData(){                          // actions performed when we want to upload data to the web
  // If I am currently doing pullups, then let's see if I've finished
  if (activePerson == 1) {
    long currentTime = millis();
    elapsedTimeSinceLastPullUp = currentTime - timestampOfLastPullUp;
 
    if (elapsedTimeSinceLastPullUp > 5000){              // after 5 seconds from last pull up lets upload the data
      connectWifi();
      Serial.println("Attempting to send data");
      postData();
      Serial.println("----------");
      Serial.println("Data sent!");
      Serial.print(pullups_total_arduino);
      Serial.println(" pullups sent.");
      Serial.print("String pullups_total = ");
      Serial.println(pullups_total);
      client.stop();
      Serial.println("Disconnected from Wifi");
      Serial.println("----------");
      pullups_total_arduino = 0;
      timeBetween = 0;                    // (not working) resetting the timer to 0 so the device doesn't keep uploading data
      activePerson = 0;
    }
  }
}


void postData() {
//  pullups_total = pullup_string + pullups_total_arduino;

//  postURL = postURLstart + pullups_total_arduino + postURLend;
//  Serial.println(postURL);

  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    client.print("GET /api/exercises/post.php?");
    Serial.print("GET /api/exercises/post.php?");
    
    client.print(user_id);
    Serial.print(user_id);
    
    client.print(device_udid);
    Serial.print(device_udid);
    
    client.print(exercise_type_id);
    Serial.print(exercise_type_id);
    
    client.print(repetitions);
    Serial.print(repetitions);
    
    client.print(pullups_total_arduino);
    Serial.print(pullups_total_arduino);
    
    client.println(postURLend);
    Serial.println(postURLend);
    
    client.println("Host: www.fitsor.com");
    client.println("User-Agent: Arduino/1.0");
    client.println("Content-Type: text/html;");
    client.println("Connection: close");
//    client.print("Content-Length: ");
//    client.println(postURL.length());
//    client.println(pullups_total.length());
    client.println();
//    client.println(pullups_total);
  } 
  else {
    Serial.println("Connection failed");
    Serial.println("Disconnecting.");
    client.stop();
  }
}


void connectWifi() {
  while (status != WL_CONNECTED) {
    Serial.println("----------");
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);
    delay(10000);
  }
  printWifiStatus();
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void beep() {
  Serial.println("I'm beeping");
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 1; thisNote++) {

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/noteDurations[thisNote];
    tone(8, melody[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
}

/*
Credits
Dhaval Karwa
Ben Salinas
John Park - makeprojects.com/Project/PIR-Sensor-ArduinoAlarm/72/1
Tom Igoe - arduino.cc/en/Tutorial/WiFiWebClientRepeating
*/

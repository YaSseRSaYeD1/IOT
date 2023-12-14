#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiMulti.h>
#include <BlynkSimpleEsp32.h>

char auth[] = "F0-o_Vv0J1y9doJlcuGK32Eg6YOvxUhH";
char ssid[] = "////////////.";
char pass[] = "/////////////////////////";

WiFiMulti WifiMulti;

#define wifiStatus 25
#define serverStatus 26

bool connectedOrNot = false;
bool initialDisconnect = false;
bool blynkConnection = false;

WidgetLED idleLED(V1);
WidgetLED measuringLED(V2);
WidgetLED runningLED(V3);
BlynkTimer timer;

#define RXD1 16
#define TXD1 17

int voltage = 0;
int current1 = 0;
int current2 = 0;
int loadControl = 0;
String dataStream;

void SendData()
{
  Serial1.write(9);
  dataStream = Serial1.readStringUntil('_');
  /* 00000 00000 000 */
  String voltageString = dataStream.substring(2, 5);
  String currentString1 = dataStream.substring(6, 10);
  //String currentString2 = dataStream.substring(11, 15);
  String idleStateString = dataStream.substring(20, 21);
  String runningStateString = dataStream.substring(21, 22);
  String measuringStateString = dataStream.substring(22, 23);

  voltage = voltageString.toInt();
  current1 = currentString1.toInt();
  //current2 = currentString2.toInt();

  Blynk.virtualWrite(V4, voltage);
  Blynk.virtualWrite(V5, current1);
  //Blynk.virtualWrite(V6, current2);

  if (voltage > 250)
  {
    Blynk.notify("WARNING: Overvoltage Detected!");
    Blynk.email("", "Electric Guard", "WARNING: Overvoltage Detected!");
  }
  
  if (idleStateString == "0")
  {
    idleLED.off();
  }
  else if (idleStateString == "1")
  {
    idleLED.on();
  }

  if (measuringStateString == "0")
  {
    measuringLED.off();
  }
  else if (measuringStateString == "1")
  {
    measuringLED.on();
  }
  if (runningStateString == "0")
  {
    runningLED.off();
  }
  else if (runningStateString == "1")
  {
    runningLED.on();
  }
}

BLYNK_WRITE(V8)
{
  loadControl = param.asInt();
  if (loadControl == 0)
  {
    digitalWrite(serverStatus, LOW);
  }
  else
  {
    digitalWrite(serverStatus, HIGH);
  }
}

void setup()
{
  pinMode(wifiStatus, OUTPUT);
  pinMode(serverStatus, OUTPUT);
  digitalWrite(wifiStatus, LOW);
  digitalWrite(serverStatus, HIGH);
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);

  Blynk.begin(auth, ssid, pass);
  Blynk.notify("System Connected!");
  timer.setInterval(1000L, SendData);
  Blynk.virtualWrite(V8, 1);
}

void loop()
{
  if ((WiFi.status() == WL_CONNECTED)) 
  {
    digitalWrite(wifiStatus, HIGH);
    if (connectedOrNot == false)
    {
      connectedOrNot = true;
      initialDisconnect = false;
      while (!Blynk.connected())
      {
        if (blynkConnection == false)
        {
          blynkConnection = true;
          digitalWrite(serverStatus, LOW);
        }
        Blynk.connect();
        delay(1000);
      }
      blynkConnection = false;
      digitalWrite(serverStatus, HIGH);
      delay(1000);
    }
    Blynk.run();
    timer.run();
  }
  else
  {
    connectedOrNot = false;
    if (initialDisconnect == false)
    {
      initialDisconnect = true;
      digitalWrite(wifiStatus, LOW);
      digitalWrite(serverStatus, LOW);
    }
  }
}

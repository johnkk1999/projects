#include <TinyGPSPlus.h>

#define gsmTx 4
#define gsmRx 5
#define gpsRx 13
#define gpsTx 12

#include "behaviorCore.h"
//SerialPIO Serial2(gsmTx, gsmRx,fifosize);
//SerialPIO Serial2(gpsTx, gpsRx, fifosize);


SerialPIO gsmSerial(gsmTx, gsmRx);
//SerialPIO Serial2(gpsTx, gpsRx,512);
TinyGPSPlus gps;
void deleteAllSms();
void sendText(String );
void count();

bool gsmRead = true;
bool gpsRead = false;
bool read_text = false;
uint8_t callStatus = 0;
uint8_t checkCallStatus();

String gsmData = "";
String gpsData = "";
String gpsLoc = "";
String beacon = "";

char *phoneNumber = "+254776797754";
volatile int ticks = 0;
volatile int fenceRadius = 1000;

void setup()
{
  Serial.begin(9600);
  delay(500);

  gsmSerial.begin(9600);
  delay(500);
  Serial2.begin(9600);
  delay(500);
  gsmSerial.printf("AT+IPR=9600\n");
  delay(500);
  gsmSerial.printf("ATE0\n");
  delay(500);
  gsmSerial.printf("AT+CMGF=1\n");
  deleteAllSms();
  //sendText("hellow");
  //  makeCall();
  delay(200);
}
void loop()
{
  //check for incoming calls
  if (3 == checkCallStatus())
  {
    Serial.println(gpsLoc);
    sendText(gpsLoc);
    terminateCall();
  }

  //update gsm serial
  readGsmSerial();
  //update gps Serial
  readGpsSerial();
  // Handling gps data
  //Checking for incoming call

  if (gpsRead == true)
  {

    gpsLoc = String(gps.location.lat(), 6);
    gpsLoc += ",";
    gpsLoc += String(gps.location.lng(), 6);
    Serial.println("<=====>");
    Serial.println(gpsLoc);
    Serial.println("<=====>");
    gpsRead = false;
  }

  //Handling gsm data
  if (gsmRead == true)
  {
    Serial.println(gsmData);
    String str = getCommand(gsmData);
    gsmRead = false;

  }



  Serial.print("Radius: ");
  Serial.println(fenceRadius);
  Serial.print("Beacon: ");
  Serial.println(beacon);
  if (read_text == true)
  {
    readText();


  }
  count();
  //  Serial.println("<=========>");
}

void readGsmSerial()
{
  gsmData = "";
  while (gsmSerial.available() > 0)
  {
    char ch = (char) gsmSerial.read();
    gsmData += ch;
    if (ch == '\n')
    {
      gsmRead = true;
    }
  }
}
void readGpsSerial()
{
  while (Serial2.available() > 0)
  {

    if (gps.encode(Serial2.read()))
    {
      gpsRead = true;
    }

  }
}
void readText()
{
  //  gsmSerial.printf("AT+CMGL=\"ALL\"\n");
  gsmSerial.printf("AT+CMGR=1\n");
  delay(200);
  readGsmSerial();
  getCommand(gsmData);

}
String getCommand(String str)
{
  byte first = str.indexOf("#&");
  byte second = str.indexOf("&", first + 3);
  if ((second - first) > 5) {
    String str2 = str.substring(first + 3, second);
    uint8_t fc = str2.indexOf(",");
    uint8_t sc = str2.indexOf(",", fc + 1);
    uint8_t tc = str2.indexOf(",", sc + 1);

    String locGet = str2.substring(0, fc);
    String radSet = str2.substring(fc + 1, sc);
    int radius = str2.substring(sc + 1, tc).toInt();
    String beaconSet = str2.substring(tc + 1, tc + 2);
    Serial.printf("%c %c %d %c \n", locGet[0], radSet[0], radius, beaconSet[0]);

    //Sending Current Location
    if (locGet[0] == 'T')
    {
      //Send current loction
      sendText(gpsLoc);
    }
    //setting radius of the fence
    if (radSet[0] == 'T')
    {
      fenceRadius = radius;
    }
    //setting the beacon
    if (beaconSet[0] == 'T')
    {
      beacon = gpsLoc;
    }
    deleteAllSms();
    delay(300);
    return str2;
  }
  else
    return "no valid command";



}

void terminateCall()
{

  gsmSerial.printf("ATH\r\n");
  delay(100);
  Serial.println("Call terminated");
}
uint8_t checkCallStatus()
{
  gsmData = "";

  gsmSerial.printf("AT+CPAS\r\n");
  readGsmSerial();
  gsmRead = false;

  return gsmData.substring(gsmData.indexOf("+CPAS: ") + 7, gsmData.indexOf("+CPAS: ") + 9).toInt();

}
void sendText(String str)
{

  gsmSerial.printf("AT+CMGS=\"%s\"\n", phoneNumber);

  //  gsmSerial.print(phoneNumber);
  //  gsmSerial.println("\"");
  gsmSerial.printf("%s", str.c_str());
  gsmSerial.printf("%c", (char)26);
  delay(2000);
  Serial.println("Text sent");
}
void makeCall()
{
  gsmSerial.printf("ATD+ %s;\n", phoneNumber);
  delay(3000);
  gsmSerial.printf("ATH\n");
}
void deleteAllSms()
{
  Serial.println("Deleting sms");
  gsmSerial.printf("AT+CMGDA=\"DEL ALL\"\n");
  delay(1000);

}
void count()
{
  if (ticks < 10 )
  {
    //    String strs = "hello from arduino";
    read_text = true;

    // sendText(strs);
  }
  else if (ticks > 10 && ticks < 20)
  {
    read_text = false;
  }
  else if (ticks > 20)
  {
    ticks = 0;
  }

  ticks++;
}

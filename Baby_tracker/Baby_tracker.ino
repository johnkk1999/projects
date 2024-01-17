#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

//GSM RX AND TX PIN
#define GsmRxPin 11
#define GsmTxPin 10
//GPS TX AND RX PIN
#define GpsRxPin 4
#define GpsTxPin 3

TinyGPSPlus gps;
SoftwareSerial gsmSerial(GsmTxPin, GsmRxPin);
SoftwareSerial gpsSerial(GpsTxPin, GpsRxPin);

String gsmData = " ";

String gpsLoc = "";

bool gsmRead = true;
bool gpsRead = false;

bool alertTest = true;

char *phoneNumber = "+254710175999";

//float initialLatitude = -1.046027;
//float initialLongitude = 37.095085;

float initialLatitude =  -1.5426450;
float initialLongitude = 37.262116;
const float maxDistance = 250;

float latitude, longitude;
unsigned long currentMillis;
void setup()
{
  Serial.begin(9600);
  delay(100);
  gsmSerial.begin(9600);
  gpsSerial.begin(9600);
  delay(1000);
  gsmSerial.print("AT+IPR=9600\n");
  delay(500);
  gsmSerial.print("ATE0\n");
  delay(500);
  gsmSerial.print("AT+CMGF=1\n");
  delay(500);
currentMillis = millis();

}
void loop()
{

  if(millis() - currentMillis > 6000 && alertTest == true)
    {
      sendAlert(latitude,longitude);
      alertTest = false;
    }
  //get Gsm data
  readGsmSerial();
  //update gps Serial
  readGpsSerial();

  // Handling gps data(process gps data after complete gps serial read)
  if (gpsRead == true)
  {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    float distance = getDistance(latitude, longitude, initialLatitude, initialLongitude);
    if (distance > maxDistance)
    {
      
    //  sendAlert(latitude,longitude);
    }

        Serial.println(distance);
        gpsRead = false;
  }


//Handling gsm data
if (gsmRead == true)
  {
    Serial.println(gsmData);
    //    String str = getCommand(gsmData);
    gsmRead = false;

  }

}

void terminateCall()
{

  gsmSerial.print("ATH\r\n");
  delay(100);
  Serial.println("Call terminated");
}

uint8_t checkCallStatus()
{
  gsmData = "";

  gsmSerial.print("AT+CPAS\r\n");
  readGsmSerial();
  gsmRead = false;

  return gsmData.substring(gsmData.indexOf("+CPAS: ") + 7, gsmData.indexOf("+CPAS: ") + 9).toInt();

}
void sendAlert(float childLat, float childLong)
{
  Serial.println("Sending Alert!!");
  String sms_data = "";
  sms_data = "Alert! Your Child is outside the School.\r";
  sms_data += "https://www.google.com/maps/place/";
  sms_data += String(childLat, 8);
  sms_data += ",";
  sms_data += String(childLong, 8);
  sendText(sms_data);
}
void sendText(String str)
{

  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(phoneNumber);
  gsmSerial.println("\"");

  delay(100);
  gsmSerial.print(str.c_str());
  delay(100);
  gsmSerial.write(26);
  delay(2000);
  Serial.println("Text sent");
}

float getDistance(float flat1, float flon1, float flat2, float flon2) {
  // Variables
  float dist_calc = 0;
  float dist_calc2 = 0;
  float diflat = 0;
  float diflon = 0;
  // Calculations
  diflat = radians(flat2 - flat1);
  flat1 = radians(flat1);
  flat2 = radians(flat2);
  diflon = radians((flon2) - (flon1));
  dist_calc = (sin(diflat / 2.0) * sin(diflat / 2.0));
  dist_calc2 = cos(flat1);
  dist_calc2 *= cos(flat2);
  dist_calc2 *= sin(diflon / 2.0);
  dist_calc2 *= sin(diflon / 2.0);
  dist_calc += dist_calc2;
  dist_calc = (2 * atan2(sqrt(dist_calc), sqrt(1.0 - dist_calc)));
  dist_calc *= 6371000.0; //Converting to meters
  return dist_calc;
}
void readGsmSerial()
{
  gsmData = "";
  while (gsmSerial.available() > 0)
  {
    char ch = (char) gsmSerial.read();
    gsmData += ch;
    Serial.print(ch);
    if (ch == '\n')
    {
      gsmRead = true;
    }
  }
}
void readGpsSerial()
{
  while (gpsSerial.available() > 0)
  {

    if (gps.encode(gpsSerial.read()))
    {
      gpsRead = true;
    }

  }
}

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"  //Provide the token generation process info.
#include "addons/RTDBHelper.h"   //Provide the RTDB payload printing info and other helper functions.

// Insert your network credentials
#define WIFI_SSID "john-hp3"
#define WIFI_PASSWORD "kali2010"
// Insert Firebase project API Key
#define API_KEY "AIzaSyCUBHBuvPKQwy6gUPcx_zXVI7iQ6vucFtk"
// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://lesson-attendance-32885-default-rtdb.firebaseio.com/"

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SoftwareSerial fingSensor(13, 12);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingSensor);
//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

//uint8_t id = -1;
//String classFile = "1,Mary Right,j26-1102-2018,\n2,John Right,j26-1114-2018,\n3,Mary Left,j26-1102-2018,\n4,John left,j26-1114-2018,\n5,Esther Right,j26-1120-2018\n";
String attendees = "";
int mode = 1;
int counter = 0;

int students[10] = { 0 };


String getStudentName(int);
String getStudentReg(int);
int getNumberOfStudents();
void setStudentRecord(int);
int getStudentCount(int);

void setup() {
  Serial.begin(9600);
  delay(100);
  finger.begin(57600);

  delay(200);

  //wifi initialise
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");

  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initilise finger print sensor
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
  delay(1000);
  //Initialise screen
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  // display.emptyDatabase();

  delay(1000);
  // Serial.println("Getting total number of students");
  // while (Firebase.ready()) {
  //   if (Firebase.RTDB.getString(&fbdo, "sNo/no")) {
  //     if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_string) {
  //       studentNo = String(fbdo.to<String>()).toInt();
  //       Serial.println(studentNo);
  //       break;
  //     }
  //   }
  // }
  Serial.println("<================>");
  getNumberOfStudents();
  getStudentName(2);
  getStudentReg(2);
  setStudentRecord(2);
}

void loop() {

  // int studentNo = 0;
  if (mode == 0) {
    int no_students = getNumberOfStudents();
    Serial.println("Enrolling ");

    for (int i = 1; i <= no_students; i++) {
 
      // String str2 = String(i);
      // str2 += ",";
      // uint8_t first = classFile.indexOf(str2) + 2;
      // uint8_t second = classFile.indexOf(",", first);
      // uint8_t third = classFile.indexOf(",", second + 1);
      // String name = classFile.substring(first, second);
      // String regNo = classFile.substring(second + 1, third);
      String name = getStudentName(i);
      String regNo = getStudentReg(i);
      Serial.println(name);
      Serial.println(regNo);
      name += "\nplace\nfinger";
      dispString(name, true);
      while (!getFingerprintEnroll(i))
        ;
      delay(3000);
    }
  }
  //verification of finger prints
  else {
    Serial.println("Finger verifiction");
    uint8_t v_id = getFingerprintID();
    if (v_id > 0) {
      if (false == checkVerified(v_id)) {
        // records verified id
        students[counter] = v_id;
        counter++;
        if (counter > 9)
          counter = 0;


        // String str2 = String(v_id);
        // str2 += ",";
        // uint8_t first = classFile.indexOf(str2) + 2;
        // uint8_t second = classFile.indexOf(",", first);
        // uint8_t third = classFile.indexOf(",", second + 1);

        // String name = classFile.substring(first, second);
        // String regNo = classFile.substring(second + 1, third);

        String name = getStudentName(v_id);
        String regNo = getStudentReg(v_id);
        setStudentRecord(v_id);
        dispString(name, true);
        dispString(regNo, false);
        dispString("Verified", false);

        //Marking the register for lesson attendance

        attendees += name;
        attendees += ",";
        attendees += regNo;
        attendees += '\n';
        delay(3000);
      } else {
        dispString("Already\nVerified", true);
        delay(3000);
      }
    }
  }
  // first = classFile.indexOf("1,") + 2;
  // second = classFile.indexOf(",");
  // display.println(first, second);

  //finger enrolling
  Serial.println(attendees);
  delay(1000);
}
String getStudentName(int id) {
  String student = "";
  if (Firebase.ready()) {
    String str = "students/";
    str += String(id);
    str += "/Name";
    if (Firebase.RTDB.getString(&fbdo, &str[0])) {
      if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_string) {
        String str1 = fbdo.to<String>();
        uint8_t first = str1.indexOf("\"");
        uint8_t second = str1.indexOf("\"", first + 2);
        student = str1.substring(first + 1, second - 1);
        Serial.println(student);
      }
    }
  }
  return student;
}

String getStudentReg(int id) {
  String studentReg = "";
  if (Firebase.ready()) {
    String str = "students/";
    str += String(id);
    str += "/reg";
    if (Firebase.RTDB.getString(&fbdo, &str[0])) {
      if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_string) {
        String str1 = fbdo.to<String>();
        uint8_t first = str1.indexOf("\"");
        uint8_t second = str1.indexOf("\"", first + 2);
        studentReg = str1.substring(first + 1, second - 1);
        Serial.println(studentReg);
      }
    }
  }
  return studentReg;
}

int getNumberOfStudents() {
  int studentNo = 0;
  while (Firebase.ready()) {
    if (Firebase.RTDB.getString(&fbdo, "sNo/no")) {
      if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_string) {
        studentNo = String(fbdo.to<String>()).toInt();
        Serial.println(studentNo);
        return studentNo;
      }
    }
  }
  return 0;
}

void setStudentRecord(int id) {
  int currCount = getStudentCount(id);
  currCount++;
  String str = "students/";
  str += String(id);
  str += "/Count";
  if (Firebase.RTDB.setString(&fbdo, &str[0], currCount)) {
    Serial.println("Record set");
  }
}
int getStudentCount(int id) {
  int count = 0;
  String str = "students/";
  str += String(id);
  str += "/Count";
  if (Firebase.ready()) {

    if (Firebase.RTDB.getString(&fbdo, &str[0])) {
      if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_string) {
        count = String(fbdo.to<String>()).toInt();
        Serial.println(count);
      }
    }
  }
  return count;
}
bool checkVerified(int ids) {
  for (int i = 0; i < 10; i++) {
    Serial.println(students[i]);
    if (students[i] == ids)
      return true;
  }
  return false;
}
void dispString(String str, bool clear) {
  if (clear == true) {
    display.clearDisplay();
    display.setCursor(0, 0);
  }


  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.println(str);
  display.display();
}
uint8_t getFingerprintEnroll(int id) {
  int p = -1;
  Serial.println("Waiting for valid finger to enroll as #");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        dispString("Image\ntaken", false);
        break;
      case FINGERPRINT_NOFINGER:
        break;
    }
  }
  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      dispString("image\nconverted", false);
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
  }

  dispString("Remove\nFinger", true);

  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
loop:
  dispString("Place\nFinger", true);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        dispString("Image\nTaken", false);
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
    }
  }
  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      dispString("image\nconverted", true);
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    dispString("print\nmatched", true);
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    goto loop;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Enrolled");
    dispString("Enrolled", true);
  }
  return true;
}



uint8_t getFingerprintID() {
  dispString("Place\nFinger", true);
  uint8_t p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        dispString("Image\ntaken", false);
        break;
      case FINGERPRINT_NOFINGER:
        // Serial.println(".");
        break;
    }
  }
  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return 0;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    return finger.fingerID;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    dispString("No Match", true);
    dispString("Try Again!", false);
    delay(3000);

  } else {
    Serial.println("Unknown error");
  }

  return 0;
}

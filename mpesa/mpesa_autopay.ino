#include <SoftwareSerial.h>
#include <SIM800L.h>

SoftwareSerial sim800lSerial(10, 11); // RX, TX pins
SIM800L* sim800l;

#define SIM800_RX_PIN 4
#define SIM800_TX_PIN 5
#define SIM800_RST_PIN 6
void setup() {
  Serial.begin(9600);
  while (!Serial);

  SoftwareSerial* serial = new SoftwareSerial(SIM800_RX_PIN, SIM800_TX_PIN);
  serial->begin(9600);
  delay(1000);

  // Wait for the module to register on the network
  while (!sim800l->registered()) {
    delay(1000);
  }

  // Connect to GPRS
  while (!sim800l->gprsConnect("your_apn")) {
    delay(1000);
  }

  // Wait for a stable connection
  delay(2000);
}

void loop() {
  // Define the request headers
  String headers = "Content-Type: application/json\r\n";
  headers += "Authorization: Bearer YuhAtU2ZedAkDhbtPStagve7q2Wi";

  // Define the request data
  String data = "{"
                "\"BusinessShortCode\": 174379,"
                "\"Password\": \"MTc0Mzc5YmZiMjc5ZjlhYTliZGJjZjE1OGU5N2RkNzFhNDY3Y2QyZTBjODkzMDU5YjEwZjc4ZTZiNzJhZGExZWQyYzkxOTIwMjMwNTI5MjAzNDU3\","
                "\"Timestamp\": \"20230529203457\","
                "\"TransactionType\": \"CustomerPayBillOnline\","
                "\"Amount\": 1,"
                "\"PartyA\": 254708374149,"
                "\"PartyB\": 174379,"
                "\"PhoneNumber\": 254708374149,"
                "\"CallBackURL\": \"https://mydomain.com/path\","
                "\"AccountReference\": \"Johnltd\","
                "\"TransactionDesc\": \"Murangano\""
                "}";

  // Send an HTTPS POST request
  sim800l->httpsPost("sandbox.safaricom.co.ke", "/mpesa/stkpush/v1/processrequest", 443, headers, data);

  // Wait for the response
  while (sim800l->httpIsBusy()) {
    delay(100);
  }

  // Print the response
  String response = sim800l->httpResponse();
  Serial.println(response);

  // Wait for some time before making the next request
  delay(5000);
}

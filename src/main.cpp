#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <map>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  

#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define EM18_RX_PIN 16   // RX pin for EM-18 RFID Reader
#define REMOVE_BUTTON_PIN 4  // GPIO pin for the remove button
#define SESSION_END_BUTTON_PIN 5  // GPIO pin for the session end button

// Wi-Fi credentials
const char* ssid = "Shiva";
const char* password = "shiva62003@";
const char* addItemUrl = "https://shoppingkart-kzkf.onrender.com/addItem";  
const char* remItemUrl = "https://shoppingkart-kzkf.onrender.com/remItem";  
const char* settleBillUrl = "https://shoppingkart-kzkf.onrender.com/settleBill";  

String cartId = "cart1";  // Fixed cartId for each cart
String rfidTag = "";  // To store RFID tag number
float totalCost = 0.0;  // To store the total cost of items in the cart
String lastScannedItem = "";  // To store the last scanned item name
float lastScannedPrice = 0.0;  // To store the last scanned item price
bool isRemoveMode = false;  // Flag to check if we are in remove mode

// Map to store RFID tags and their corresponding item names and prices
std::map<String, String> itemDatabase = {
  {"090031EB2AF9", "Milk - Rs 60.0"},
  {"150026F45D9A", "Sugar - Rs 40.0"},
  {"180045AF00F2", "Bread - Rs 25.0"},
  {"19007EBFA27A", "Apple - Rs 80.0"}
};

std::map<String, float> itemPrices = {
  {"090031EB2AF9", 60.0},
  {"150026F45D9A", 40.0},
  {"180045AF00F2", 25.0},
  {"19007EBFA27A", 80.0}
};

// Function to initialize Wi-Fi
void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

// Function to send item ID and cart ID to the server
void sendItemToServer(String itemId) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(addItemUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> jsonDoc;
    jsonDoc["cartId"] = cartId;
    jsonDoc["itemId"] = itemId;

    String payload;
    serializeJson(jsonDoc, payload);

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.println("Item sent successfully.");
    } else {
      Serial.println("Error sending item: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void remItemToServer(String itemId) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(remItemUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> jsonDoc;
    jsonDoc["cartId"] = cartId;
    jsonDoc["itemId"] = itemId;

    String payload;
    serializeJson(jsonDoc, payload);

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.println("Item sent successfully.");
    } else {
      Serial.println("Error sending item: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

// Function to notify the server of session end
void sendSessionEndToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(settleBillUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> jsonDoc;
    jsonDoc["cartId"] = cartId;

    String payload;
    serializeJson(jsonDoc, payload);

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.println("Session end sent successfully.");

      // Reset local cart state
      totalCost = 0.0;
      lastScannedItem = "";
      lastScannedPrice = 0.0;

      // Update OLED to reflect reset
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print(F("Session Ended"));
      display.setCursor(0, 10);
      display.print(F("Total: Rs 0.0"));
      display.display();
    } else {
      Serial.println("Error sending session end: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}


// Function to update the OLED display
void updateDisplay(bool isItemRemoved = false, String message = "") {
  display.clearDisplay();
  display.setCursor(0,0);

  if (isItemRemoved) {
    display.print(F("Removing Item"));
  } else {
    display.print(F("Item: "));
    display.print(lastScannedItem);
    display.setCursor(0, 10);
    display.print(F("Price: Rs "));
    display.print(lastScannedPrice, 2);
  }

  display.setCursor(0, 20);
  display.print(F("Total: Rs "));
  display.print(totalCost, 2);

  if (message != "") {
    display.setCursor(0, 40);
    display.print(message);
  }

  display.display();
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(REMOVE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SESSION_END_BUTTON_PIN, INPUT_PULLUP);

  // Initialize OLED display
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print(F("RFID Shopping Cart: "));
  display.display();

  // Initialize Serial for RFID Reader and Wi-Fi
  Serial1.begin(9600, SERIAL_8N1, EM18_RX_PIN);
  setupWiFi();
}

void loop() {
  // Read RFID tag
  if (Serial1.available()) {
    rfidTag = "";
    while (Serial1.available()) {
      char c = Serial1.read();
      rfidTag += c;
      delay(5);
    }

    // Check if the tag is in the database
    if (itemDatabase.find(rfidTag) != itemDatabase.end()) {
      if (isRemoveMode) {
        totalCost -= itemPrices[rfidTag];
        Serial.println(F("Item Removed: "));
        Serial.println(itemDatabase[rfidTag]);
        updateDisplay(true);
        isRemoveMode = false;
        remItemToServer(rfidTag);
      }
      else
      {
        lastScannedItem = itemDatabase[rfidTag];
        lastScannedPrice = itemPrices[rfidTag];
        totalCost += lastScannedPrice;

        Serial.println(F("Item Added: "));
        Serial.println(lastScannedItem);
        
        // Update OLED display and send item ID to server
        updateDisplay(false);
        sendItemToServer(rfidTag);
      }
    } else {
      Serial.println(F("Item Not Found"));
      updateDisplay(true, F("Item Not Found"));
    }
  }

  // Check if the remove button is pressed
  if (digitalRead(REMOVE_BUTTON_PIN) == LOW) {
    // Serial.println(digitalRead(REMOVE_BUTTON_PIN));
    isRemoveMode = true;
    delay(500);
  }

  // Check if the session end button is pressed
  if (digitalRead(SESSION_END_BUTTON_PIN) == LOW) {
    // Serial.println(digitalRead(SESSION_END_BUTTON_PIN));
    Serial.println(F("send settle signal"));
    sendSessionEndToServer();
    delay(1000);
  }
}

# Smart Shopping Cart - README  
## **Software Requirements**
1. **PlatformIO extension** in Visual Studio Code.  
2. Libraries:  
   - **Adafruit SSD1306** and **Adafruit GFX**: For OLED display control.  
   - **ArduinoJson**: For JSON handling.  
   - **WiFi** and **HTTPClient**: For Wi-Fi and HTTP communication.  

---

## **Installation and Setup**

### 1. Clone the Repository  
Clone the repository to your local system:  
```bash  
git clone https://github.com/ShivaSai06/shoppingCart_
cd shoppingCart_ 
```  

### 2. Install PlatformIO in VS Code  
1. Open **Extensions** (`Ctrl+Shift+X`) in VS Code.  
2. Search for **PlatformIO IDE** and install it.  

### 3. Open the Project  
Open the folder containing the code in VS Code.  

### 4. Install Dependencies  
Add the following configuration to your `platformio.ini` file:  
```ini  
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

lib_deps = 
   adafruit/Adafruit SSD1306 @ ^2.5.3
   adafruit/Adafruit GFX Library @ ^1.11.11
   bblanchon/ArduinoJson @ ^6.21.2
   Wire  ; Ensures that the Wire library is installed if not included

```  
Run the following command to install the required libraries:  
```bash  
platformio lib install  
```  

### 5. Configure Wi-Fi and Server Details  
Update the following constants in the code (`main.cpp`) with your credentials and server endpoints:  
```cpp  
const char* ssid = "YourWiFiSSID";  
const char* password = "YourWiFiPassword";  
```  

### 6. Upload the Code  
Connect your ESP32 microcontroller to your system and upload the code:  
- Use the **PlatformIO Upload** button in VS Code, or  
- Run the following command:  
```bash  
platformio run --target upload  
```  

### 7. Open the Serial Monitor  
To view logs and debug information, open the Serial Monitor in VS Code:  
```bash  
platformio device monitor  
```  

---

## **How It Works**  
1. **RFID Scanning**: The RFID reader reads item tags and checks them in the local database.  
2. **Display Update**: The OLED screen displays the item's name, price, and updated total cost.  
3. **Server Communication**:  
   - **Add Item**: Sends the scanned item's ID to the server.  
   - **Remove Item**: Sends the item's ID for removal.  
   - **Session End**: Sends the cart's total bill to the server.  
4. **Buttons**:  
   - **Remove Item**: Switches to "Remove Mode" to subtract the cost of an item.  
   - **Session End**: Finalizes the session and resets the cart.  

---


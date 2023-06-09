/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <Arduino.h>
#include <WiFiMulti.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>

#include <UniversalTelegramBot.h>

#include <NTPClient.h>

// https://arduinojson.org/
#include <ArduinoJson.h>

#include <HTTPClient.h>

// // Include certificate data (see note above)
// #include "cert.h"
// #include "private_key.h"

const char *ssid = "Jakie haslo?";
const char *password = "niepamietam";

const char *server = "https://node.rusin.dev";
// const char *serverStatus = "server-status/";
const char *serverStatus = "http://192.168.100.50/server-status/";
// const char *serverUploadEntries = "https://node.rusin.dev/upload-entries";
const char *serverUploadEntries = "http://192.168.100.50:5005/upload-entries";

const char *rusindev_root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEnjCCA4agAwIBAgIUH9/AEti4C0DZDqtx1SfhYU8EqN8wDQYJKoZIhvcNAQEL\n"
    "BQAwgYsxCzAJBgNVBAYTAlVTMRkwFwYDVQQKExBDbG91ZEZsYXJlLCBJbmMuMTQw\n"
    "MgYDVQQLEytDbG91ZEZsYXJlIE9yaWdpbiBTU0wgQ2VydGlmaWNhdGUgQXV0aG9y\n"
    "aXR5MRYwFAYDVQQHEw1TYW4gRnJhbmNpc2NvMRMwEQYDVQQIEwpDYWxpZm9ybmlh\n"
    "MB4XDTIzMDQwNDIyMTIwMFoXDTM4MDMzMTIyMTIwMFowYjEZMBcGA1UEChMQQ2xv\n"
    "dWRGbGFyZSwgSW5jLjEdMBsGA1UECxMUQ2xvdWRGbGFyZSBPcmlnaW4gQ0ExJjAk\n"
    "BgNVBAMTHUNsb3VkRmxhcmUgT3JpZ2luIENlcnRpZmljYXRlMIIBIjANBgkqhkiG\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAoTWBoPPiEWuZW369UWmu4fdP+DQsPzq8vIt5\n"
    "eKzdtTkLaRVYBy7Xqeehl8xg1hoBxfldzFPC0ic4ywvkD6cBanncc884Xpp3EU48\n"
    "rx79I5TRxexZAPo7Ie0Mn1ITupUI7Y+T8Dh0Mhw0QUmW8BEITOd5J+pOFVjl/bQo\n"
    "zxR3vAIt8u8fNuHkJVW7GX4RXBiF4BPKQl/9tEx9CZAGicaOp/aKYwrNDk2pe1qx\n"
    "lqxKYKyRJkO9ytt83VgBTU+7swpFY1n4SBSk0ozvjtovRVL8DlWmn08dAotRXEOb\n"
    "ckmrSc2JdcMVpOCpH26lsLdidNOGeTYKHeGTsi0A7DKi5fh12wIDAQABo4IBIDCC\n"
    "ARwwDgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcD\n"
    "ATAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBRC9yXJ+NG0Jq5wCwk7xLMC8WadNDAf\n"
    "BgNVHSMEGDAWgBQk6FNXXXw0QIep65TbuuEWePwppDBABggrBgEFBQcBAQQ0MDIw\n"
    "MAYIKwYBBQUHMAGGJGh0dHA6Ly9vY3NwLmNsb3VkZmxhcmUuY29tL29yaWdpbl9j\n"
    "YTAhBgNVHREEGjAYggsqLnJ1c2luLmRldoIJcnVzaW4uZGV2MDgGA1UdHwQxMC8w\n"
    "LaAroCmGJ2h0dHA6Ly9jcmwuY2xvdWRmbGFyZS5jb20vb3JpZ2luX2NhLmNybDAN\n"
    "BgkqhkiG9w0BAQsFAAOCAQEAb/wmXubae/b1u7bjzXV8wy2GwbBOiKNCpS4msCed\n"
    "x+/E2phb4QSqCXIdjdHQE6tBKKUR/UdFAOGg6yDiI0BWDyVB+bL2xiVx7SEUHGI/\n"
    "IpLtcXPicUV16zFhVehi07S6xBkIdJXu4tFZr4O7WrePP9uO5SWDBnUgyJhEYFsZ\n"
    "cQdllnZYqLTGGfzoRFvuZN0LGIZR8shDq+rJNO+7i3wQwZZjEwF8+ed+oKNDCcK0\n"
    "5LJoiAnT33gKcTsMIL71BY6X9C94nRDLhsUM9xeKyANCV4vX6Pw0vnwVAEZtPR1J\n"
    "lmvQCkWy4HJEi4FsX6K2gOWRmp+13JsEKoH4LFT+MwIegQ==\n"
    "-----END CERTIFICATE-----";

// const char *server = "https://node.rusin.dev/";

const int ledRed = 2;
const int ledBlue = 1;

const uint32_t connectTimeoutMs = 10000;

#ifdef ESP8266
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

int scanTime = 5; // In seconds
BLEScan *pBLEScan;

WiFiMulti wifiMulti;
WiFiUDP ntpUDP;

// char *ntp_server = "pool.ntp.org";
// int interval = 30000;

NTPClient timeClient(ntpUDP, "pl.pool.ntp.org");

#define BOTtoken "5831606623:AAGxSkxXdHeHM-gSYWBbue2BurW1EM6m5KE" // your Bot Token (Get from Botfather)
#define CHAT_ID "-1001980783840"

WiFiClientSecure client;
HTTPClient http;

// void WiFiClientSecure::setInsecure()
// {
//     _CA_cert = NULL;
//     _cert = NULL;
//     _private_key = NULL;
//     _pskIdent = NULL;
//     _psKey = NULL;
//     _use_insecure = true;
// }

UniversalTelegramBot bot(BOTtoken, client);

/*
 * globals
 */
String topic;
struct tm timeinfo;

uint8_t mac[6];
String my_mac;

int devicesFound = 0;
long scanCount = 0;



/*
 * Given a byte array of length (n), return the ASCII hex representation
 * and properly zero pad values less than 0x10.
 * String(0x08, HEX) will yield '8' instead of the expected '08' string
 */
String hexToStr(uint8_t *arr, int n)
{
  String result;
  result.reserve(2 * n);
  for (int i = 0; i < n; ++i)
  {
    if (arr[i] < 0x10)
    {
      result += '0';
    }
    result += String(arr[i], HEX);
  }
  return result;
}



int generateRandomInt(int min, int max)
{
  // ustawienie ziarna generatora liczb pseudolosowych
  srand(time(NULL));
  // wygenerowanie losowej liczby całkowitej w zakresie [min, max]
  int losowa = rand() % (max - min + 1) + min;
  return losowa;
}

/*
 * Return a string in RFC3339 format of the current time.
 * Will return a placeholder if there is no network connection to an
 * NTP server.
 */
String getIsoTime()
{
  char timeStr[21] = {0}; // NOTE: change if strftime format changes

  // Serial.print(F("Waiting for NTP time sync..."));
  time_t time_now = timeClient.getEpochTime();

  localtime_r(&time_now, &timeinfo);

  if (timeinfo.tm_year <= (2016 - 1900))
  {
    // date is not valid from NTP
    return String("YYYY-MM-DDTHH:MM:SSZ");
  }
  else
  {
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
    Serial.print('Current time: ');
    Serial.println(timeStr);
    return String(timeStr);
  }
}


// Function to make a POST request with JSON data
void sendJsonData(String endpoint, String payload)
{
  // // Convert JSON data to String
  // String jsonString;
  // serializeJson(jsonData, jsonString);

  // Set HTTP headers
  
  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");

  // Send POST request with JSON data
  int httpCode = http.POST(payload);

  // Check for successful request
  if (httpCode != 0)
  {
    Serial.println("POST request successful!");
  }
  else
  {
    Serial.println("Error sending POST request.");
  }

  // Free resources
  http.end();
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    // Serial.printf(advertisedDevice.getAddress().toString().c_str());
    Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());

    // Construct a JSON-formatted string with device information
    StaticJsonDocument<1024> json;

    json["time"] = getIsoTime();
    json["mac"] = hexToStr(*advertisedDevice.getAddress().getNative(), 6);

    String payload = hexToStr(advertisedDevice.getPayload(), advertisedDevice.getPayloadLength());
    json["payload"] = payload.c_str();

    if (advertisedDevice.haveRSSI())
    {
      json["rssi"] = advertisedDevice.getRSSI();
    }

    if (advertisedDevice.haveTXPower())
    {
      json["tx"] = advertisedDevice.getTXPower();
    }

    if (advertisedDevice.haveName())
    {
      json["name"] = advertisedDevice.getName().c_str();
    }

    if (advertisedDevice.haveManufacturerData())
    {
      json["manufacturerData"] = advertisedDevice.getManufacturerData().c_str();
    }

    json["serviceData"] = advertisedDevice.getServiceData().c_str();

    json["serviceDataUUID"] = advertisedDevice.getServiceDataUUID().toString();

    json["isAdvertiedService"] = (bool)advertisedDevice.isAdvertisingService(advertisedDevice.getServiceDataUUID());

    char buffer[1024];
    size_t len = serializeJson(json, buffer);

    // convert array char to String
    String payloadPOST = String(buffer, len);

    Serial.println();
    Serial.println(buffer);

    String endpoint = "http://192.168.1.178/btscan";
    sendJsonData(endpoint, payloadPOST);

    String mac = advertisedDevice.getAddress().toString().c_str();

    delay(5);
  };
};

  void setup()
  {
    Serial.begin(115200);
    Serial.println("Scanning...");

    // pinMode(ledRed, OUTPUT);
    // pinMode(ledBlue, OUTPUT);

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); // create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); // active scan uses more power, but get results faster
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99); // less or equal setInterval value

    delay(3000);
    // WiFi.mode(WIFI_AP_STA);
    WiFi.mode(WIFI_STA);
    // WiFi.macAddressBytes(mac);
    // WiFi.begin(ssid, password);
    wifiMulti.addAP("Jakie haslo?", "niepamietam");
    wifiMulti.addAP("Retech Guest", "retech.pl");
    wifiMulti.addAP("Oaza", "twojamatka");
    wifiMulti.addAP("Pustynia", "1q2w3e4r5");

    // while (WiFi.status() != WL_CONNECTED)
    // {
    //   delay(500);
    //   Serial.print(".");
    // }
    // Serial.println("");
    // Serial.println("WiFi connected");
    // Serial.println("IP address: ");
    // Serial.println(WiFi.localIP());
    while (wifiMulti.run() != WL_CONNECTED)
    {
      Serial.println("WiFi not connected!");
      delay(1000);
    }
    if (wifiMulti.run() == WL_CONNECTED)
    {
      Serial.println("");
      Serial.print("WiFi connected: ");
      Serial.println(WiFi.SSID());
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      // WiFi.setHostname("paranoja-bluetooth");
      Serial.print("Broadcast IP address: ");
      Serial.println(WiFi.broadcastIP());
      Serial.print("DNS server IP address: ");
      Serial.println(WiFi.dnsIP());
      Serial.println();
    }
    delay(4000);
    timeClient.begin();
    timeClient.setTimeOffset(3600);
    delay(1000);
    timeClient.update();
    delay(1000);
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  }

  void blinkLed(int led, int times, int interval = 100)
  {
    for (int i = 0; i < times; i++)
    {
      digitalWrite(led, !digitalRead(led));
      delay(interval);
    }
  }

  void deviceInfo()
  {
    // JSON formatted payload
    StaticJsonDocument<512> status_json;
    status_json["state"] = 'Active';
    status_json["time"] = getIsoTime();
    status_json["uptime_ms"] = millis();
    status_json["packets"] = scanCount;
    status_json["ssid"] = WiFi.SSID();
    status_json["rssi"] = WiFi.RSSI();
    status_json["ip"] = WiFi.localIP().toString();
    status_json["hostname"] = WiFi.getHostname();
    status_json["mac"] = WiFi.macAddress();

    // status_json["version"] = GIT_VERSION;

    char buffer[512];
    serializeJson(status_json, buffer);

    return;
  }

  void loop()
  {

    // if (bufferListObj.getBufferSize() >= maxMacNumber)
    // {
    //   bufferListObj.clearBufferList();
    // }

    // update timeserver + "/server-status";

    scanTime = generateRandomInt(1, 6);

    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    Serial.printf("[SCAN %d] Devices found: ", ++scanCount);
    devicesFound = foundDevices.getCount();
    Serial.println(devicesFound);

    // blinkLed(33, devicesFound, 100);

    Serial.println();
    Serial.printf("Scan done! Scan was %d seconds\n", scanTime);
    pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory

    int wait = 10 * generateRandomInt(10, 99);
    Serial.printf("Restarting at %d miliseconds\n", wait);
    delay(wait);

    // Serial.printf("Server is %s", checkIfServerIsOnline() ? "running\n" : "not running\n");
    Serial.println();

    client.flush();
  }

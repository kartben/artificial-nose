/* 
The MIT License (MIT)

Copyright (c) 2015 Volodymyr Shymanskyy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include <ArduinoHttpClient.h>
#include <ArduinoOTA.h> // only for InternalStorage
#include <rpcWiFi.h>
#include <WiFiClient.h>


void restartMCU();
bool parseURL(String url, String& protocol, String& host, int& port, String& uri);
void enterOTA(String otaPackageURL);


void restartMCU() {
  NVIC_SystemReset();
}

#define DEBUG_PRINT(...) Serial.println(__VA_ARGS__)
#define OTA_FATAL(...) { DEBUG_PRINT(__VA_ARGS__); delay(1000); restartMCU(); }


bool parseURL(String url, String& protocol, String& host, int& port, String& uri)
{
  int index = url.indexOf(':');
  if(index < 0) {
    return false;
  }

  protocol = url.substring(0, index);
  url.remove(0, (index + 3)); // remove protocol part

  index = url.indexOf('/');
  String server = url.substring(0, index);
  url.remove(0, index);       // remove server part

  index = server.indexOf(':');
  if(index >= 0) {
    host = server.substring(0, index);          // hostname
    port = server.substring(index + 1).toInt(); // port
  } else {
    host = server;
    if (protocol == "http") {
      port = 80;
    } else if (protocol == "https") {
      port = 443;
    }
  }

  if (url.length()) {
    uri = url;
  } else {
    uri = "/";
  }
  return true;
}

void enterOTA(String otaPackageURL) {
  String protocol, host, url;
  int port;
  
  DEBUG_PRINT(String("OTA: ") + otaPackageURL);

  if (!parseURL(otaPackageURL, protocol, host, port, url)) {
    OTA_FATAL(F("Cannot parse URL"));
  }

  DEBUG_PRINT(String("Connecting to ") + host + ":" + port);

  Client* client = NULL;
  if (protocol == "http") {
    client = new WiFiClient();
#ifdef USE_SSL
  } else if (protocol == "https") {
    client = &_blynkWifiClient;
    //client = new WiFiClientSecure();
#endif
  } else {
    OTA_FATAL(String("Unsupported protocol: ") + protocol);
  }
  HttpClient http(*client, host, port);
  http.get(url);

  int statusCode = http.responseStatusCode();
  if (statusCode != 200) {
    http.stop();
    OTA_FATAL(String("HTTP status code: ") + statusCode);
  }


  int contentLength = http.contentLength();
  if (contentLength == HttpClient::kNoContentLengthHeader) {
    http.stop();
    OTA_FATAL("Content-Length not defined");
  }

  if (!InternalStorage.open(contentLength)) {
    http.stop();
    OTA_FATAL("Not enough space to store the update");
  }
  //InternalStorage.debugPrint();

  DEBUG_PRINT("Flashing...");

  int written = 0;
  int prevProgress = 0;
  uint8_t buff[256];
  while (client->connected() && written < contentLength) {

    int len = http.readBytes(buff, sizeof(buff));
    if (len <= 0) continue;

    for (int i = 0; i<len; i++) {
      InternalStorage.write(buff[i]);
    }
    written += len;

    const int progress = (written*100)/contentLength;
    if (progress - prevProgress >= 10 || progress == 100) {
      Serial.print(String("\r ") + progress + "%");
      prevProgress = progress;
    }
  }
  Serial.println();
  InternalStorage.close();
  http.stop();

  if (written != contentLength) {
    OTA_FATAL(String("Interrupted at ") + written + " / " + contentLength + " bytes");
  }

  DEBUG_PRINT("=== Update successfully completed. Rebooting.");
  InternalStorage.apply();
}


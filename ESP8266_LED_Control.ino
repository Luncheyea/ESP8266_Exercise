#include <SoftwareSerial.h>

const String SSID = "HTC Portable Hotspot CA84";
const String PASSWORD = "72f800efe130";
const String Port = "9000";
String IP = "";

void(* RESET) (void) = 0; //重置Arduino的function

SoftwareSerial WifiSerial(2, 3); // RX:pin2, TX:pin3
String ATcommand = "";
uint8_t connectID = 0;

void setup() {
  WifiSerial.begin(9600);
  Serial.begin(9600);
  Serial.print("\r\nstart\r\n");

  for (uint8_t pin = 10; pin <= 12; pin++)
    pinMode(pin, OUTPUT);
  digitalWrite(12, HIGH);

  //重啟ESP8266
  ATcommand = "AT+RST";
  WiFiSerialClearBuffer();
  WifiSerial.println(ATcommand);
  delay(1000);

  //測試EDP8266
  ATcommand = "AT";
  setATcommand("Testing..", ATcommand);

  //把ESP8266的Wifi連線模式設成station模式
  ATcommand = "AT+CWMODE=1";
  setATcommand("Operate to station..", ATcommand);

  //連接wifi
  ATcommand = "AT+CWJAP=\"" + SSID + "\",\"" + PASSWORD + "\"";
  setATcommand("Connect to WiFi..", ATcommand);

  //取得IP位址
  ATcommand = "AT+CIFSR";
  String response = setATcommand(ATcommand);
  for (uint8_t index = 11; index <= 22; index++)
    IP += response[index];  //IP位址的顯示在respone字串中11~22的位子
  Serial.print(IP); Serial.print("\r\n");

  //打開"多重連線"
  ATcommand = "AT+CIPMUX=1";
  setATcommand(ATcommand, 15);

  //讓ESP8266進入伺服器模式，等候其他裝置與它連線
  ATcommand = "AT+CIPSERVER=1," + Port;
  setATcommand(ATcommand, 15);

  Serial.print("---Initialize Successfully!---\r\n");
}

void loop() {
  if (WifiSerial.available()) {
    WifiSerial.flush();
    if (WifiSerial.find("+IPD,")) {
      uint8_t connectID = WifiSerial.read() - '0'; //取得連線ID，並從ASCII轉成整數
      String StrWebMsg = "";
      while (WifiSerial.available()) {
        StrWebMsg += (char)WifiSerial.read();
        delay(5);
      }

      Serial.print("connectID=");
      Serial.println(connectID);
      Serial.println(StrWebMsg);
      delay(2000);

      ATcommand = "AT+CIPCLOSE=" + connectID;
      Serial.println(setATcommand(ATcommand)); // 送出「中斷連線」命令
    }
  }

  delay(50);

}

String setATcommand(String ATcmd) {
  String response = ""; // 接收ESP回應值的變數
  WiFiSerialClearBuffer();  //清除資料緩衝區
  WifiSerial.println(ATcmd);  // 送出AT命令到ESP模組
  WifiSerial.flush(); //等待資料傳送完

  while (WifiSerial.available()) {
    char c = WifiSerial.read(); // 接收ESP傳入的字元
    response += c;
  }
  return response; // 回傳ESP的回應
}

void setATcommand(String ATcmd, uint8_t times) {
  Serial.print("Loading..");
  WiFiSerialClearBuffer();  //清除資料緩衝區
  WifiSerial.println(ATcmd);
  WifiSerial.flush(); //等待資料傳送完

  for (uint8_t i = 0; i < times; i++) {
    Serial.print(".");
    String response = "";
    while (WifiSerial.available()) {
      char c = WifiSerial.read(); // 接收ESP傳入的字元
      response += c;
    }

    if (response.indexOf("OK") != -1 || response.indexOf("no change") != -1) {
      Serial.print("OK!\r\n");
      return;
    }
  }
  RESET();  //如果10次都沒"OK"，就重置Arduino
}

void setATcommand(String msg, String ATcmd) {
  Serial.print(msg);
  for (uint8_t i = 0; i < 10; i++) {
    Serial.print(".");
    WiFiSerialClearBuffer();  //清除資料緩衝區
    WifiSerial.println(ATcmd);
    WifiSerial.flush(); //等待資料傳送完

    if (WifiSerial.find("OK")) {
      Serial.print("OK!\r\n");
      delay(500);
      return;
    }
  }
  RESET();  //如果10次都沒"OK"，就重置Arduino
}

void WiFiSerialClearBuffer() {
  while (WifiSerial.available()) {
    WifiSerial.read();
  }
}






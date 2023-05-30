#include <WiFi.h>
#include <esp_now.h>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
  int xz;
  int yz;
  int zz; // 接受到的坐标轴数据
} struct_message;
// Create a struct_message called myData
struct_message myData;

const int in1 = 17;
const int in2 = 16;
const int in3 = 19;
const int in4 = 18;
const int l_en = 22;
const int r_en = 23;
const int output25 = 25; //  舵机信号
const int output26 = 26;
const int output27 = 27;
const int freq = 5000; // 频率
const int ledChannel = 0;
const int resolution = 8;

void l_front(int amount) // 左向前（下同）
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  if (amount > 2100)
    ledcWrite(0,
              100 + (int)((amount - 2100) / 1996.0 * 155)); // 规整数值至0-255
  else
    ledcWrite(0, 100 + (int)(1700 - amount) / 1700.0 * 155);
}

void l_back(int amount)
{
  digitalWrite(in2, LOW);
  digitalWrite(in1, HIGH);
  if (amount > 2100)
    ledcWrite(0, 100 + (int)((amount - 2100) / 1996.0 * 155));
  else
    ledcWrite(0, 100 + (int)(1700 - amount) / 1700.0 * 155);
}

void r_front(int amount)
{
  digitalWrite(in4, LOW);
  digitalWrite(in3, HIGH);
  if (amount > 2100)
    ledcWrite(1, 100 + (int)((amount - 2100) / 1996.0 * 155));
  else
    ledcWrite(1, 100 + (int)(1700 - amount) / 1700.0 * 155);
}

void r_back(int amount)
{
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  if (amount > 2100)
    ledcWrite(1, 100 + (int)((amount - 2100) / 1996.0 * 155));
  else
    ledcWrite(1, 100 + (int)(1700 - amount) / 1700.0 * 155);
}

void stop_lr()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}
void setdoor(int single)
{
  // 上下状态 抬起为0,放下为1
  static unsigned char status;
  // 不动
  if (single == 0)
    return;
  if (status)
  {
    // 设定占空比
    ledcWrite(2, 128); // 测试
    status = 0;
    return;
  }
  ledcWrite(2, 64);
  status = 1;
}
// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("x: ");
  Serial.println(myData.xz);
  Serial.print("y: ");
  Serial.println(myData.yz);
  Serial.print("z: ");
  Serial.println(myData.zz);
  Serial.println();
}

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println(WiFi.macAddress());

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(l_en, OUTPUT);
  pinMode(r_en, OUTPUT);
  ledcSetup(0, freq, resolution);
  ledcAttachPin(l_en, 0);
  ledcSetup(1, freq, resolution);
  ledcAttachPin(r_en, 1);
  // 舵机信号
  ledcSetup(2, 50, resolution); // 50Hz,信道2
  ledcAttachPin(output25, 2);
}

void loop()
{
  // Serial.print(WiFi.macAddress());
  if (myData.xz > 2100) // 大于就是摇杆往前推了
  {
    l_front(myData.xz);
    r_front(myData.xz);

    // Serial.println("front"); 
  }
  if (myData.xz < 1700)
  {
    l_back(myData.xz);
    r_back(myData.xz);
    // Serial.println("back");
  }
  if (myData.yz > 2100)
  {
    l_front(myData.yz);
    r_back(myData.yz);
    // Serial.println("left");
  }
  if (myData.yz < 1700) // 小于就是摇杆转了，要往左走
  {
    r_front(myData.yz);
    l_back(myData.yz);
    // Serial.println("right");
  }
  if (myData.xz <= 2100 && myData.xz >= 1700 && myData.yz <= 2100 &&
      myData.yz >= 1700)
  {
    stop_lr();
    // Serial.println("stop");
  }
}

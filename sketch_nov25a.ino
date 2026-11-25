#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// =====================
// 腳位定義
// =====================
#define RELAY1_PIN 32   // 繼電器 1
#define RELAY2_PIN 33   // 繼電器 2（請依實際接線修改）

// =====================
// BLE UUID（與網頁相同）
// =====================
#define SERVICE_UUID  "12345678-1234-1234-1234-000000000001"
#define INFO_UUID     "12345678-1234-1234-1234-000000000010"
#define DATA_UUID     "12345678-1234-1234-1234-000000000020"
#define COMMAND_UUID  "12345678-1234-1234-1234-000000000030"

bool deviceConnected = false;

// =====================
// BLE 連線狀態回呼
// =====================
class ServerCallback : public BLEServerCallbacks {
    void onConnect(BLEServer* server) {
        deviceConnected = true;
        Serial.println("BLE Connected");
    }
    void onDisconnect(BLEServer* server) {
        deviceConnected = false;
        Serial.println("BLE Disconnected");
        server->getAdvertising()->start();
    }
};

// =====================
// 接收網頁指令
// =====================
class CommandCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *c) {
        String cmd = c->getValue().c_str();
        cmd.trim();
        Serial.println("Received: " + cmd);

        // 繼電器 1
        if (cmd == "RELAY1:ON") {
            digitalWrite(RELAY1_PIN, HIGH);
            Serial.println("Relay 1 ON");
        }
        else if (cmd == "RELAY1:OFF") {
            digitalWrite(RELAY1_PIN, LOW);
            Serial.println("Relay 1 OFF");
        }
        // 繼電器 2
        else if (cmd == "RELAY2:ON") {
            digitalWrite(RELAY2_PIN, HIGH);
            Serial.println("Relay 2 ON");
        }
        else if (cmd == "RELAY2:OFF") {
            digitalWrite(RELAY2_PIN, LOW);
            Serial.println("Relay 2 OFF");
        }
    }
};

void setup() {
    Serial.begin(115200);

    // 初始化繼電器腳位
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);
    digitalWrite(RELAY1_PIN, LOW);  // 預設關閉
    digitalWrite(RELAY2_PIN, LOW);

    // BLE 初始化
    BLEDevice::init("6792_Device");
    BLEDevice::setMTU(512);

    BLEServer *server = BLEDevice::createServer();
    server->setCallbacks(new ServerCallback());

    BLEService *service = server->createService(SERVICE_UUID);

    // INFO
    BLECharacteristic *info = service->createCharacteristic(
        INFO_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    info->setValue("{\"product\":\"Relay_Controller\",\"version\":\"1.0\"}");

    // DATA（保留，未來可擴充狀態回報）
    BLECharacteristic *dataChar = service->createCharacteristic(
        DATA_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    dataChar->addDescriptor(new BLE2902());
    dataChar->setValue("{}");

    // COMMAND
    BLECharacteristic *command = service->createCharacteristic(
        COMMAND_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    command->setCallbacks(new CommandCallback());

    service->start();

    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);
    advertising->start();

    Serial.println("BLE Relay Controller Started");
}

void loop() {
    // 目前不需要在 loop 做事情
    delay(100);
}
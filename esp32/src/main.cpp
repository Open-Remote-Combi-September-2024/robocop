
#include "secret.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>
#include <math.h>

#define SENSOR_PIN 34   // ESP32 pin GPIO34 connected to the OUT pin of the sound sensor
#define BUZZER_PIN 12
#define VREF 1.1        // ESP32 ADC reference value
#define COEFFICIENT 50

WiFiClient askClient;
PubSubClient client(askClient);

//MQTT reconnect
void check_openremote_connection() {
    int counter = 5;
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (!client.connect(ClientID, username, mqttpass, lastwill, 1, 1, lastwillmsg))
        {
            Serial.print("failed");
            Serial.println(" -> trying again in 5 seconds");
            
            counter--;
            if (counter < 0)
            { return; }

            delay(5000);
            continue;
        }
        Serial.println("-> MQTT client connected");
        client.subscribe(topic);
        Serial.print("Subscribed to: ");
        Serial.println(topic);
        counter--;
    }
}

//MQTT callback
void callback(char* topic, byte * payload, unsigned int length) {
    for (int i = 0; i < length; i++) {
        Serial.println(topic);
        Serial.print(" has send ");
        Serial.print((char)payload[i]);
    }

}

void check_wifi_connection()
{
    int counter = 5;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.printf(".");
      counter--;
      if (counter < 0)
      { return; }
    }
}

void connect_wifi(const char* ssid, const char* password) {
    WiFi.disconnect(true);  // Disconnect from the previous WiFi
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.mode(WIFI_STA); // Client mode
    
    WiFi.begin(ssid, password);

    // Try 5 times till connected
    check_wifi_connection();
}

int min(int arr[], int size) {
    if (size <= 0) {
        return -1;
    }

    int min = arr[0];
    
    for (int i = 1; i < size; i++) {
        if (arr[i] < min) {
            min = arr[i];
        }
    }

    return min;
}

int max(int arr[], int size) {
    if (size <= 0) {
        return -1;
    }

    int max = arr[0];

    for (int i = 1; i < size; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }

    return max;
}

void setup() {
    delay(5000);
    Serial.begin(115200); delay(10);

    pinMode(BUZZER_PIN, OUTPUT);


    //============ WiFi Setup ============//
    connect_wifi(ssid, password);

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.printf("Connection to %s failed\n", ssid);
        esp_restart();
    }
    Serial.printf("Connected to %s\nObtained IP: ");
    Serial.println(WiFi.localIP());
    //============ END ============//


    //============ MQTT Setup ============//
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    //============ END ============//

    //============ Sound Sensor Setup ============//
    pinMode(SENSOR_PIN, INPUT);
    //============ END ============//
}

#define SIZE_OF_SENSOR_VALUES 20
#define PIN_QUIET 3
#define PIN_MODERATE 4
#define PIN_LOUD 5
const int sampleWindow = 100;    // Sample window width in mS (100 mS = 40Hz)
int sensorValues[SIZE_OF_SENSOR_VALUES];
char sound_level[16];


float listen() {
    float voltageValue,dbValue;
    voltageValue = analogRead(SENSOR_PIN) / 1024.0 * VREF;
    dbValue = voltageValue * COEFFICIENT;      //convert voltage to dB
    return dbValue;
}

#include <sys/socket.h> // For socket functions
#include <arpa/inet.h>  // For sockaddr_in
#include <unistd.h>     // For close
void cue() {
    Serial.printf("hi");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
    }
    
    Serial.printf("hi");

    // Define server address
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(1337); // Port 1337
    //server.sin_addr.s_addr = inet_addr("192.168.1.14"); // Server IP
    server.sin_addr.s_addr = inet_addr("192.168.137.20"); // Server IP

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        close(sock);
    }

    // Send some data
    const char *message = "scream";
    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
    }
    
    // Close the socket
    close(sock);
}

void loop() {
    // check if we're connected for 5 times
    check_openremote_connection();

    if (client.state() != MQTT_CONNECTED)
    {
        Serial.println("OpenRemote MQTT connection failed");
        esp_restart();
    }

    Serial.println("Listening...");
    float db = listen();
    delay(125);

    Serial.print(db,1);
    Serial.println(" dBA");

    if (db > 50)
    { cue(); }

    sprintf(sound_level, "%e", db);
    if (client.publish(lastwill,  sound_level))
    { printf("Publish success :)\n"); }
    else
    { printf("Publish failed :(\n"); }
    

    printf("\n\n");

}

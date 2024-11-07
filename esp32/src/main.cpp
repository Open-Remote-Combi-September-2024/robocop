#include "secret.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>
#include <math.h>

#define SENSOR_PIN 34 // ESP32 pin GPIO34 connected to the OUT pin of the sound sensor

WiFiClient askClient;
PubSubClient client(askClient);

//MQTT reconnect
void check_openremote_connection() {
    int counter = 5;
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt connection
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

const int sampleWindow = 100;    // Sample window width in mS (100 mS = 40Hz)
int sensorValues[SIZE_OF_SENSOR_VALUES];
char sound_level[16];

double calculateRMS(const std::vector<uint16_t>& samples) {
    double sum = 0;

    for (uint16_t sample : samples) {
        sum += sample * sample; // Square the sample
    }

    double mean = sum / (double)samples.size(); // Calculate mean of squares
    
    return std::sqrt(mean);
}

double listen() {
    std::vector<uint16_t> samples;
    const int sampleWindow = 50;    // Sample window width in mS (100 mS = 40Hz) human ear 20 Hz - 20000 Hz
    uint16_t sample;

    int startMillis = millis();
    while (millis() - startMillis < sampleWindow) {
        sample = analogRead(SENSOR_PIN);
        samples.push_back(sample);
    }

    double rms = calculateRMS(samples);
    return 20*log10(rms);
}

#include <sys/socket.h> // For socket functions
#include <arpa/inet.h>  // For sockaddr_in
#include <unistd.h>     // For close
void alert_cue() {
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
    double db = listen();

    Serial.printf("Decibel: %f\n", db);

    if (db > 50)
    { alert_cue(); }


    sprintf(sound_level, "%e", db);
    if (client.publish(lastwill,  sound_level))
    { printf("Publish success :)\n"); }
    else
    { printf("Publish failed :(\n"); }
    

    printf("\n\n");

    delay(200);
}

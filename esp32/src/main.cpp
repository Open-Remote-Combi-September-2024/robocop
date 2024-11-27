
#include <WiFi.h>
#include "secret.h"
//#include "AudioTools.h"
#include "BluetoothA2DP.h"
//#include "AudioTools/AudioLibs/A2DPStream.h"
//#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include <PubSubClient.h>
#include <vector>
#include <math.h>
//#include "AudioTools/AudioLibs/AudioSourceSDFAT.h"

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

//const char* recordings[] = {"http://192.168.1.14:8000/shutup.mp3"};
//URLStream urlStream(ssid, password);
//AudioSourceURL source(urlStream, recordings, "audio/mp3");
//A2DPStream out;
//MP3DecoderHelix decoder;
//AudioPlayer player(source, out, decoder);
#include "audio_recordings.h"
BluetoothA2DPSource a2dp_source;
SoundData *shutup = new OneChannelSoundData((int16_t*)shutup_raw, shutup_raw_len/2);
void setup() {
    delay(2000);
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

    /*
    //============ MQTT Setup ============//
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    //============ END ============//
    */
    /*
    //============ Sound Sensor Setup ============//
    pinMode(SENSOR_PIN, INPUT);
    //============ END ============//
    */

    a2dp_source.set_local_name("esp32");
    a2dp_source.start("Jabra Speak 710");  

   /*
    player.setVolume(0.1);
    player.begin();

    auto cfg = out.defaultConfig(TX_MODE);
    cfg.silence_on_nodata = true;   // prevent disconnect when there is no audio data
    cfg.name = "Jabra Speak 710";   // set the device here. Otherwise the first available device is used for output
    //cfg.auto_reconnect = true;    // if this is use we just quickly connect to the last device ignoring cfg.name
    out.begin(cfg);
    */
}

float listen() {
    float voltageValue,dbValue;
    voltageValue = analogRead(SENSOR_PIN) / 1024.0 * VREF;
    dbValue = voltageValue * COEFFICIENT;      //convert voltage to dB
    return dbValue;
}

void loop() {
    // check if we're connected for 5 times
/*
    check_openremote_connection();

    if (client.state() != MQTT_CONNECTED)
    {
        Serial.println("OpenRemote MQTT connection failed");
        esp_restart();
    }
*/

    Serial.println("Listening...");
    float db = listen();
    delay(125);

    Serial.print(db,1);
    Serial.println(" dBA");

    /*
    sprintf(sound_level, "%e", db);
    if (client.publish(lastwill,  sound_level))
    { printf("Publish success :)\n"); }
    else
    { printf("Publish failed :(\n"); }

    printf("\n\n");
    */

    /*
    Serial.println("loop");
    player.copy();
    */
    a2dp_source.set_volume(40);
    a2dp_source.write_data(shutup);
    a2dp_source.delay_ms(2000);
    a2dp_source.set_volume(60);
    a2dp_source.write_data(shutup);
    a2dp_source.delay_ms(2000);
    a2dp_source.set_volume(80);
    a2dp_source.write_data(shutup);
    a2dp_source.delay_ms(2000);
}

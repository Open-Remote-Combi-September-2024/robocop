// Wifi 
const char* ssid = "YOURWIFISSID";                      // Wifi SSID
const char* password = "YOURWIFIPASSWORD";              // Wifi Password

//MQTT Broker
const char* mqtt_server = "192.168.1.1";                // the IP of the mqtt server
unsigned int mqtt_port = 1883;                          // the port of the mqtt server
const char* username = "master:mqttserviceuser";        // username of your service user (realm:username)
const char* mqttpass = "password";                      // Service User Secret
const char* ClientID = "CLIENTID";                    // arbitrary id

//LastWill
const char* lastwill = "master/CLIENTID/writeattributevalue/WRITEATTRIBUTENAME/YOURASSETID";
const char* lastwillmsg = "0";


//subscribing Topic
const char *topic = "master/CLIENTID/attributevalue/SUBSCRIBEATTRIBUTENAME/#";

//Local CA

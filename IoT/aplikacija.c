// Publisher
#include <stdio.h>
#include <mosquitto.h>
#include <string.h>

#define MQTT_PORT 1883

typedef enum {FALSE, TRUE} BOOL;

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	if (rc == 0) {
		printf("Subcribing to topic -t broker \n ");
		mosquitto_subscribe(mosq, NULL, "broker", 0);
	} else {
		mosquitto_disconnect(mosq);
	}
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	// Print the topic for the first message received, then disconnect
	printf("Topic: %s\n", msg->topic);
	printf("Message: %s\n", msg->payload);
	//mosquitto_disconnect(mosq);
}

int main() {
	int rc;
	struct mosquitto * mosq;

	mosquitto_lib_init();

	mosq = mosquitto_new("publisher-test", true, NULL);
	
	int tag = 1;

	rc = mosquitto_connect(mosq, "127.0.0.1", MQTT_PORT, 60);
	if (rc != 0) {
		printf("Client could not connect to broker! Error Code: %d\n", rc);
		mosquitto_destroy(mosq);
		return -1;
	}

	printf("We are now connected to the broker!\n");

	BOOL stanje = TRUE;
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);
	//mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_loop_start(mosq);
	while(1){
		
		char reg[6];
		printf("Enter vehicle registration (6 charachters):\n");
		scanf ("%s", reg);
				
		printf("The tag of your vehicle is: %d\n", tag);
		char msg[10];
		sprintf(msg, "%s %d", reg, tag);
		++tag;
				
		mosquitto_publish(mosq, NULL, "app", strlen(msg), msg, 0, false);
		}
	
	//mosquitto_loop_stop(mosq, true);

	//mosquitto_loop_forever(mosq, -1, 1);

	//mosquitto_disconnect(mosq);
	//mosquitto_destroy(mosq);
	//mosquitto_lib_cleanup();

	return 0;
}

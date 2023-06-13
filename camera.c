//===================================================== file = mclient.c =====
//=  A multicast client to receive multicast datagrams                       =
//============================================================================
//=  Notes:                                                                  =
//=    1) This program receives on a multicast address and outputs the       =
//=       received buffer to the screen.                                     =
//=    2) Conditionally compiles for Winsock and BSD sockets by setting the  =
//=       initial #define to WIN or BSD as appropriate.                      =
//=    3) The multicast group address is GROUP_ADDR.                         =
//=--------------------------------------------------------------------------=
//=  Build: bcc32 mclient.c or cl mclient.c wsock32.lib for Winsock          =
//=         gcc mclient.c -lsocket -lnsl for BSD                             =
//=--------------------------------------------------------------------------=
//=  History:  JNS (07/11/02) - Genesis                                      =
//============================================================================

//----- Include files -------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <stdlib.h>         // Needed for memcpy()
#include <sys/types.h>    // Needed for system defined identifiers.
#include <netinet/in.h>   // Needed for internet address structure.
#include <sys/socket.h>   // Needed for socket(), bind(), etc...
#include <arpa/inet.h>    // Needed for inet_ntoa()
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <mosquitto.h>

//----- Defines -------------------------------------------------------------
#define PORT_NUM         27002             // Port number used
#define GROUP_ADDR "231.255.255.240"       // Address of the multicast group
#define HEADER_RESPONSE "200 OK"

#define DEVICE_ID	1235
#define DEVICE_TYPE 1

#define BUFFER_LEN 1024

//----- Datatypes -----------------------------------------------------------
typedef struct {
	int sock;
	struct sockaddr_in addr;
} descriptor;
struct parametri{
	unsigned int mss;
	struct sockaddr_in ad;
	unsigned int adlen;
};
typedef enum {FALSE, TRUE}BOOL;
BOOL stanje;
int timer1;
int temp;
int provera;

void on_connect(struct mosquitto *mosq, void *obj, int rc){}


void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){}


void *receiveMsg(void* params)
{
	struct parametri *param = (struct parametri*) params;
	int retcode;
	int error = 0;
	int counter = 0;
	int brojac = 10;
	while(1){
		unsigned char buffer[BUFFER_LEN];
		unsigned int multi_server_sock = param->mss;
		struct sockaddr_in client_addr = param->ad;
		unsigned int addr_len = param->adlen;
		if((retcode = recvfrom(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len)) < 0){
			printf("Error");
		}
		//printf("%s\n", buffer);
		char* buff_temp = strdup(buffer);
		char* token = strtok(buff_temp, "\n");
		//printf("%s\n", strtok(token, " "));
		
		
		if(strcmp(strtok(token, " "), "M-SEARCH") == 0 && counter == 0){
			printf("SEARCH PRIMLJENN\n");
			snprintf(buffer, sizeof(buffer),
		            "HTTP/1.1 %s\n"
		            "HOST: %s:%d\r\n"
		            "USN: %d\n"
	            	"ST: broker\r\n"
		            "CONFIGID.UPNP.ORG: %d",
		            HEADER_RESPONSE,
		            GROUP_ADDR, PORT_NUM,
		            DEVICE_TYPE,
		            DEVICE_ID
	        	);
	        

			//snprintf(buffer, sizeof(buffer), "200 OK-%d-%d", DEVICE_ID, DEVICE_TYPE);
			if(counter == 0){
				error = sendto(multi_server_sock, buffer, sizeof(buffer), MSG_CONFIRM, (struct sockaddr *)&client_addr, sizeof(client_addr));
				//printf("QWE");
				printf("%s\n", buffer);
				counter = 1;
			}
		}
		else if(strcmp(buffer, "ALIVE?") == 0){
				snprintf(buffer, sizeof(buffer), "200 OK-%d-%d", DEVICE_ID, DEVICE_TYPE);
				error = sendto(multi_server_sock, buffer, sizeof(buffer), MSG_CONFIRM, (struct sockaddr *)&client_addr, sizeof(client_addr));
				//printf("RTY");
				printf("%d\n", error);
		}
		else if (strcmp(buffer, "NOTIFY OK") == 0){
				printf("Client recieved NOTIFY OK message\n");
				//counter = 2;
		}
		else if((strcmp(buffer, "DEVICE ADDED") == 0) || counter == 1){
			//printf("RTY\n");
			if(brojac != 0){
				//sprintf(buffer, "NOTIFY ALIVE");
				snprintf(buffer, sizeof(buffer),
		            "HTTP/1.1 %s\n"
		            "HOST: %s:%d\r\n"
		            "USN: %d\n"
	            	"NT: broker\r\n"
	            	"NTS: ssdp:alive\n"
		            "CONFIGID.UPNP.ORG: %d",
		            HEADER_RESPONSE,
		            GROUP_ADDR, PORT_NUM,
		            DEVICE_TYPE,
		            DEVICE_ID
	        	);
	        	
				//printf("%s\n", buffer);
				sendto(multi_server_sock, buffer, sizeof(buffer), MSG_CONFIRM, (struct sockaddr *)&client_addr, sizeof(client_addr));
				brojac--;
			}
			if(brojac == 0){
				//sprintf(buffer, "NOTIFY BYE BYE");
				snprintf(buffer, sizeof(buffer),
		            "HTTP/1.1 %s\n"
		            "HOST: %s:%d\r\n"
		            "USN: %d\n"
	            	"NT: broker\r\n"
	            	"NTS: ssdp:byebye\n"
		            "CONFIGID.UPNP.ORG: %d",
		            HEADER_RESPONSE,
		            GROUP_ADDR, PORT_NUM,
		            DEVICE_TYPE,
		            DEVICE_ID
	        	);
	        	
				//printf("%s\n", buffer);
				sendto(multi_server_sock, buffer, sizeof(buffer), MSG_CONFIRM, (struct sockaddr *)&client_addr, sizeof(client_addr));
			}
			sleep(10);
		}
		
		free(buff_temp);

	}
}

//===== Main program ========================================================
void main(void)
{

  unsigned int         multi_server_sock; // Multicast socket descriptor
  struct ip_mreq       mreq;              // Multicast group structure
  struct sockaddr_in   client_addr;       // Client Internet address
  unsigned int         addr_len;          // Internet address length
  unsigned char        buffer[BUFFER_LEN];// Buffer that is used to send data
  int                  retcode;           // Return code



  // Create a multicast socket and fill-in multicast address information
  multi_server_sock=socket(AF_INET, SOCK_DGRAM,0);
  mreq.imr_multiaddr.s_addr = inet_addr(GROUP_ADDR);
  mreq.imr_interface.s_addr = INADDR_ANY;

  // Create client address information and bind the multicast socket
  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = INADDR_ANY;
  client_addr.sin_port = htons(PORT_NUM);
  
	stanje = TRUE;
	timer1 = 25;
  	BOOL bOptVal = TRUE;
  	int bOptLen = sizeof(BOOL);
  	
	if(setsockopt(multi_server_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&bOptVal, bOptLen)){
		printf("Using SO_REUSEADDR OK");
  	}

	retcode = bind(multi_server_sock,(struct sockaddr *)&client_addr,
		             sizeof(struct sockaddr));
	  if (retcode < 0)
	  {
		printf("*** ERROR - bind() failed with retcode = %d \n", retcode);
		return;
	  }

	  // Have the multicast socket join the multicast group
	  retcode = setsockopt(multi_server_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		         (char*)&mreq, sizeof(mreq)) ;
	  if (retcode < 0)
	  {
		printf("*** ERROR - setsockopt() failed with retcode = %d \n", retcode);
		return;
	  }

    // Set addr_len
    addr_len = sizeof(client_addr);


    pthread_t t1;
    struct parametri p1;
	p1.mss = multi_server_sock;
	p1.ad = client_addr;
	p1.adlen = addr_len;
	pthread_create(&t1, NULL, receiveMsg, &p1);
    int error = 0, cnt = 1;
	struct mosquitto *mosq;
	int rc;
	
	mosquitto_lib_init();
	char registracija[6];
	
	
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL) {
		printf("Failed to create client instance.\n");
		return;
	}
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

	rc = mosquitto_connect(mosq, "127.0.0.1", 1883, 60);
	if (rc != MOSQ_ERR_SUCCESS) {
		printf("Connect failed: %s\n", mosquitto_strerror(rc));
		return;
	}
	mosquitto_loop_start(mosq);
	
	while(1){
	/*
		if(timer1 > 0){
			timer1 -= 1;
			
			snprintf(buffer, sizeof(buffer), "S: Senzor poruka No. %d\n", cnt);
			//snprintf(buffer, sizeof(buffer), "A: Aktuator poruka No. %d\n", cnt);
			
			printf("%s\n", buffer);
			++cnt;
			
			sendto(multi_server_sock, buffer, sizeof(buffer), MSG_CONFIRM, (struct sockaddr *)&client_addr, sizeof(client_addr));
			
			sleep(1);
			printf("%d\n", timer1);
			
		}	
		if(timer1 == 0)
			stanje = FALSE;
		if(stanje == FALSE){
			stanje = TRUE;
			//snprintf(buffer, sizeof(buffer), "NOTIFY BYE BYE-%d-%d", DEVICE_ID, DEVICE_TYPE);
			//snprintf(buffer, sizeof(buffer), "NOTIFY BYE BYE");
			
			snprintf(buffer, sizeof(buffer),
		            "HTTP/1.1 %s\n"
		            "HOST: %s:%d\r\n"
		            "USN: %d\n"
	            	"NT: broker\r\n"
	            	"NTS: ssdp:byebye\n"
		            "CONFIGID.UPNP.ORG: %d",
		            HEADER_RESPONSE,
		            GROUP_ADDR, PORT_NUM,
		            DEVICE_TYPE,
		            DEVICE_ID
	        );
			
			printf("%s\n", buffer);
			error = sendto(multi_server_sock, buffer, sizeof(buffer), MSG_CONFIRM, (struct sockaddr *)&client_addr, sizeof(client_addr));
			printf("%d", error);
			break;
		}
	*/
	
	provera = rand() % 101;
	if (provera % 2 == 0){
		mosquitto_publish(mosq, NULL, "garaza/senzor/prisustvo", sizeof("y"), "y", 0, false);
		printf("Enter registration: ");
		scanf("%s", registracija);
		mosquitto_publish(mosq, NULL, "garaza/senzor/tablice", sizeof(registracija), registracija, 0, false);
		
		sleep(30);
	}
	else{
		mosquitto_publish(mosq, NULL, "garaza/senzor/prisustvo", sizeof("n"), "n", 0, false);
		sleep(2);
	}
	
  }

  pthread_join(t1, NULL);

  // Close and clean-up
  close(multi_server_sock);
}

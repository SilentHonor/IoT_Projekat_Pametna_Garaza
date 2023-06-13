//===================================================== file = mserver.c =====
//=  A multicast server to send multicast datagrams                          =
//============================================================================
//=  Notes:                                                                  =
//=    1) This program sends datagrams one per second to a multicast group.  =
//=    2) Conditionally compiles for Winsock and BSD sockets by setting the  =
//=       initial #define to WIN or BSD as appropriate.                      =
//=    3) The multicast group address is GROUP_ADDR.                         =
//=--------------------------------------------------------------------------=
//=  Build: bcc32 mserver.c or cl mserver.c wsock32.lib for Winsock          =
//=         gcc mserver.c -lsocket -lnsl for BSD                             =
//=--------------------------------------------------------------------------=
//=  History:  JNS (07/11/02) - Genesis                                      =
//============================================================================
#define  BSD                // WIN for Winsock and BSD for BSD sockets

//----- Include files -------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <stdlib.h>         // Needed for memcpy() and itoa()
#ifdef WIN
  #include <winsock.h>      // Needed for all Windows stuff
#endif
#ifdef BSD
  #include <sys/types.h>    // Needed for system defined identifiers.
  #include <netinet/in.h>   // Needed for internet address structure.
  #include <sys/socket.h>   // Needed for socket(), bind(), etc...
  #include <arpa/inet.h>    // Needed for inet_ntoa()
  #include <fcntl.h>
  #include <unistd.h>
  #include <netdb.h>
  #include <string.h>
  #include <pthread.h>
  #include <stdbool.h>
  #include <mosquitto.h>
#endif

//----- Defines -------------------------------------------------------------
#define PORT_NUM         27002            // Port number used
#define GROUP_ADDR "231.255.255.240"      // Address of the multicast group
#define BROKER_ID	11
#define DEVICE_TYPE 3 //3 - broker


#define BUFFER_LEN 1024
#define BR_KOLA 3

struct parametri{
	unsigned int mss;
	struct sockaddr_in ad;
	unsigned int adlen;
};

struct automobil{
	char* registracija;
	char* tag;
};



struct device_desc{
   char id[10];
   char type[50];
};


struct automobil garaza[BR_KOLA];
struct device_desc uredjaji[8];
int br_uredjaja = 0;
int br_kola = 0;

bool ima_li_ida(char id[]){
	for(int i=0; i<5; i++){
		if(!strcmp(uredjaji[i].id, id)){
			return true;
		}
	}
	return false;
}



void split_and_save(char buff[]){
	/*
	snprintf(buffer, sizeof(buffer),
		            "HTTP/1.1 %s\n"
		            "HOST: %s:%d\r\n"
		            "USN: %d\n"
	            	"ST: broker\r\n"
		            "CONFIGID.UPNP.ORG: %d",
		            HEADER_RESPONSE,
		            GROUP_ADDR, PORT_NUM,
		            DEVICE_TYPE, -> **** 2.
		            DEVICE_ID -> **** 1.
	        	);
	*/
	
	int i=0;
	char* tmp_buff_1 = strdup(buff);
	char* token = strtok(tmp_buff_1, "\n"); 	//HTTP/1.1
	token = strtok(NULL, "\n");					//HOST
	token = strtok(NULL, "\n");					//USN -> DEVICE_TYPE
	
	//printf("SPLIT TOKEN: %s\n", token);
	
	char* tmp_type = strdup(token);
	char* devtype = strtok(tmp_type, " ");
	devtype = strtok(NULL, " ");
	
	char* tmp_buff_2 = strdup(buff);
	char* token_2 = strtok(tmp_buff_2, "\n");	//HTTP/1.1
	
	token_2 = strtok(NULL, "\n");			//HOST
	token_2 = strtok(NULL, "\n");			//USN
	token_2 = strtok(NULL, "\n");			//ST
	token_2 = strtok(NULL, "\n");			//CONFIGID -> DEVICE_ID
	
	//printf("SPLIT TOKEN 2: %s\n", token);
	
	char* tmp_id = strdup(token);
	char* devid = strtok(tmp_id, " ");
	devid = strtok(NULL, " ");
	/*
	while(token != NULL){
		if(i==1){
			if(!ima_li_ida(token)){
				strcpy(uredjaji[br_uredjaja].id, token);
			}else{
				break;
			}
		}else if(i==2){
			strcpy(uredjaji[br_uredjaja].type, token);
			br_uredjaja++;
		}
		i++;
		token = strtok(NULL, "-");
	}
	*/
	if(!ima_li_ida(devid)){
		strcpy(uredjaji[br_uredjaja].id, devid);
		strcpy(uredjaji[br_uredjaja].type, devtype);
		br_uredjaja++;
	}
	
	free(tmp_buff_1);
	free(tmp_buff_2);
	free(tmp_type);
	free(tmp_id);
}




/* -> param su socket i adresa
				-> samo socket pa napraviti addr_dest ILI
				   napraviti typedef strukturu koja sadrzi oba podatka
				   */
void *receiveMsg(void* params)
{
	struct parametri *param = (struct parametri*) params;
	int retcode;
	while(1){
		unsigned char buffer[BUFFER_LEN];
		
		unsigned int multi_server_sock = param->mss;
		struct sockaddr_in addr_dest = param->ad;
		unsigned int addr_len = param->adlen;
		if((retcode = recvfrom(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr_dest, &addr_len)) < 0){
			printf("Error");
		}
		//printf("BUFFER:\n%s\n\n", buffer);
		unsigned char* temp_buff = strdup(buffer);

		
		char* token = strtok(temp_buff, "\n");
		//printf("TOKEN: %s\n", token);
		//printf("BUFFER:\n%s\n\n", buffer);
		
		char* temp_tok = strtok(token, " ");
		
		temp_tok = strtok(NULL, " ");
		char* tok_200 = temp_tok; // 200
		
		temp_tok = strtok(NULL, " ");
		char* tok_ok = temp_tok; // OK
		
		//printf("%s %s\n", tok_200, tok_ok);
		
		/*
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
		            DEVICE_ID -> NEEDED HERE
	        	); 
	    */
	    unsigned char* temp_buff_2 = strdup(buffer);
	    char* token2 = strtok(temp_buff_2, "\n");
	    
		//for ssdp:alive or ssdp:goodbye
		token2 = strtok(NULL, "\n");	//HOST
		token2 = strtok(NULL, "\n");	//USN
		token2 = strtok(NULL, "\n");	//NT
		token2 = strtok(NULL, "\n");	//NTS ->ssdp msg
		//printf("TOKEN 2: %s\n", token2);
		
		char* ssdp_tok = strtok(token2, " ");
		ssdp_tok = strtok(NULL, " ");
		//printf("SSDP TOKEN: %s\n", ssdp_tok);
		
		unsigned char* msg_buff = strdup(buffer);
		char* tok_msg = strtok(msg_buff, ":");
		
		printf("%s %s\n", tok_200, tok_ok);
		if(strcmp(tok_200, "200") == 0 && strcmp(tok_ok, "OK") == 0){ //200 OK
			//printf("OVDE SAM\n");
			//printf("%s\n", buffer);
			split_and_save(buffer);
			//printf("BUFFER NAKON FJE:\n%s", buffer);
			printf("\nbroj uredjaja %d\n", br_uredjaja);
			sprintf(buffer, "DEVICE ADDED");
			//printf("%s\n", buffer);
			sendto(multi_server_sock, buffer, sizeof(buffer), 0,
			(struct sockaddr*)&addr_dest, addr_len);
			//printf("sveok\n");

		}
		else if(strcmp(ssdp_tok, "ssdp:alive") == 0){ //NOTIFY ALIVE
			sprintf(buffer, "NOTIFY OK");
			//printf("%s\n", buffer);
			sendto(multi_server_sock, buffer, sizeof(buffer), 0,
			(struct sockaddr*)&addr_dest, addr_len);
		}
		else if(strcmp(ssdp_tok, "ssdp:byebye") == 0){ //NOTIFY BYE BYE
			printf("Evo me\n");
			char* temp_buff_byebye = strdup(buffer);
			/*
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
		            DEVICE_ID -> NEEDED HERE
	        	); 
	        */
	        
			char* tok = strtok(temp_buff_byebye, "\n");	//HTTP
			token = strtok(NULL, "\n");					//HOST
			token = strtok(NULL, "\n");					//USN
			token = strtok(NULL, "\n");					//NT
			token = strtok(NULL, "\n");					//NTS
			token = strtok(NULL, "\n");					//CONFIGID ->this is the one that holds the id
			
			char* id_tok = strtok(token, " ");
			id_tok = strtok(NULL, " ");
			
			for(int i=0; i<8; i++){
				if(!strcmp(uredjaji[i].id, id_tok)){
					strcpy(uredjaji[i].id, "");
					strcpy(uredjaji[i].type, "");
					br_uredjaja--;
				}
			}
			free(temp_buff_byebye);
		}
		else if (strcmp(tok_msg, "S") == 0 || strcmp(tok_msg, "A") == 0){
			//printf("PORUKA OD EKSTERNOG UREDJAJA: %s", buffer);
		}
		

		
		free(temp_buff);
		free(temp_buff_2);
		free(msg_buff);

	}
}

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	if (rc == 0) {
		printf("Subcribing to topic -t app \n ");
		mosquitto_subscribe(mosq, NULL, "app", 0);
		
		printf("Subcribing to topic -t garaza/senzor/tag \n ");
		mosquitto_subscribe(mosq, NULL, "garaza/senzor/tag", 0);
		
		printf("Subcribing to topic -t garaza/senzor/prisustvo \n ");
		mosquitto_subscribe(mosq, NULL, "garaza/senzor/prisustvo", 0);
		
		printf("Subcribing to topic -t garaza/senzor/tablice \n ");
		mosquitto_subscribe(mosq, NULL, "garaza/senzor/tablice", 0);
		
		printf("Subcribing to topic -t garaza/aktuator/tag \n ");
		mosquitto_subscribe(mosq, NULL, "garaza/aktuator/tag", 0);
		
		printf("Subcribing to topic -t garaza/aktuator/prisustvo \n ");
		mosquitto_subscribe(mosq, NULL, "garaza/aktuator/prisustvo", 0);
		
		printf("Subcribing to topic -t garaza/aktuator/tablice \n ");
		mosquitto_subscribe(mosq, NULL, "garaza/aktuator/tablice", 0);
		
	} else {
		mosquitto_disconnect(mosq);
	}
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	// Print the topic for the first message received, then disconnect
	printf("%s, %s\n", uredjaji[0].id, uredjaji[0].type);
	printf("Topic: %s\n", msg->topic);
	printf("Message: %s\n", msg->payload);
	char brKola[2];
	
	//podaci se cuvaju na brokeru
	if (strcmp(msg->topic, "app") == 0){
		printf("PORUKA PRIMLJENA OD APLIKACIJE!\n");
		char* tmp = strdup(msg->payload);
		char* tok = strtok(tmp, " ");
		
		if (br_kola < BR_KOLA){
			//mosquitto_publish(mosq, NULL, "garaza/aktuator/tablice", sizeof(tok), tok, 0, false);
			garaza[br_kola].registracija = strdup(tok);
			
			tok = strtok(NULL, " ");
			garaza[br_kola].tag = strdup(tok);
			++br_kola;
			sprintf(brKola, "%d", BR_KOLA - br_kola);
			mosquitto_publish(mosq, NULL, "garaza/aktuator/prisustvo", sizeof(brKola), brKola, 0, false);
		}
		else{
			printf("Capacity full!");
		}
		int i;
		for (i = 0; i < br_kola; ++i){
			printf("AUTO %d: registracija: %s, tag: %s\n", i+1, garaza[i].registracija, garaza[i].tag);
		}
		
		
	}
	else if (strcmp(msg->topic, "garaza/senzor/tablice") == 0){
		char* reg = strdup(msg->payload);
		bool reserved = false;
		int i;
		for (i = 0; i < br_kola; ++i){
			if (strcmp(garaza[i].registracija, reg) == 0){
				reserved = true;
				break;
			}
		}
		if (reserved == true) mosquitto_publish(mosq, NULL, "garaza/aktuator/tablice", sizeof("y"), "y", 0, false);
		else mosquitto_publish(mosq, NULL, "garaza/aktuator/tablice", sizeof("n"), "n", 0, false);
	}
	else if (strcmp(msg->topic, "garaza/senzor/tag") == 0){
		char* tag = strdup(msg->payload);
		bool reserved = false;
		int i;
		for (i = 0; i < br_kola; ++i){
			if (strcmp(garaza[i].tag, tag) == 0){
				reserved = true;
				break;
			}
		}
		if (reserved == true) mosquitto_publish(mosq, NULL, "garaza/aktuator/tag", sizeof("y"), "y", 0, false);
		else mosquitto_publish(mosq, NULL, "garaza/aktuator/tag", sizeof("n"), "n", 0, false);
	}
	//else printf("TOPIC: %s\n MSG: %s", msg->topic, msg->payload);
	
	//mosquitto_disconnect(mosq);
}


//===== Main program ========================================================
void main(void)
{
#ifdef WIN
  WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
  WSADATA wsaData;                              // Stuff for WSA functions
#endif
  unsigned int         server_s;                // Server socket descriptor
  unsigned int         multi_server_sock;       // Multicast socket descriptor
  struct sockaddr_in   addr_dest;               // Multicast group address
  struct ip_mreq       mreq;                    // Multicast group descriptor
  unsigned char        TTL;                     // TTL for multicast packets
  struct in_addr       recv_ip_addr;            // Receive IP address
  unsigned int         addr_len;                // Internet address length
  unsigned char        buffer[BUFFER_LEN];      // Buffer that is used to send data
  int                  count;                   // Loop counter
  int                  retcode;                 // Return code

#ifdef WIN
  // This stuff initializes winsock
  WSAStartup(wVersionRequested, &wsaData);
#endif

  // Create a multicast socket
  multi_server_sock=socket(AF_INET, SOCK_DGRAM,0);

  // Create multicast group address information
  addr_dest.sin_family = AF_INET;
  addr_dest.sin_addr.s_addr = inet_addr(GROUP_ADDR);
  addr_dest.sin_port = htons(PORT_NUM);

  // Set the TTL for the sends using a setsockopt()
  TTL = 1;
  retcode = setsockopt(multi_server_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&TTL, sizeof(TTL));
  if (retcode < 0)
  {
    printf("*** ERROR - setsockopt() failed with retcode = %d \n", retcode);
    return;
  }

  // Set addr_len
  addr_len = sizeof(addr_dest);
  
  

  // Multicast the message forever with a period of 1 second
  count = 0;
  printf("*** Sending multicast datagrams to '%s' (port = %d) \n", GROUP_ADDR, PORT_NUM);
  
  	struct mosquitto *mosq;
	int rc;

	mosquitto_lib_init();

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

    pthread_t t1;
	struct parametri p1;
	p1.mss = multi_server_sock;
	p1.ad = addr_dest;
	p1.adlen = addr_len;
	pthread_create(&t1, NULL, receiveMsg, &p1);
	
	for (int i = 0; i < 10; ++i){
	
		// Build the message in the buffer
		
			snprintf(buffer, sizeof(buffer),
		            "M-SEARCH * HTTP/1.1\n"
		            "HOST: %s:%d\r\n"
		            "MAN: ssdp:discover\n"
		            "USN: %d\n"
	            	"ST: broker\r\n"
		            "CPUUID.UPNP.ORG: %d",
		            GROUP_ADDR, PORT_NUM,
		            DEVICE_TYPE,
		            BROKER_ID
	        	);
	        
		printf("%s\n", buffer);
		
		// Send buffer as a datagram to the multicast group
		sendto(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr_dest, addr_len);
		sleep(1);
	}

  while(1){
    //recvfrom(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr_dest, &addr_len);
    //printf("%s\n", buffer);
    
    mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);

	mosquitto_lib_cleanup();
  }

  pthread_join(t1, NULL);

  // Close and clean-up
#ifdef WIN
  closesocket(multi_server_sock);
  WSACleanup();
#endif
#ifdef BSD
  close(multi_server_sock);
#endif
}

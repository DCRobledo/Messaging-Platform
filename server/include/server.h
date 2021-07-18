#ifndef HEADER_SERVIDOR
#define HEADER_SERVIDOR

#include "linked_list.h"

struct client_id {
	char port[6];
	char ip[15];
	int sd;
};

int get_client_host(char *ip, int16_t port);
int search_client(char *user_name, struct client **client_str);
int connect_client(char *user_name, struct client *client);
int recieve_pending_messages(struct client *receiver);
int send_message(char *user_name, struct message *message);
void prepare_structure(struct client *client, struct client_id client_id);

int register_IMPLEMENTATION (char *user_name, struct client_id client_id);
int unregister_IMPLEMENTATION (char *user_name);
int connect_IMPLEMENTATION (char *user_name, struct client_id client_id);
int disconnect_IMPLEMENTATION (char *user_name);
int send_IMPLEMENTATION (char *sender, char *reciever, char* message, unsigned int *message_id);

int process_request(char *operation_code, struct client_id client_id, unsigned int *message_id);
void end_service(int local_sc);
int service(void *sc);

#endif

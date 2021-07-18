#ifndef LINKED_LIST_HEADER
#define LINKED_LIST_HEADER

#include <pthread.h>
#include "../../RPC/register_archive.h"

typedef enum state{
    CONNECTED,
    DISCONNECTED
} state;

typedef struct node {
    void *element;
    struct node *next; // Next node in the list
} node;

typedef struct linked_list {
    struct node *head; // First node of the list
    char is_created;
	pthread_mutex_t mutex_list;
} linked_list;

typedef struct client {
    char user_name[255];

    state state;
    char ip[16];
    char port[6];

    linked_list messages;
    unsigned int last_message_id;
} client;

typedef struct message {
    int id;
    char message[256];
	char sender[256];
} message;


int init(struct linked_list *list);

int add_client(struct linked_list *list, client* client);
int add_message(struct linked_list *list, message *message);

int get_client(struct linked_list *list, char* user_name, client **client);
int get_client_rpc(struct linked_list *list, char* user_name, client_rpc **client);
int get_message(struct linked_list *list, int message_id, message **message);

int modify_client(struct linked_list *list, char* user_name, client* new_client);
int modify_message(struct linked_list *list, int message_id, message *new_message);

int delete_client(struct linked_list *list, char *user_name);
int delete_message(struct linked_list *list, int message_id);

int find_client(struct linked_list *list, char *user_name);
int find_message(struct linked_list *list, int message_id);

int pop_message(struct linked_list *list, struct message **msg);

int length(struct linked_list *list);

void print_list(struct linked_list *list);

#endif

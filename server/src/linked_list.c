#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "../include/linked_list.h"


int init(struct linked_list *list) {
    if(list->is_created != 1)
        pthread_mutex_init (&list->mutex_list, NULL);
    

    pthread_mutex_lock(&list->mutex_list);
    if(list->head !=NULL) {
        struct node *current =list->head;
        struct node *next = NULL;

        // We remove all previous nodes
        while(current !=NULL){
            next =current->next;
            free(current);
            current = next ;
        } 

        // And create a new list
        list->head =NULL;
        
    }

    list->is_created = 1;

    pthread_mutex_unlock(&list->mutex_list);
    return 0;
}


int add_client(struct linked_list *list, client* client) {
    pthread_mutex_lock(&list->mutex_list);

    int res = 0;

    if(list == NULL) {
        perror("Error, the list doesn't exist\n");
        res = -1;
    }

    else {
        struct node *new_node = (struct node*) malloc(sizeof(struct node));
        new_node->element = client;
        new_node->next = NULL;

        if(list->head == NULL)
            list->head = new_node;

        else {
            struct node *current_node = list->head; 

            while(current_node->next != NULL) 
                current_node = current_node->next;

            current_node->next = new_node;
        }
    }

    pthread_mutex_unlock(&list->mutex_list);
    
    return res;
}

int add_message(struct linked_list *list, message *message) {
    pthread_mutex_lock(&list->mutex_list);

    int res = 0;

	if (list == NULL) {
        perror("Error, the list doesn't exist\n");
        res = -1;
    }

	else {
        node *new_node = (struct node*) malloc(sizeof(struct node));
        new_node->element = message;
        new_node->next = NULL;

        if(list->head == NULL) 
            list->head = new_node;
            
        else {
            struct node *current_node = list->head;

            while(current_node->next != NULL) 
                current_node = current_node->next;

            current_node->next = new_node;
        }
        
	}

    pthread_mutex_unlock(&list->mutex_list);

    return res;
}


int get_client(struct linked_list *list, char* user_name, client **client) {
    pthread_mutex_lock(&list->mutex_list);

    int res = 0;

    if(list == NULL) res = -1;

    else if(list->head == NULL) res = -1;

    else {
        struct node *current = list->head;
        bool client_found = false;

        while(current != NULL && !client_found) {
            struct client *current_client = current->element;
            
            if(strcmp(current_client->user_name, user_name) == 0) {
                client_found = true;
				*client = current_client;
                res = 0;
            }

            current = current->next;
        }

        if(!client_found) {
            res = -2;
        }
    }

    pthread_mutex_unlock(&list->mutex_list);

    return res;
}

int get_client_rpc(struct linked_list *list, char* user_name, client_rpc **client) {
    pthread_mutex_lock(&list->mutex_list);

    int res = 0;

    if(list == NULL) res = -1;

    else if(list->head == NULL) res = -1;

    else {
        struct node *current = list->head;
        bool client_found = false;

        while(current != NULL && !client_found) {
            struct client_rpc *current_client = current->element;
            
            if(strcmp(current_client->user_name, user_name) == 0) {
                client_found = true;
				*client = current_client;
                res = 0;
            }

            current = current->next;
        }

        if(!client_found) {
            res = -2;
        }
    }

    pthread_mutex_unlock(&list->mutex_list);

    return res;
}

int get_message(struct linked_list *list, int message_id, message **message) {
    pthread_mutex_lock(&list->mutex_list);

    int res = -1;

    if(list == NULL) perror("Error, the list doesn't exist\n");

    else if(list->head == NULL) res = -2;

    else {
        struct node *current = list->head;
        bool message_found = false;

        while(current != NULL && !message_found) {
            struct message *current_message = current->element;
            
            if(message_id == current_message->id) {
                message_found = true;
				*message = current_message;
                res = 0;
            }

            current = current->next;
        }

        if(!message_found) {
            perror("Error: client not found on get_client\n");
            res = -3;
        }
    }

    pthread_mutex_unlock(&list->mutex_list);

    return res;
}


int modify_client(struct linked_list *list, char* user_name, client* new_client) {
    pthread_mutex_lock(&list->mutex_list);

    int res = -1;

    if(list == NULL) perror("Error, the list doesn't exist\n");

    else if(list->head == NULL) perror("Error, the list is empty\n");

    else {
        struct node *current = list->head;
        bool client_found = false;

        while(current->next != NULL && !client_found) {
            struct client *current_client = current->element;
            
            if(strcmp(current_client->user_name, user_name) == 0) {
                client_found = true;

				free(current->element);
                current->element = new_client;

                res = 0;
            }

            current = current->next;
        }

        if(!client_found) {
            perror("Error: client not found\n");
            res = -1;
        }
    }

    pthread_mutex_unlock(&list->mutex_list);

    return res;
}

int modify_message(struct linked_list *list, int message_id, message *new_message) {
    pthread_mutex_lock(&list->mutex_list);

    int res = -1;

    if(list == NULL) perror("Error, the list doesn't exist\n");

    else if(list->head == NULL) perror("Error, the list is empty\n");

    else {
        struct node *current = list->head;
        bool message_found = false;

        while(current->next != NULL && !message_found) {
            struct message *current_message = current->element;
            
            if(message_id == current_message->id) {
                message_found = true;

				free(current->element);
                current->element = new_message;

                res = 0;
            }

            current = current->next;
        }

        if(!message_found) {
            perror("Error: client not found\n");
            res = -1;
        }
    }

    pthread_mutex_unlock(&list->mutex_list);

    return res;
}

int delete_client(struct linked_list *list, char *user_name) {
    pthread_mutex_lock(&list->mutex_list);
	bool client_found = false;

	if (list != NULL  && list->head != NULL) {
		struct node *current = list->head;
		struct node *next = current->next;
		struct client *current_client = current->element;
		if (!strcmp(current_client->user_name, user_name)) {
			list->head = next;
			free(current);
			free(current_client);
			client_found = true;
		} 
		while (!client_found  && next != NULL) {  
			current_client = next->element;
			if (!strcmp(current_client->user_name, user_name)) {
				current->next = next->next;
				free(next);
				free(current_client);
				client_found = true;
			} else {
				current = next;
				next = current->next;
			}
		} 
	}
    pthread_mutex_unlock(&list->mutex_list);
	if (!client_found)
		perror("Error: client not found\n");
	return !client_found;
}


int delete_message(struct linked_list *list, int message_id) {
    pthread_mutex_lock(&list->mutex_list);

    int res = -1;

    if(list == NULL) perror("Error, the list doesn't exist\n");

    else if(list->head == NULL) perror("Error, the list is empty\n");

    else {
        struct node *current = list->head;
        struct message *current_message = current->element;
        if(message_id == current_message->id){
            list->head =current->next;
            res = 0;
        }

        else {
            bool message_found = false;

            while(current->next->next != NULL && !message_found) {
                current_message = current->next->element;
                
                if(message_id == current_message->id) {
                    message_found = true;

                    struct node *aux = current->next->next;
					free(current->next->element);
                    free(current->next);
                    current->next = aux;
                    res = 0;
                }

                current = current->next;
            }
        
            if(!message_found) {
                perror("Error: client not found\n");
                res = -1;
            }
        } 
    }

    pthread_mutex_unlock(&list->mutex_list);

    return res;
}


int find_client(struct linked_list *list, char *user_name) {
    pthread_mutex_lock(&list->mutex_list);

    int res = 0;

    if(list == NULL) {
        res = -1;
        perror("Error, the list doesn't exist\n");
    } 

    else if(list->head == NULL) res = 0;

    else {
        struct node *current = list->head;

        while(current != NULL) {
            struct client *current_client = current->element;
            
            if(strcmp(current_client->user_name, user_name) == 0)
                res = 1;

            current = current->next;
        }
    }

    pthread_mutex_unlock(&list->mutex_list);

    return res;
}

int find_message(struct linked_list *list, int message_id) {
    pthread_mutex_lock(&list->mutex_list);

    int res = 0;

    if(list == NULL) {
        res = -1;
        perror("Error, the list doesn't exist\n");
    } 

    else if(list->head == NULL) res = 0;

    else {
        struct node *current = list->head;

        while(current->next != NULL) {
            struct message *current_message = current->element;
            
            if(message_id == current_message->id)
                res = 1;

            current = current->next;
        }
    }

    pthread_mutex_unlock(&list->mutex_list);

    return res;
}


int pop_message(struct linked_list *list, struct message **msg) {
	int list_lenght = length(list);

	if (list_lenght <= 0) return -1;

	*msg  = list->head->element;

	struct node *node = list->head;
	list->head = node->next;
	free(node);

	return --list_lenght;
}

int length(struct linked_list *list) {
    pthread_mutex_lock(&list->mutex_list);

    int res = -1;

    if(list == NULL) perror("Error, the list doesn't exist\n");

    else if(list->head == NULL) res = 0;

    else {
        res = 0;
        struct node *current = list->head; 
        while(current != NULL) {
            current = current->next;
            res++;
        }
    }

    pthread_mutex_unlock(&list->mutex_list);

    return res;
}


void print_list(struct linked_list *list) {
    pthread_mutex_lock(&list->mutex_list);

    if(list == NULL) perror("Error, the list doesn't exist\n");

    else if(list->head == NULL) perror("Error, the list is empty\n");

    else {
        struct node *current = list->head;
        struct client *current_client = current->element;
        int i = 0;
        while(current != NULL) {
            printf("Client %d:\n", i);
            printf("    user_name: %s\n", current_client->user_name);
            printf("    state: %d\n", current_client->state);
            printf("    ip: %s\n", current_client->port);
            printf("    port: %s\n", current_client->port);
            printf("    messages:\n");

            struct node *current_message_node = current_client->messages.head;
            struct message *current_message = current_message_node->element;
            int j = 0;

            while(current_message_node != NULL) {
                printf("        Message %d:\n", j);
                printf("            id: %d\n", current_message->id);
                printf("            message: %s\n", current_message->message);

                j++;
                current_message_node = current_message_node->next;
            }

            i++;
            current =current->next;
        }
    }

    pthread_mutex_unlock(&list->mutex_list);
}

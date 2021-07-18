#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/server.h"
#include "../include/linked_list.h"
#include "../include/lines.h"
#include "../include/error_message.h"

#define MAX_THREADS 10
#define MAX_REQUESTS 256
#define MAX_SIZE 256

int sc_buffer [MAX_REQUESTS];
int end = false;
unsigned int working_threads = 0;

pthread_mutex_t mutex;
pthread_mutex_t mutex_sd;
pthread_mutex_t m_end;

char ip_rpc[15];
unsigned char can_copy = 0;

pthread_cond_t all_threads_working;
pthread_cond_t any_thread_working;
pthread_cond_t is_copy;

struct linked_list register_user;

// *********************************************************************************
// *********************** AUXILIAR FUNCTIONS **************************************
// *********************************************************************************
int get_client_host(char *ip, int16_t port) {
	struct sockaddr_in client_address;
	struct hostent *hp;

	// First of all, we create the socket
	int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sd < 0){
			perror("Error in socket\n");
			return -1;
	}

	// Second, we get the server address
	bzero((char *)&client_address, sizeof(client_address));
	hp = gethostbyname(ip);
	if (hp == NULL) {
			perror("Error en gethostbyname\n");
			return -1;
	}
	// And set the corresponding family and port number
	memcpy (&(client_address.sin_addr), hp->h_addr, hp->h_length);
	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(port);

	// Third, we connect the client's socket to the server's
	if(connect(sd, (struct sockaddr *) &client_address, sizeof(client_address)) == -1) {
		perror("Error while connecting to the client\n");
		return -1;
	}

	return sd;

}

int search_client(char *user_name, struct client **client_str) {
    int error = get_client(&register_user, user_name, client_str);

    if(error == -2) {
        return -1;
    }
    
	return 0;

}

int connect_client(char *user_name, struct client *client) {
	if (search_client(user_name, &client)  == -1) return -1;

	int sd = get_client_host(client->ip, atoi(client->port));

	return sd;
}

int recieve_pending_messages(struct client *receiver) {
	struct message *msg;

	int length;

    // We pop each message from the client's pending messages list and send it to it
	while ((length = pop_message(&receiver->messages, &msg)) >= 0) {
		printf(" SEND MESSAGE %d FROM %s TO %s\ns>", msg->id, msg->sender, receiver->user_name);
		send_message(receiver->user_name, msg);
	}
	
	return 0;
}

int send_message(char *user_name, struct message *message) {
    // First of all, we prepare both the sender's and receiver's structures
	struct client *receiver = (struct client *)malloc(sizeof(struct client));
    bzero(receiver, sizeof(struct client));

	struct client *sender = (struct client *)malloc(sizeof(struct client));
    bzero(receiver, sizeof(struct client));
	

	// Now, we connect to the client's listening thread
	int sd = connect_client(user_name, receiver);
	if (sd <0 ) return sd;


    // And send the codified message -> SEND_MESSAGE\0<id>\0<sender>\0<message>\0
    char buffer[1000];
    int length = sprintf(buffer, "SEND_MESSAGE%c%d%c%s%c%s%c", 0, message->id, 0, message->sender, 0, message->message, 0);
    
    // To send the message, we simply write into the socket input
	int sended = 0;
	do {
		sended += write(sd, buffer+sended, length-sended);
	}while(sended<length);


	// Finally, we repeat all of the process to send the confirmation token to the sender of the message
	int nsd = connect_client(message->sender, sender);
	if (nsd < 0) return nsd;

	bzero(buffer, 1000);
	length = sprintf(buffer, "SEND_MESS_ACK%c%d", 0, message->id);
	sended = 0;
	do {
		sended += write(nsd, buffer+sended, length-sended);
	}while(sended<length);


	// And close all of the connections
	close(sd);
	close(nsd);

	free(receiver);
	free(sender);


	return 0;
}

void prepare_structure(struct client *client, struct client_id client_id) {
	client->state = DISCONNECTED;
    client->last_message_id = 0;
	strcpy(client->ip, client_id.ip);
	strcpy(client->port, client_id.port);
    init(&client->messages);
}



// *********************************************************************************
// *********************** SERVICES' IMPLEMENTATIONS *******************************
// *********************************************************************************
int register_IMPLEMENTATION (char *user_name, struct client_id client_id) {
    // -----------------------------------------------------------
    // ************* LOCAL STORAGE EXECUTION *********************
    // -----------------------------------------------------------

	// We need to know if the user trying to register already exists
	int find = find_client(&register_user, user_name);

	if (find == 1) {
        printf("REGISTER %s FAIL\ns> ", user_name);
        return USER_ALREADY_EXISTS;
    } 

	else if (find == -1) {
        printf("REGISTER %s FAIL\ns> ", user_name);
        return ERROR_REGISTER;
    }  

	// If it doesn't exist, we create the client locally
	struct client *client = (struct client *)malloc(sizeof(struct client));
    bzero(client, sizeof(struct client));
	strcpy(client->user_name, user_name);
    
	prepare_structure(client, client_id);

	int add = add_client(&register_user, client);
    if(add == -1) {
        printf("REGISTER %s FAIL\ns> ", user_name);
        return ERROR_REGISTER;
    }


    // ----------------------------------------------------------------
    // ************* REMOTE RPC STORAGE EXECUTION *********************
    // ----------------------------------------------------------------

    CLIENT *clnt;
	enum clnt_stat retval;

    int result;

	clnt = clnt_create(ip_rpc, register_archive, register_archive1, "tcp");
	if (clnt == NULL) {
        delete_client(&register_user, user_name);
		clnt_pcreateerror(ip_rpc);
		return -1;
		
	}

    retval = register_1(user_name, &result, clnt);

    // We check that the user hasn't been already created on the remote storage
    if (result == 1) {
        printf("REGISTER %s FAIL\ns> ", user_name);
        result = USER_ALREADY_EXISTS;
        delete_client(&register_user, user_name);
    } 

    // And that the RPC operation didn't fail
	else if (result == -1) {
        printf("REGISTER %s FAIL\ns> ", user_name);

        // In case that the register operation failed, we need to remove the already created user in the local storage in order
        // to preserve coherence withing the two storages
        delete_client(&register_user, user_name);
        result = ERROR_REGISTER;
    }  

    else if (retval != RPC_SUCCESS) {
		clnt_perror(clnt, "call failed:");
        delete_client(&register_user, user_name);
	}
	
	clnt_destroy(clnt);

	if (result == 0) printf("REGISTER %s OK\ns> ", user_name);

    return result;
}

int unregister_IMPLEMENTATION (char *user_name) {
    // -----------------------------------------------------------
    // ************* LOCAL STORAGE EXECUTION *********************
    // -----------------------------------------------------------

	// store the client in case of something was wrong
	struct client *client = (struct client *) malloc(sizeof(struct client));
	bzero(client, sizeof(struct client));
	get_client(&register_user, user_name, &client);

    int result = delete_client(&register_user, user_name);

    if(result != 0) {
       printf("UNREGISTER %s FAIL\ns> ", user_name);

       return result == 1 ? USER_NOT_EXISTS : ERROR_UNREGISTER;
    }


    // ----------------------------------------------------------------
    // ************* REMOTE RPC STORAGE EXECUTION *********************
    // ----------------------------------------------------------------

    CLIENT *clnt;
	enum clnt_stat retval;

    clnt = clnt_create(ip_rpc, register_archive, register_archive1, "tcp");
	if (clnt == NULL) {
		add_client(&register_user, client);
		clnt_pcreateerror(ip_rpc);
		return -1;
	}

    retval = unregister_1(user_name, &result, clnt);

    if(result != 0) {
		printf("UNREGISTER %s FAIL\ns> ", user_name);
		add_client(&register_user, client);

       result = result == 1 ? USER_NOT_EXISTS : ERROR_UNREGISTER;
    } else if (retval != RPC_SUCCESS) {
		clnt_perror(clnt, "call failed:");
		add_client(&register_user, client);
	}
	
	clnt_destroy(clnt);

	if (result == 0) printf("UNREGISTER %s OK\ns> ", user_name);

    return result;
}

int connect_IMPLEMENTATION (char *user_name, struct client_id client_id) {
    // First of all, we get the client to connect to
	struct client *client = (struct client *) malloc(sizeof(struct client));

	bzero(client, sizeof(struct client));

	int r = get_client(&register_user, user_name, &client);
	if (r <= -1) {
        printf("CONNECT %s FAIL\ns> ", user_name);

		return USER_NOT_EXISTS;
	}

    // Check that it is not already connected
	if (client->state != DISCONNECTED) {
        printf("CONNECT %s FAIL\ns> ", user_name);

        return USER_ALREADY_CONNECTED;
    }
		
    // And form its new client_id structure
    strcpy(client->ip, client_id.ip);
    strcpy(client->port, client_id.port);
	client->state = CONNECTED;

    printf("CONNECT %s OK\ns> ", user_name);

    // Whenever a client connects' we make it recieve its pending message if there are any
	recieve_pending_messages(client);
	
	return 0;
}

int disconnect_IMPLEMENTATION (char *user_name) {
    // We simply get the client from the list
	struct client *client;
	int r = get_client(&register_user, user_name, &client);

    // Check that we can disconnect it
	if (r == -2) {
        printf("DISCONNECT %s FAIL\ns> ", user_name);

        return USER_NOT_EXISTS;
    } 

    if(r == -1) {
        printf("DISCONNECT %s FAIL\ns> ", user_name);

        return ERROR_DISCONNECT;
    } 

    if(client->state != CONNECTED) {
        printf("DISCONNECT %s FAIL\ns> ", user_name);

        return USER_NOT_CONNECTED;
    } 

    // And change its state
	client->state = DISCONNECTED;
	bzero(client->ip, sizeof(client->ip));
	bzero(client->port, sizeof(client->port));

    printf("DISCONNECT %s OK\ns> ", user_name);

    return r;
}

int send_IMPLEMENTATION (char *sender, char *reciever, char *message, unsigned int *message_id) {
    // First of all, we get both the sender and the reciever
	struct client *client1, *client2;
	struct message *msg = (struct message *)malloc(sizeof(struct message));

	int r1 = get_client(&register_user, sender, &client1),
		r2 = get_client(&register_user, reciever, &client2);

	if (r1 || r2) return USER_NOT_EXISTS;

    // Then, we prepare the message's structure
	strcpy(msg->sender, sender);
	strcpy(msg->message, message);

    // And increse the last massage id atribute of the sender
    client1->last_message_id++;
    msg->id = client1->last_message_id;

    // If the reciever is connected, we send directly the message to it
    if(client2->state == CONNECTED) {
        if((send_message(client2->user_name, msg)) == -1)
            return ERROR_SEND;

        printf("SEND MESSAGE %d FROM %s TO %s\ns> ", msg->id, msg->sender, client2->user_name);
    }

    // Otherwise, we add the message to its pending messages' list    
    else {
        if((add_message(&(client2->messages), msg)) == -1)
            return ERROR_SEND;
        
        printf("MESSAGE %d FROM %s TO %s STORED\ns> ", msg->id, msg->sender, client2->user_name);
    }
	*message_id = msg->id;

    return 0;
}



// ********************************************************************
// *********************** MAIN SERVICE *******************************
// ********************************************************************
int process_request(char *operation_code, struct client_id client_id, unsigned int *message_id) {
    // Because we use the user name of the invoker-client in every single operation, we get this element on the very start of the process
    char user_name[MAX_SIZE];
	int sc = client_id.sd;
    if(readLine(sc, user_name, MAX_SIZE) == -1) {
        perror("Error while receiving the client's message\n");
        end_service(sc);
        return -1;
    }
    

    // REGISTER Request
    if (strcmp(operation_code, "REGISTER") == 0)
        return register_IMPLEMENTATION (user_name, client_id);


    // UNREGISTER Request
    else if (strcmp(operation_code, "UNREGISTER") == 0)
        return unregister_IMPLEMENTATION (user_name);


    // CONNECT Request
    else if (strcmp(operation_code, "CONNECT") == 0) {
        // Get the client's port
        char client_port[MAX_SIZE];
        if(readLine(sc, client_port, MAX_SIZE) == -1) {
            perror("Error while receiving the client's message\n");
            end_service(sc);
            return -1;
        }

        strcpy(client_id.port, client_port);

        return connect_IMPLEMENTATION (user_name, client_id);
    }


    // DISCONNECT Request
    else if (strcmp(operation_code, "DISCONNECT") == 0)
        return disconnect_IMPLEMENTATION (user_name);


    // SEND Request
    else if (strcmp(operation_code, "SEND") == 0) {
        // Get the reciever's user name
        char reciever[MAX_SIZE];
        if(readLine(sc, reciever, MAX_SIZE) == -1) {
            perror("Error while receiving the client's message\n");
            end_service(sc);
            return -1;
        }

        // Get the message to send
        char message[MAX_SIZE];
        if(readLine(sc, message, MAX_SIZE) == -1) {
            perror("Error while receiving the client's message\n");
            end_service(sc);
            return -1;
        }

        return send_IMPLEMENTATION (user_name, reciever, message, message_id);
    }

    return -1;
}

void end_service(int local_sc) {
    close(local_sc);

    pthread_mutex_lock(&mutex);

    working_threads--;
    pthread_cond_signal(&all_threads_working);

    pthread_mutex_unlock(&mutex);
}

int service(void *sc) {
    // First of all, we copy the socket descriptor to our local scope
    struct client_id *pointer_client_id = (struct client_id *) sc;
	struct client_id client_id;
    
    // Threads' main execution loop
    for(;;) {

        // We check if there are no working threads right now
        pthread_mutex_lock(&mutex);

		while(!working_threads) 
			pthread_cond_wait(&any_thread_working, &mutex);
		
		pthread_mutex_unlock(&mutex);


        // If any thread is copying the socket descriptor que wait
        pthread_mutex_lock(&mutex_sd);

		while(!can_copy)
          pthread_cond_wait(&is_copy, &mutex_sd);

		client_id = *pointer_client_id;
		can_copy = 0;

		pthread_mutex_unlock(&mutex_sd);
        
        char buffer [MAX_SIZE];
        bzero(buffer, sizeof(buffer));

        // First, we receive the word indicating the operation to execute
	    if(readLine(client_id.sd, buffer, MAX_SIZE) == -1) {
            perror("Error while receiving the client's message\n");
            end_service(client_id.sd);
            continue;
        }

        // Process the request
		unsigned int message_id = 0;
        int result = process_request(buffer, client_id, &message_id);

        bzero(buffer, sizeof(buffer));
        buffer[0] = (char) result;
		int l = 1;
		if (message_id > 0) 
			l = sprintf(buffer, "%c%c%d", buffer[0], 0, message_id);

        // And send the reply to the client
        if(sendMessage(client_id.sd, buffer, l) == -1) {
            perror("Error when sending replay to client\n");
            end_service(client_id.sd);
            continue;
        }

        end_service(client_id.sd);
    }

    pthread_exit(0); 
}


/* *
 * Main execution of the program
 * 
 * 
 * @param args number of arguments of the function call
 * @param argv[] arguments of the function, expected format ./server -p <server_port> -s <RPC_IP>
 * @return 0 if correct execution and -1 if error occured
 * */
int main(int args, char* argv[]) {
    // Argument treatment
    if (args != 5) {
        printf("Usage: server -p <port> -s <IP_RPC>\n");
        return -1;
	}

    strcpy(ip_rpc, argv[4]);

    char host[20];
    gethostname(host, 20);
    printf("s> init server %s:%s\n", host, argv[2]);

    write(1, "s> ", 3);

    int sd;
	int val;
	int err;

    // Socket creation
    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd < 0) {
        perror("Error in socket");
        return -1;
    }

    // Socket's options assigments
    val = 1;
    err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));
    if (err < 0) {
        perror("Error in option\n");
        return -1;
    }


    
    // Get both server's and client's addresses
    struct sockaddr_in server_addr, client_addr;

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[2]));

    // Bind both addresses
    if(bind(sd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Error in binding the conexion\n");
        close(sd);
        return -1;
    }

    // Wait for clients to send requests
    if(listen(sd, SOMAXCONN) == -1) {
        perror("Error in listening to the client's conexion\n");
        close(sd);
        return -1;
    }

    socklen_t size = sizeof(client_addr);

    pthread_attr_t t_attr;
    pthread_t thid[MAX_THREADS];

    // Sincronization tools initialization
    pthread_mutex_init (&mutex, NULL);
    pthread_mutex_init (&m_end, NULL);
    pthread_mutex_init (&mutex_sd, NULL);

    pthread_cond_init (&all_threads_working, NULL);
    pthread_cond_init (&any_thread_working, NULL);
    pthread_cond_init (&is_copy, NULL);


	struct client_id client_id;

    // We create the pool of threads
    pthread_attr_init(&t_attr);

    for (int i = 0; i < MAX_THREADS; i++) {
        if (pthread_create(&thid[i], NULL, (void *) service, (void *) &client_id) != 0) {
            perror ("Error ocurred while creating pool of threads\n");
            return -1;
        }
    }

    // Main execution loop
    while (true) {

        // If all threads are ocuppied, we wait
        pthread_mutex_lock (&mutex);

        while (working_threads == MAX_THREADS)
            pthread_cond_wait(&all_threads_working, &mutex);

        pthread_mutex_unlock (&mutex);

        // We receive a client request
        client_id.sd = accept(sd, (struct sockaddr *) &client_addr, (socklen_t *) &size);
		strcpy(client_id.ip, inet_ntoa(client_addr.sin_addr));
        if(client_id.sd == -1) {
            perror("Error in receiving a client's message\n");
            break;
        }



        // Decrease the number of available threads
        pthread_mutex_lock (&mutex);

        working_threads++;

        pthread_cond_signal(&any_thread_working);

        pthread_mutex_unlock(&mutex);


        // Pass the socket descriptor to the local thread scopes
        pthread_mutex_lock (&mutex_sd);

        can_copy = 1;

        pthread_cond_signal(&is_copy);

        pthread_mutex_unlock(&mutex_sd);
    }

    // Server execution finish
    pthread_mutex_lock(&m_end);
    end = true;
    pthread_mutex_unlock (&m_end);

    pthread_mutex_lock (&mutex);
    pthread_cond_broadcast(&any_thread_working);
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i<MAX_THREADS; i++) {
        if(pthread_join(thid[i], NULL) != 0) {
            perror("Error in waiting threads to end\n");
            return -1;
        }
    }

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_sd);
    pthread_cond_destroy(&any_thread_working);
    pthread_cond_destroy(&all_threads_working);
    pthread_mutex_destroy(&m_end);

    close(sd);

    printf("All requests were attended!\n");

    return 0;
}

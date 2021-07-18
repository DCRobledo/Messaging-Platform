#include "../../RPC/register_archive.h"
#include "../../server/include/linked_list.h"

struct linked_list register_user;

bool_t
register_1_svc(char *user_name, int *result,  struct svc_req *rqstp)
{

	*result = find_client(&register_user, user_name);

    if (*result == 0) {
        struct client *client = (struct client *)malloc(sizeof(struct client));
        bzero(client, sizeof(struct client));
        strcpy(client->user_name, user_name);
        client->last_message_id = 0;
        
       *result = add_client(&register_user, client); 
    }
    

	return *result == 0;
}

bool_t
unregister_1_svc(char *user_name, int *result,  struct svc_req *rqstp)
{

    *result = delete_client(&register_user, user_name);

	return *result == 0;
}

bool_t
find_1_svc(char *user_name, client_rpc *result,  struct svc_req *rqstp)
{

	struct client_rpc *client = (struct client_rpc *) malloc(sizeof(struct client_rpc));
    
	bzero(client, sizeof(struct client_rpc));

	int get = get_client_rpc(&register_user, user_name, &client);
	
	if(get == -2) result->error_code = -1;

	return get == 0;
}

int
register_archive_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free (xdr_result, result);

	/*
	 * Insert additional freeing code here, if needed
	 */

	return 1;
}
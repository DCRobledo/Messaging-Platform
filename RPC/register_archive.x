struct client_rpc {
    opaque user_name[255];
    unsigned int last_message_id;
    char error_code;
};

program register_archive{
    version register_archive1{
        int REGISTER(string user_name<255>) = 2;
        int UNREGISTER(string user_name<255>) = 3;
        client_rpc FIND(string user_name<255>) = 4;
    } = 1;
} = 99;
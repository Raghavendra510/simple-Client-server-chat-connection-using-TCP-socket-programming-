# simple-Client-server-chat-

TCP CONNECTION USING SOCKET PROGRAMMING IN C LANGUAGE 

-> we have two files server and client 
->to run :
            compile server : gcc server.c -o s -lpthread
            run : ./s <port_number>

  compile clients in different terminals : gcc  client.c -o c -lpthread
  run : ./c <server_ip> <port_number>

->usage : 
        each client has a name 
        clients can just type a message to broadcast it 
        clients can use '@username' to send to a particular client (private messaging)
        clients can use '/list' command to see the list of active users that are connected to the server 
        clients can use '-1' to disconnect from the server 
        each time when a client connects or disconnects , server displays a notification 
        if a client tries to contact an offline client then it shows error 

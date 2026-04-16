# simple-Client-server-chat-

TCP CONNECTION USING SOCKET PROGRAMMING IN C LANGUAGE 

-> we have two files server and client <br>
->to run :<br>
            compile server : gcc server.c -o s -lpthread<br>
            run : ./s <port_number> <br>
<br><br>
  compile clients in different terminals : gcc  client.c -o c -lpthread
  <br>run : ./c <server_ip> <port_number>
<br><br><br>
->usage : <br>
        each client has a name<br> 
        clients can just type a message to broadcast it <br>
        clients can use '@username' to send to a particular client (private messaging)<br>
        clients can use '/list' command to see the list of active users that are connected to the server <br>
        clients can use '-1' to disconnect from the server <br>
        each time when a client connects or disconnects , server displays a notification <br>
        if a client tries to contact an offline client then it shows error <br>

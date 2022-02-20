# OSN
## Assignment 5
## Question 3

## Server:

### To compile 
```
g++ server.cpp -lpthread -o server
```
### To run
```
./server <number_of_worker_threads>
```

* The server listens for connections on the LISTEN port.

* When the server recieves a connection request it adds the socket_fd onto a queue.

* There are m worker threads that take up the task of executing the operations given by the clients in the queue.

* The worker threads pop out the top most socket_fd in the queue and start reading from it.

* The dictionary has been imlemented using a map of pair<int,string>.

* The possible operations are:
1. insert
```
insert <key> <value> : This command is supposed to create a new “key” on the server’s dictionary and set its
value as <value>.
```
2. update
```
update <key> <value> : This command is supposed to update the value corresponding to <key> on the
server’s dictionary and set its value as <value>.
```
3. concat
```
concat <key1> <key2> : Let values corresponding to the keys before execution of this command be {key1:
value_1, key_2:value_2}. Then, the corresponding values after this command's execution should be {key1:
value_1+value_2, key_2: value_2+value_1}.
```
4. delete
```
delete <key> : This command is supposed to remove the <key> from the dictionary.
5. fetch
```
fetch <key> : Must display the value corresponding to the key if it exists at the connected server, and an error
“Key does not exist” otherwise
```

* The worker threads runs the command given by the client and returns the output string appended to the worker thread id to the resective client.

## Client:

### To compile 
```
g++ client.cpp -lpthread -o client
```
### To run
```
./client
```

* The client creates thread for each operation given by the user.

* The thread simulates different client and tries to connect to the server on SERVER_PORT.

* On connection to the server, the thread(client) sends the operation in the form of a string to the server which is taken up by one of the worker threads on the server side.

* The client then waits for the server to execcute the operation and return the ouput.

* The client then reads the string sent by the server and prints it out on the terminal in the right format for the user.

* The client then drops the connection with the server.
# Multithreaded-Server-Client

Entities simulated using threads:

1. Server
2. Client

### General working

The client program spawns multiple threads to request from the server. Each request is to modify a global dictionary. The server listens to the client requests and when obtained, pushes the client socket fd into a queue and calls one of its worker threads from the thread pool to service the request

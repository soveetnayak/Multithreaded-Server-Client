#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <iostream>
#include <assert.h>
#include <tuple>
#include <vector>
#include <map>
#include <bits/stdc++.h>
using namespace std;

#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define ANSI_RESET "\x1b[0m"

typedef long long LL;

#define MAX_CLIENTS 100
#define PORT_ARG 8000

int worker_threads;

map<int, string> dictionary;
queue<int> q;

pthread_mutex_t mutex_dictionary = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_q = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_q = PTHREAD_COND_INITIALIZER;

const int initial_msg_len = 256;
const LL buff_sz = 1048576;

pair<string, int> read_string_from_socket(const int &fd, int bytes)
{
    string output;
    output.resize(bytes);

    int bytes_received = read(fd, &output[0], bytes - 1);
    if (bytes_received <= 0)
    {
        cerr << "Failed to read data from socket.\n";
    }
    output[bytes_received] = 0;
    output.resize(bytes_received);
    return {output, bytes_received};
}

int send_string_on_socket(int fd, const string &s)
{
    // debug(s.length());
    int bytes_sent = write(fd, s.c_str(), s.length());
    if (bytes_sent < 0)
    {
        cerr << "Failed to SEND DATA via socket.\n";
    }

    return bytes_sent;
}

string map_operation(string cmd)
{
    string final;
    string operation;
    for (int i = 0; i < cmd.length(); i++)
    {
        if (cmd[i] == ' ')
        {
            operation = cmd.substr(0, i);
            break;
        }
    }
    vector<string> tokens;

    // stringstream class check1
    stringstream check1(cmd);

    string intermediate;

    // Tokenizing w.r.t. space ' '
    while (getline(check1, intermediate, ' '))
    {
        tokens.push_back(intermediate);
    }
    pthread_mutex_lock(&mutex_dictionary);
    if (operation == "insert")
    {
        int key = stoi(tokens[1]);
        string value = tokens[2];
        if (dictionary.find(key) == dictionary.end())
        {
            dictionary[key] = value;
            //cout << "Insertion successful" << endl;
            final = "Insertion successful";
        }
        else
        {
            // cout << "Key already exists" << endl;
            final = "Key already exists";
        }
    }
    else if (operation == "delete")
    {
        int key = stoi(tokens[1]);
        if (dictionary.find(key) != dictionary.end())
        {
            dictionary.erase(key);
            //cout << "Deletion successful" << endl;
            final = "Deletion successful";
        }
        else
        {
            //cout << "No such key exists" << endl;
            final = "No such key exists";
        }
    }
    else if (operation == "update")
    {
        int key = stoi(tokens[1]);
        string value = tokens[2];
        if (dictionary.find(key) != dictionary.end())
        {
            dictionary[key] = value;
            //cout << dictionary[key] << endl;
            final = dictionary[key];
        }
        else
        {
            // cout << "Key does not exist" << endl;
            final = "Key does not exist";
        }
    }
    else if (operation == "concat")
    {
        int key1 = stoi(tokens[1]);
        int key2 = stoi(tokens[2]);

        if (dictionary.find(key1) != dictionary.end() && dictionary.find(key2) != dictionary.end())
        {
            string value1 = dictionary[key1];
            string value2 = dictionary[key2];
            dictionary[key1] = value1 + value2;
            dictionary[key2] = value2 + value1;
            //cout << dictionary[key2] << endl;
            final = dictionary[key2];
        }
        else
        {
            //cout << "Concat failed as at least one of the keys does not exist" << endl;
            final = "Concat failed as at least one of the keys does not exist";
        }
    }
    else if (operation == "fetch")
    {
        int key = stoi(tokens[1]);
        if (dictionary.find(key) != dictionary.end())
        {
            //cout << dictionary[key] << endl;
            final = dictionary[key];
        }
        else
        {
            //cout << "Key does not exist" << endl;
            final = "Key does not exist";
        }
    }
    else
    {
        //cout << "Invalid operation" << endl;
        final = "Invalid operation";
    }
    pthread_mutex_unlock(&mutex_dictionary);
    return final;
}

void handle_connection(int client_socket_fd, int thread_id)
{
    int received_num, sent_num;
    int ret_val = 1;

    string cmd;
    tie(cmd, received_num) = read_string_from_socket(client_socket_fd, buff_sz);
    ret_val = received_num;
    if (ret_val <= 0)
    {
        printf("Server could not read msg sent from client\n");
        // goto close_client_socket_ceremony;
    }
    cout << "Client: " << cmd << endl;
    string msg_to_send_back = map_operation(cmd);
    string final = msg_to_send_back;
    final = to_string(thread_id) + ":" + final;

    sleep(2);
    cout << "Server: " << final << endl;
    int sent_to_client = send_string_on_socket(client_socket_fd, final);
    if (sent_to_client == -1)
    {
        perror("Error while writing to client. Seems socket has been closed");
        // goto close_client_socket_ceremony;
    }

    // close_client_socket_ceremony:
    close(client_socket_fd);
    printf(BRED "Disconnected from client" ANSI_RESET "\n");
}

void *thread_function(void *arg)
{
    int *thread_id = (int *)arg;
    while (true)
    {
        int client_socket_fd;
        pthread_mutex_lock(&mutex_q);
        //cout << "Thread:" << *thread_id << endl;
        if (q.empty())
        {
            pthread_cond_wait(&cond_q, &mutex_q);
        }
        client_socket_fd = q.front();
        q.pop();
        pthread_mutex_unlock(&mutex_q);
        handle_connection(client_socket_fd, *thread_id);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./server <worker threads>\n");
        return 0;
    }

    socklen_t clilen;

    struct sockaddr_in serv_addr_obj, client_addr_obj;
    int wel_socket_fd, client_socket_fd, port_number;

    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (wel_socket_fd < 0)
    {
        perror("Error creating welcoming socket");
        exit(-1);
    }

    bzero((char *)&serv_addr_obj, sizeof(serv_addr_obj));
    port_number = PORT_ARG;
    serv_addr_obj.sin_family = AF_INET;
    serv_addr_obj.sin_addr.s_addr = INADDR_ANY;
    serv_addr_obj.sin_port = htons(port_number); //process specifies port

    if (bind(wel_socket_fd, (struct sockaddr *)&serv_addr_obj, sizeof(serv_addr_obj)) < 0)
    {
        perror("Error on bind on welcome socket");
        exit(-1);
    }

    listen(wel_socket_fd, MAX_CLIENTS);
    cout << "Server has started listening on the LISTEN PORT" << endl;
    clilen = sizeof(client_addr_obj);

    worker_threads = atoi(argv[1]);
    cout << "Number of worker threads: " << worker_threads << endl;
    pthread_t worker_threads_arr[worker_threads];
    int worker_threads_id[worker_threads];
    for (int i = 0; i < worker_threads; i++)
    {
        worker_threads_id[i] = i;
        pthread_create(&worker_threads_arr[i], NULL, thread_function, (void *)(&worker_threads_id[i]));
    }

    while (1)
    {
        printf("Waiting for a new client to request for a connection\n");
        client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fd < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }

        printf(BGRN "New client connected from port number %d and IP %s \n" ANSI_RESET, ntohs(client_addr_obj.sin_port), inet_ntoa(client_addr_obj.sin_addr));

        pthread_mutex_lock(&mutex_q);
        q.push(client_socket_fd);
        pthread_cond_signal(&cond_q);
        pthread_mutex_unlock(&mutex_q);
    }

    close(wel_socket_fd);
    return 0;
}
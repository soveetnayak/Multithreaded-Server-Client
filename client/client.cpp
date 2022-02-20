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
#include <pthread.h>
#include <iostream>
#include <semaphore.h>
#include <assert.h>
#include <queue>
#include <vector>
#include <tuple>
using namespace std;

#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define ANSI_RESET "\x1b[0m"

#define SERVER_PORT 8000
#define MAX_CLIENTS 100
const long long buff_sz = 1048576;
int socket_fd[MAX_CLIENTS];

struct thread_data
{
    int thread_id;
    pair<int,string> cmd;

};

pair<string, int> read_string_from_socket(int fd, int bytes)
{
    std::string output;
    output.resize(bytes);

    int bytes_received = read(fd, &output[0], bytes - 1);
    // debug(bytes_received);
    if (bytes_received <= 0)
    {
        cerr << "Failed to read data from socket. Seems server has closed socket\n";
        // return "
        exit(-1);
    }

    // debug(output);
    output[bytes_received] = 0;
    output.resize(bytes_received);

    return {output, bytes_received};
}
int send_string_on_socket(int fd, const string &s)
{
    // cout << "We are sending " << s << endl;
    int bytes_sent = write(fd, s.c_str(), s.length());
    // debug(bytes_sent);
    // debug(s);
    if (bytes_sent < 0)
    {
        cerr << "Failed to SEND DATA on socket.\n";
        // return "
        exit(-1);
    }

    return bytes_sent;
}
int get_socket_fd(struct sockaddr_in *ptr)
{
    struct sockaddr_in server_obj = *ptr;

    // socket() creates an endpoint for communication and returns a file
    //        descriptor that refers to that endpoint.  The file descriptor
    //        returned by a successful call will be the lowest-numbered file
    //        descriptor not currently open for the process.
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Error in socket creation for client");
        exit(-1);
    }

    int port_num = SERVER_PORT;

    memset(&server_obj, 0, sizeof(server_obj)); // Zero out structure
    server_obj.sin_family = AF_INET;
    server_obj.sin_port = htons(port_num); //convert to big-endian order

    // Converts an IP address in numbers-and-dots notation into either a
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    //https://stackoverflow.com/a/20778887/6427607

    /////////////////////////////////////////////////////////////////////////////////////////
    /* connect to server */

    if (connect(socket_fd, (struct sockaddr *)&server_obj, sizeof(server_obj)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(-1);
    }

    return socket_fd;
}
void *thread_function(void *arg)
{
    struct thread_data *data = (struct thread_data *)arg;
    string x = data->cmd.second;
    struct sockaddr_in server_obj;
    socket_fd[data->thread_id] = get_socket_fd(&server_obj);

    //cout << "Connection to server successful" << endl;
    string s;
    s = x;
    for (int i = 0; i < s.length(); i++)
    {
        if (s[i] == ' ')
        {
            x[i] = '\0';
            s = s.substr(i + 1);
            break;
        }
    }
    int time = stoi(x);

    sleep(time);

    send_string_on_socket(socket_fd[data->thread_id], s);
    int num_bytes_read;
    string output_msg;
    tie(output_msg, num_bytes_read) = read_string_from_socket(socket_fd[data->thread_id], buff_sz);
    cout << data->cmd.first << ":" << output_msg << endl;

    return NULL;
}
void begin_process(int m)
{
    // struct sockaddr_in server_obj;
    // socket_fd = get_socket_fd(&server_obj);

    // cout << "Connection to server successful" << endl;

    pthread_t threads[m];
    pair <int, string> array[m];
    for (int i = 0; i < m; i++)
    {
        array[i].first = i;
        getline(cin, array[i].second);
    }

    for (int i = 0; i < m; i++)
    {
        struct thread_data *t_data = new thread_data;
        t_data->thread_id = i;
        t_data->cmd = array[i];
        pthread_create(&threads[i], NULL, thread_function, (void *)t_data);
    }

    for (int i = 0; i < m; i++)
    {
        pthread_join(threads[i], NULL);
    }
    // part;
}
int main(int argc, char *argv[])
{
    int m;
    string input;
    getline(cin, input);
    m = stoi(input);
    begin_process(m);
    return 0;
}
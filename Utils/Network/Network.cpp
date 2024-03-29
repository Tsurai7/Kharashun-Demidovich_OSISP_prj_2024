#include "Network.h"
#include "../Clipboard/Clipboard.h"

#define SEND_PORT 8787
#define RECEIVE_PORT 8484 // program can't send info to all users in network with different in/out ports
#define BROADCAST_ADDRESS "255.255.255.255"


void sendBroadcast(const char *message) {
    int socket_fd;
    struct sockaddr_in broadcast_addr;

    // Socket creation
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setting socket info for broadband
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_ADDRESS);
    broadcast_addr.sin_port = htons(SEND_PORT);

    // Enabling broadband
    int broadcast_enable = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) == -1) {
        perror("Error: Setsockopt failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Sending message
    if (sendto(socket_fd, message, strlen(message), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) == -1) {
        printf("Error: Sendto failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("Broadcast message sent: %s\n", message);

    close(socket_fd);
}


void *receiveBroadcast(void *arg) {
    int socket_fd;
    struct sockaddr_in server_address, client_address;

    // Creating socket
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setting server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SEND_PORT);

    // Binding socket to address
    if (bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("Listening for broadcasts...\n");

    while (1) {
        char buffer[1024] = {0};
        socklen_t client_len = sizeof(client_address);

        // Getting message from client
        ssize_t recv_len = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &client_len);

        if (recv_len < 0) {
            perror("Receive failed");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }

        runSetClipCommand(buffer);
        printf("Received message from %s:%d: %s\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), buffer);
    }

    return NULL; // Этот код никогда не выполнится, но требуется для компиляции с pthread
}

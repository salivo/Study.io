#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    const char* server_ip = "127.0.0.1"; // Server IP address
    int server_port = 12345;            // Server port
    int sock = 0;
    struct sockaddr_in server_address;

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error!" << std::endl;
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported!" << std::endl;
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Connection failed!" << std::endl;
        return -1;
    }

    std::cout << "Connected to the server!" << std::endl;

    // Communicate with the server
    std::string message;
    char buffer[1024] = {0};

    while (true) {
        std::cout << "Enter message (type 'bye' to exit): ";
        std::getline(std::cin, message);

        // Send the message to the server
        send(sock, message.c_str(), message.size(), 0);

        // Break the loop if the user types "bye"
        if (message == "bye") {
            std::cout << "Closing connection..." << std::endl;
            break;
        }

        // Receive a response from the server
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0'; // Null-terminate the received data
            std::cout << "Server: " << buffer << std::endl;
        } else {
            std::cerr << "Connection closed by the server!" << std::endl;
            break;
        }

        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));
    }

    // Close the socket
    close(sock);

    return 0;
}

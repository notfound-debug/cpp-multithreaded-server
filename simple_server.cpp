#include <iostream>
#include <cstring>
#include <sys/socket.h> // The main socket library
#include <netinet/in.h> // Structures for storing addresses (sockaddr_in)
#include <unistd.h>     // For close()

int main() {
    // 1. Create the Socket
    // AF_INET = IPv4
    // SOCK_STREAM = TCP (Reliable, connection-based)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // 2. Bind the Socket to an IP and Port
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on ANY network interface (Wi-Fi, Ethernet, localhost)
    address.sin_port = htons(8080);       // Port 8080. htons converts number to network byte order

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed. Is the port busy?" << std::endl;
        return 1;
    }

    // 3. Listen for incoming connections
    // The '10' is the backlog: how many people can wait on hold before we reject them.
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }

    std::cout << "Server is listening on port 8080..." << std::endl;

    while (true) {
        // 4. Accept a connection
        // This BLOCKs the execution until someone connects.
        // It returns a NEW file descriptor specific to this single connection.
        int new_socket = accept(server_fd, nullptr, nullptr);
        if (new_socket < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        // 5. Read data (Basic read, no HTTP parsing yet)
        char buffer[30000] = {0};
        read(new_socket, buffer, 30000);
        
        std::cout << "----- RECEIVED MESSAGE -----\n" << buffer << "\n----------------------------" << std::endl;

        // 6. Send a generic response and close
        const char* hello = "Hello from the raw socket server!";
        send(new_socket, hello, strlen(hello), 0);
        
        close(new_socket); // Hang up the phone
    }

    close(server_fd); // Close the main listener (unreachable code in this while true loop)
    return 0;
}

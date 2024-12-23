/*
* ----------------------------------------------------------------------------
 *  ClientSocket Class for Web Crawling
 * ----------------------------------------------------------------------------
 *  This class handles the crawling of web pages, including sending HTTP GET
 *  requests, receiving responses, and extracting internal and external links
 *  from the pages. It manages socket connections, calculates response times,
 *  and tracks discovered pages and linked sites.
 *
 *  Key Features:
 *  - Sends HTTP GET requests and measures response times.
 *  - Extracts internal and external URLs from the page content.
 *  - Keeps track of discovered pages, linked sites, and response statistics.
 *  - Supports socket management and cleanup (Winsock).
 *
 *  The class can be used as part of a basic web crawler to discover pages and
 *  analyze site performance.
 * ----------------------------------------------------------------------------
 */

#include "clientSocket.h"
#include <chrono>
#include <stdexcept>

using namespace std::chrono;

// Constructor: Initializes the client socket, sets up the hostname, port, page limit, and crawl delay.
ClientSocket::ClientSocket(string hostname, int port, int pagesLimit, int crawlDelay)
    : hostname(hostname), port(port), pagesLimit(pagesLimit), crawlDelay(crawlDelay), sock(INVALID_SOCKET) {

    // Initializes Winsock for socket communication. If it fails, an exception is thrown.
    if (!initializeWinsock()) {
        throw runtime_error("Failed to initialize Winsock");
    }

    // Start with the root path and initialize the discovered pages map to prevent revisiting.
    pendingPages.add("/", "");
    discoveredPages["/"] = true;
}

// Destructor: Cleans up any resources, especially the socket.
ClientSocket::~ClientSocket() {
    cleanup();
}

// Initializes the Winsock library. Returns true if successful, false otherwise.
bool ClientSocket::initializeWinsock() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

// Creates a socket for TCP communication. Returns true if successful, false otherwise.
bool ClientSocket::createSocket() {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return sock != INVALID_SOCKET;
}

// Connects to the given host and port using the previously created socket. Returns true if successful.
bool ClientSocket::connectToHost() {
    struct hostent *host = gethostbyname(hostname.c_str());  // Resolves the host.
    if (!host) return false;

    SOCKADDR_IN sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    sockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

    // Tries to connect to the server.
    return connect(sock, (SOCKADDR*)(&sockAddr), sizeof(sockAddr)) != SOCKET_ERROR;
}

// Cleans up the socket connection and terminates the Winsock library.
void ClientSocket::cleanup() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    WSACleanup();
}

// Creates an HTTP GET request string to fetch the given path from the specified host.
string ClientSocket::createHttpRequest(string host, string path) {
    return "GET " + path + " HTTP/1.1\r\n"
           "Host: " + host + "\r\n"
           "Connection: close\r\n\r\n";
}

// Starts discovering pages, extracting links from them, and keeps track of the crawl process.
SiteStats ClientSocket::startDiscovering() {
    SiteStats stats;
    stats.hostname = hostname;  // Set the hostname in the stats.

    // While there are pages to crawl and within the page limit, continue crawling.
    while (!pendingPages.empty() && (pagesLimit == -1 || int(stats.discoveredPages.getHead() ? 1 : 0) < pagesLimit)) {
        // Get the next page to crawl from the queue.
        string path = pendingPages.front();
        pendingPages.pop();

        // Introduce a delay between requests, except for the root page.
        if (path != "/") Sleep(crawlDelay);

        // Try to create a socket and connect to the host. If it fails, mark the page as failed and continue.
        if (!createSocket() || !connectToHost()) {
            stats.numberOfPagesFailed++;
            cleanup();
            continue;
        }

        // Create and send the HTTP request.
        string request = createHttpRequest(hostname, path);
        auto startTime = high_resolution_clock::now();  // Start timer for measuring response time.

        // Send the HTTP request over the socket.
        if (send(sock, request.c_str(), (int)request.length(), 0) == SOCKET_ERROR) {
            stats.numberOfPagesFailed++;
            cleanup();
            continue;
        }

        string response;
        char buffer[4096];
        double responseTime = -1;

        // Receive the server's response and measure the response time.
        while (true) {
            int bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead <= 0) break;

            if (responseTime < -0.5) {
                auto endTime = high_resolution_clock::now();
                responseTime = duration<double, milli>(endTime - startTime).count();
            }

            buffer[bytesRead] = '\0';
            response += buffer;  // Accumulate the response.
        }

        // Clean up the socket after the response is received.
        closesocket(sock);
        sock = INVALID_SOCKET;

        // Add the discovered page and its response time to the stats.
        stats.discoveredPages.add(hostname + path, to_string(responseTime));

        // Extract URLs from the response HTML and queue them for further crawling if not already discovered.
        LinkedList extractedUrls = extractUrls(response);
        Node* current = extractedUrls.getHead();
        while (current) {
            // If the URL is a valid internal link, add it to the pending pages if not already discovered.
            if (current->url.empty() || current->url == hostname) {
                if (!discoveredPages[current->metadata]) {
                    pendingPages.add(current->metadata, "");
                    discoveredPages[current->metadata] = true;
                }
            } else {
                // If the URL is an external link, add it to the linked sites.
                if (!discoveredLinkedSites[current->url]) {
                    discoveredLinkedSites[current->url] = true;
                    stats.linkedSites.add(current->url, "");
                }
            }
            current = current->next;  // Move to the next extracted URL.
        }
    }

    // Calculate statistics (min, max, and average response times) for the discovered pages.
    Node* page = stats.discoveredPages.getHead();
    double totalTime = 0;
    int count = 0;

    while (page) {
        double responseTime = stod(page->metadata);
        totalTime += responseTime;
        count++;

        // Update min and max response times.
        if (stats.minResponseTime < 0 || responseTime < stats.minResponseTime)
            stats.minResponseTime = responseTime;
        if (stats.maxResponseTime < 0 || responseTime > stats.maxResponseTime)
            stats.maxResponseTime = responseTime;

        page = page->next;  // Move to the next page.
    }

    // Calculate the average response time.
    if (count > 0)
        stats.averageResponseTime = totalTime / count;

    return stats;  // Return the site stats after the crawling is complete.
}

/*
 * ----------------------------------------------------------------------------
 *  ClientSocket Implementation
 * ----------------------------------------------------------------------------
 *  This file implements the ClientSocket class for web crawling operations.
 *  It handles HTTP connections, page retrieval, and statistics gathering in a
 *  thread-safe manner. The implementation is designed to work with a
 *  multi-threaded crawler system.
 * ----------------------------------------------------------------------------
 */

#include "clientSocket.h"
#include <chrono>
#include <stdexcept>
#include <iomanip>
#include <sstream>

using namespace std::chrono;

ClientSocket::ClientSocket(string hostname, int port, int pagesLimit, int crawlDelay)
    : hostname(hostname), port(port), pagesLimit(pagesLimit), crawlDelay(crawlDelay), sock(INVALID_SOCKET) {

    // Initialize Winsock
    if (!initializeWinsock()) {
        throw runtime_error("Failed to initialize Winsock");
    }

    // Add initial page to pending queue
    pendingPages.add("/", "");
    discoveredPages["/"] = true;
}

ClientSocket::~ClientSocket() {
    cleanup();
}

bool ClientSocket::initializeWinsock() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

bool ClientSocket::createSocket() {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET) {
        return false;
    }

    // Set socket timeout to prevent hanging
    DWORD timeout = 10000;  // 10 seconds
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    return true;
}

bool ClientSocket::connectToHost() {
    struct hostent *host = gethostbyname(hostname.c_str());
    if (!host) return false;

    SOCKADDR_IN sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    sockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

    // Set non-blocking mode for connect timeout
    unsigned long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    connect(sock, (SOCKADDR*)(&sockAddr), sizeof(sockAddr));

    // Wait for connection with timeout
    fd_set fdset;
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;

    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);

    if (select(0, NULL, &fdset, NULL, &tv) == 1) {
        // Set back to blocking mode
        mode = 0;
        ioctlsocket(sock, FIONBIO, &mode);
        return true;
    }

    return false;
}

void ClientSocket::cleanup() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    WSACleanup();
}

string ClientSocket::createHttpRequest(string host, string path) {
    return "GET " + path + " HTTP/1.1\r\n"
           "Host: " + host + "\r\n"
           "Connection: close\r\n\r\n";
}

SiteStats ClientSocket::startDiscovering() {
    // Initialize statistics object
    SiteStats stats;
    stats.hostname = hostname;
    stats.averageResponseTime = -1;
    stats.minResponseTime = -1;
    stats.maxResponseTime = -1;
    stats.numberOfPagesFailed = 0;

    // Main crawling loop
    while (!pendingPages.empty() && (pagesLimit == -1 || int(stats.visitedPages.size()) < pagesLimit)) {
        string path = pendingPages.front();
        pendingPages.pop();

        // Implement crawl delay except for initial page
        if (path != "/") {
            Sleep(crawlDelay);
        }

        // Create and connect socket
        if (!createSocket() || !connectToHost()) {
            stats.numberOfPagesFailed++;
            cleanup();
            continue;
        }

        // Send HTTP request and measure response time
        string request = createHttpRequest(hostname, path);
        auto startTime = high_resolution_clock::now();

        if (send(sock, request.c_str(), (int)request.length(), 0) == SOCKET_ERROR) {
            stats.numberOfPagesFailed++;
            cleanup();
            continue;
        }

        // Receive response
        string response;
        char buffer[4096];
        double responseTime = -1;

        while (true) {
            int bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead <= 0) break;

            // Calculate response time on first data received
            if (responseTime < -0.5) {
                auto endTime = high_resolution_clock::now();
                responseTime = duration<double, milli>(endTime - startTime).count();
            }

            buffer[bytesRead] = '\0';
            response += buffer;
        }

        closesocket(sock);
        sock = INVALID_SOCKET;

        // Store page statistics
        string fullUrl = hostname + path;
        stats.visitedPages.push_back(PageStats(fullUrl, responseTime));
        stats.discoveredPages.add(fullUrl, to_string(responseTime));

        // Update response time statistics
        if (stats.minResponseTime < 0 || responseTime < stats.minResponseTime) {
            stats.minResponseTime = responseTime;
        }
        if (stats.maxResponseTime < 0 || responseTime > stats.maxResponseTime) {
            stats.maxResponseTime = responseTime;
        }

        // Process extracted URLs
        LinkedList extractedUrls = extractUrls(response);
        Node* current = extractedUrls.getHead();

        while (current) {
            // Process internal links
            if (current->url.empty() || current->url == hostname) {
                if (!discoveredPages[current->metadata]) {
                    pendingPages.add(current->metadata, "");
                    discoveredPages[current->metadata] = true;
                }
            }
            // Process external links
            else {
                if (!discoveredLinkedSites[current->url]) {
                    discoveredLinkedSites[current->url] = true;
                    stats.linkedSites.add(current->url, "");
                }
            }
            current = current->next;
        }
    }

    // Calculate average response time
    if (!stats.visitedPages.empty()) {
        double totalTime = 0;
        for (const auto& page : stats.visitedPages) {
            totalTime += page.responseTime;
        }
        stats.averageResponseTime = totalTime / stats.visitedPages.size();
    }

    return stats;
}
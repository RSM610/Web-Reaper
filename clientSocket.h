/*
* ----------------------------------------------------------------------------
 *  ClientSocket Header - Web Crawling and HTTP Requests
 * ----------------------------------------------------------------------------
 *  This header defines the ClientSocket class for handling web crawling
 *  operations using socket programming. It manages HTTP requests,
 *  socket connections, and collects statistics such as response times
 *  and discovered pages.
 *
 *  Key Features:
 *  - Establishes socket connections for HTTP requests.
 *  - Crawls websites, extracts internal and external links.
 *  - Tracks response times, discovered pages, and linked sites.
 *  - Supports Winsock initialization and cleanup.
 *
 *  The ClientSocket class is integral for performing web crawling tasks
 *  and gathering performance metrics for websites.
 * ----------------------------------------------------------------------------
 */

#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

// Include necessary libraries for Winsock programming, networking, and standard C++ functionality
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <map>
#include "parser.h"  // Custom parser header for extracting links from HTML responses

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib") // Link the Winsock library only for MSVC (Microsoft Visual C++)
#endif

using namespace std;

// Struct to store statistics about a website
struct SiteStats {
    string hostname;                  // Hostname or base URL of the website being crawled
    double averageResponseTime = -1;  // The average response time for pages crawled
    double minResponseTime = -1;      // The minimum response time encountered
    double maxResponseTime = -1;      // The maximum response time encountered
    int numberOfPagesFailed = 0;      // Number of pages that failed to be discovered (connection issues, etc.)
    LinkedList linkedSites;           // A linked list to store the sites that are linked from the current website
    LinkedList discoveredPages;       // A linked list to store the discovered pages and their response times
};

class ClientSocket {
public:
    // Constructor to initialize the socket client with the given hostname, port, page limit, and crawl delay
    ClientSocket(string hostname, int port = 80, int pagesLimit = -1, int crawlDelay = 1000);
    ~ClientSocket();  // Destructor to clean up resources like the socket

    // Start the discovery process by crawling the website and gathering statistics
    SiteStats startDiscovering();

private:
    string hostname;                 // The hostname or base URL of the website to be crawled
    int port;                        // The port to connect to (default is 80 for HTTP)
    int pagesLimit;                  // The maximum number of pages to crawl (default is no limit)
    int crawlDelay;                  // The delay between requests (in milliseconds)
    SOCKET sock;                     // The socket used for communication with the server

    LinkedList pendingPages;         // A linked list to keep track of pages to be crawled
    map<string, bool> discoveredPages;   // A map to store pages already discovered (to avoid revisiting)
    map<string, bool> discoveredLinkedSites; // A map to store external linked sites already discovered

    // Initializes Winsock (required for socket programming in Windows)
    bool initializeWinsock();

    // Creates a TCP socket for communication
    bool createSocket();

    // Connects to the host server using the created socket
    bool connectToHost();

    // Cleans up resources such as closing the socket and cleaning up Winsock
    void cleanup();

    // Creates a valid HTTP GET request string to fetch the specified path from the host
    string createHttpRequest(string host, string path);
};

#endif

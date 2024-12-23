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

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <map>
#include <vector>
#include "parser.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

using namespace std;

// Structure to store individual page statistics
struct PageStats {
    string url;
    double responseTime;
    PageStats(string u, double rt) : url(u), responseTime(rt) {}
};

// Struct to store statistics about a website
struct SiteStats {
    string hostname;                  // Hostname or base URL of the website being crawled
    double averageResponseTime = -1;  // The average response time for pages crawled
    double minResponseTime = -1;      // The minimum response time encountered
    double maxResponseTime = -1;      // The maximum response time encountered
    int numberOfPagesFailed = 0;      // Number of pages that failed to be discovered
    LinkedList linkedSites;           // A linked list to store linked sites
    LinkedList discoveredPages;       // A linked list to store discovered pages
    vector<PageStats> visitedPages;   // Vector to store visited pages with response times
};

class ClientSocket {
public:
    ClientSocket(string hostname, int port = 80, int pagesLimit = -1, int crawlDelay = 1000);
    ~ClientSocket();
    SiteStats startDiscovering();

private:
    string hostname;                  // The hostname or base URL of the website to be crawled
    int port;                        // The port to connect to (default is 80 for HTTP)
    int pagesLimit;                  // The maximum number of pages to crawl
    int crawlDelay;                  // The delay between requests (in milliseconds)
    SOCKET sock;                     // The socket used for communication with the server

    LinkedList pendingPages;         // A linked list to keep track of pages to be crawled
    map<string, bool> discoveredPages;   // A map to store pages already discovered
    map<string, bool> discoveredLinkedSites; // A map to store external linked sites

    bool initializeWinsock();
    bool createSocket();
    bool connectToHost();
    void cleanup();
    string createHttpRequest(string host, string path);
};

#endif
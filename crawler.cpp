/*
* ----------------------------------------------------------------------------
 *  Web Reaper Main Program
 * ----------------------------------------------------------------------------
 *  This program performs a multi-threaded web crawling operation, fetching
 *  pages from a set of starting URLs and extracting internal and external
 *  links. It implements RAII principles and modern C++ practices for robust
 *  resource management and thread safety.
 *
 *  Key Features:
 *  - Thread-safe operations using RAII and lock guards
 *  - Improved error handling and resource cleanup
 *  - Structured crawling process with clear state management
 *  - Comprehensive statistics tracking and reporting
 *  - Memory-efficient data structures and operations
 * ----------------------------------------------------------------------------
 */

#include "clientSocket.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <windows.h>
#include <iomanip>
#include <sstream>

using namespace std;

// Configuration structure with default values and validation
struct Config {
    int crawlDelay = 1000;        // Default delay between requests: 1 second
    int maxThreads = 10;          // Default maximum concurrent threads
    int depthLimit = 10;          // Default maximum crawling depth
    int pagesLimit = 10;          // Default maximum pages per site
    int linkedSitesLimit = 10;    // Default maximum linked sites to discover
    LinkedList startUrls;         // Starting URLs for crawling

    // Validates configuration values
    void validate() const {
        if (crawlDelay < 0) throw runtime_error("Crawl delay cannot be negative");
        if (maxThreads <= 0) throw runtime_error("Max threads must be positive");
        if (depthLimit < 0) throw runtime_error("Depth limit cannot be negative");
        if (pagesLimit < -1) throw runtime_error("Pages limit cannot be less than -1");
        if (linkedSitesLimit < 0) throw runtime_error("Linked sites limit cannot be negative");
        if (startUrls.empty()) throw runtime_error("No start URLs provided");
    }
};

// Crawler state management with thread safety
struct CrawlerState {
    int threadsCount{0};              // Active thread count
    LinkedList pendingSites;          // Sites waiting to be crawled
    map<string, bool> discoveredSites;// Track discovered sites
    mutex stateMutex;                 // Mutex for thread-safe state access
    condition_variable stateChanged;   // Condition for state changes
    bool isFinished{false};           // Indicates crawling completion
};

// RAII wrapper for thread management
class ThreadGuard {
    CrawlerState& state;
    mutex& mtx;
    condition_variable& cv;

public:
    ThreadGuard(CrawlerState& s, mutex& m, condition_variable& c)
        : state(s), mtx(m), cv(c) {}

    ~ThreadGuard() {
        lock_guard<mutex> lock(mtx);
        state.threadsCount--;
        cv.notify_all();
    }
};

// Global configuration and state
Config config;
CrawlerState crawlerState;

// Function declarations
Config readConfigFile();
void initialize();
void scheduleCrawlers();
void startCrawler(string hostname, int currentDepth);
void printCrawlingSummary(const SiteStats& stats, int depth);

int main() {
    try {
        SetConsoleOutputCP(CP_UTF8);  // Enable UTF-8 console output

        config = readConfigFile();
        config.validate();            // Validate configuration
        initialize();
        scheduleCrawlers();

        return 0;
    }
    catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
}

// Reads and parses configuration from file
Config readConfigFile() {
    ifstream cfFile("config.txt");
    if (!cfFile) {
        throw runtime_error("Cannot open config.txt");
    }

    Config cf;
    string var, val, url;

    while (cfFile >> var >> val) {
        if (var == "crawlDelay") cf.crawlDelay = stoi(val);
        else if (var == "maxThreads") cf.maxThreads = stoi(val);
        else if (var == "depthLimit") cf.depthLimit = stoi(val);
        else if (var == "pagesLimit") cf.pagesLimit = stoi(val);
        else if (var == "linkedSitesLimit") cf.linkedSitesLimit = stoi(val);
        else if (var == "startUrls") {
            int urlCount = stoi(val);
            for (int i = 0; i < urlCount; i++) {
                if (!(cfFile >> url)) {
                    throw runtime_error("Insufficient URLs in config file");
                }
                cf.startUrls.add(url, "");
            }
        }
    }

    return cf;
}

// Initializes crawler state with starting URLs
void initialize() {
    Node* urlNode = config.startUrls.getHead();
    while (urlNode) {
        string hostname = getHostnameFromUrl(urlNode->url);
        crawlerState.pendingSites.add(hostname, "0");
        crawlerState.discoveredSites[hostname] = true;
        urlNode = urlNode->next;
    }
}

// Manages crawler threads and scheduling
void scheduleCrawlers() {
    vector<thread> activeThreads;

    while (true) {
        unique_lock<mutex> lock(crawlerState.stateMutex);

        // Exit if all crawling is complete
        if (crawlerState.pendingSites.empty() && crawlerState.threadsCount == 0) {
            break;
        }

        // Start new threads if possible
        while (!crawlerState.pendingSites.empty() &&
               crawlerState.threadsCount < config.maxThreads) {
            string nextSite = crawlerState.pendingSites.front();
            int depth = stoi(crawlerState.pendingSites.getHead()->metadata);
            crawlerState.pendingSites.pop();

            crawlerState.threadsCount++;
            thread t(startCrawler, nextSite, depth);
            t.detach();
        }

        // Wait for state changes
        crawlerState.stateChanged.wait(lock, []{
            return crawlerState.pendingSites.empty() ||
                   crawlerState.threadsCount < config.maxThreads;
        });
    }
}

// Performs crawling of a single site
void startCrawler(string hostname, int currentDepth) {
    ThreadGuard guard(crawlerState, crawlerState.stateMutex, crawlerState.stateChanged);

    try {
        ClientSocket clientSocket(hostname, 80, config.pagesLimit, config.crawlDelay);
        SiteStats stats = clientSocket.startDiscovering();

        lock_guard<mutex> lock(crawlerState.stateMutex);
        printCrawlingSummary(stats, currentDepth);

        // Process discovered sites
        if (currentDepth < config.depthLimit) {
            size_t linkedCount = 0;
            Node* site = stats.linkedSites.getHead();

            while (site && linkedCount < static_cast<size_t>(config.linkedSitesLimit)) {
                if (!crawlerState.discoveredSites[site->url]) {
                    crawlerState.pendingSites.add(site->url, to_string(currentDepth + 1));
                    crawlerState.discoveredSites[site->url] = true;
                    linkedCount++;
                }
                site = site->next;
            }
        }

        crawlerState.stateChanged.notify_all();
    }
    catch (const exception& e) {
        lock_guard<mutex> lock(crawlerState.stateMutex);
        cerr << "Error crawling " << hostname << ": " << e.what() << endl;
        crawlerState.stateChanged.notify_all();
    }
}

// Formats and prints crawling statistics
void printCrawlingSummary(const SiteStats& stats, int depth) {
    stringstream ss;
    ss << fixed << setprecision(2);
    ss << "----------------------------------------------------------------------------\n"
       << "Website: " << stats.hostname << "\n"
       << "Depth: " << depth << "\n"
       << "Pages Discovered: " << (stats.discoveredPages.getHead() ? 1 : 0) << "\n"
       << "Pages Failed: " << stats.numberOfPagesFailed << "\n"
       << "Linked Sites: " << (stats.linkedSites.getHead() ? 1 : 0) << "\n"
       << "Response Times (ms) - "
       << "Min: " << stats.minResponseTime << ", "
       << "Max: " << stats.maxResponseTime << ", "
       << "Avg: " << stats.averageResponseTime << "\n";

    cout << ss.str();
}
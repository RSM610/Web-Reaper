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

struct Config {
    int crawlDelay = 1000;
    int maxThreads = 10;
    int depthLimit = 10;
    int pagesLimit = 10;
    int linkedSitesLimit = 10;
    LinkedList startUrls;

    void validate() const {
        if (crawlDelay < 0) throw runtime_error("Crawl delay cannot be negative");
        if (maxThreads <= 0) throw runtime_error("Max threads must be positive");
        if (depthLimit < 0) throw runtime_error("Depth limit cannot be negative");
        if (pagesLimit < -1) throw runtime_error("Pages limit cannot be less than -1");
        if (linkedSitesLimit < 0) throw runtime_error("Linked sites limit cannot be negative");
        if (startUrls.empty()) throw runtime_error("No start URLs provided");
    }
};

struct CrawlerState {
    int threadsCount{0};
    LinkedList pendingSites;
    map<string, bool> discoveredSites;
    mutex stateMutex;
    condition_variable stateChanged;
    bool isFinished{false};
};

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

Config config;
CrawlerState crawlerState;

// Updated printCrawlingSummary function to match desired output format
void printCrawlingSummary(const SiteStats& stats, int depth) {
    stringstream ss;
    ss << fixed << setprecision(3);

    // Basic site information
    ss << "Website: " << stats.hostname << "\n"
       << "Depth (distance from the starting pages): " << depth << "\n"
       << "Number of Pages Discovered: " << stats.visitedPages.size() << "\n"
       << "Number of Pages Failed to Discover: " << stats.numberOfPagesFailed << "\n"
       << "Number of Linked Sites: " << (stats.linkedSites.getHead() ? 1 : 0) << "\n"
       << "Min. Response Time: " << stats.minResponseTime << "ms\n"
       << "Max. Response Time: " << stats.maxResponseTime << "ms\n"
       << "Average Response Time: " << stats.averageResponseTime << "ms\n";

    // List of visited pages
    if (!stats.visitedPages.empty()) {
        ss << "List of visited pages:\n";
        ss << "Response Time\tURL\n";
        for (const auto& page : stats.visitedPages) {
            ss << page.responseTime << "ms\t" << page.url << "\n";
        }
    }

    cout << ss.str();
}

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

void initialize() {
    Node* urlNode = config.startUrls.getHead();
    while (urlNode) {
        string hostname = getHostnameFromUrl(urlNode->url);
        crawlerState.pendingSites.add(hostname, "0");
        crawlerState.discoveredSites[hostname] = true;
        urlNode = urlNode->next;
    }
}

void startCrawler(string hostname, int currentDepth) {
    ThreadGuard guard(crawlerState, crawlerState.stateMutex, crawlerState.stateChanged);

    try {
        ClientSocket clientSocket(hostname, 80, config.pagesLimit, config.crawlDelay);
        SiteStats stats = clientSocket.startDiscovering();

        lock_guard<mutex> lock(crawlerState.stateMutex);
        printCrawlingSummary(stats, currentDepth);

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

void scheduleCrawlers() {
    vector<thread> activeThreads;

    while (true) {
        unique_lock<mutex> lock(crawlerState.stateMutex);

        if (crawlerState.pendingSites.empty() && crawlerState.threadsCount == 0) {
            break;
        }

        while (!crawlerState.pendingSites.empty() &&
               crawlerState.threadsCount < config.maxThreads) {
            string nextSite = crawlerState.pendingSites.front();
            int depth = stoi(crawlerState.pendingSites.getHead()->metadata);
            crawlerState.pendingSites.pop();

            crawlerState.threadsCount++;
            thread t(startCrawler, nextSite, depth);
            t.detach();
        }

        crawlerState.stateChanged.wait(lock, []{
            return crawlerState.pendingSites.empty() ||
                   crawlerState.threadsCount < config.maxThreads;
        });
    }
}

int main() {
    try {
        SetConsoleOutputCP(CP_UTF8);

        config = readConfigFile();
        config.validate();
        initialize();
        scheduleCrawlers();

        return 0;
    }
    catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
}
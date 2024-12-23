//
//  main.cpp

#include "clientSocket.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <map>
#include <iomanip>
#include <condition_variable>

using namespace std;

//---------------------------------------------------------------------------
// Manually implemented Linked List for storing URLs and their associated depths
//---------------------------------------------------------------------------
struct SiteNode {
    string hostname;
    int depth;
    SiteNode* next;

    SiteNode(string host, int d) : hostname(host), depth(d), next(nullptr) {}
};

class SiteQueue {
private:
    SiteNode* head;
    SiteNode* tail;

public:
    SiteQueue() : head(nullptr), tail(nullptr) {}

    ~SiteQueue() {
        clear();
    }

    void push(string hostname, int depth) {
        SiteNode* newNode = new SiteNode(hostname, depth);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    pair<string, int> front() {
        if (!head) throw runtime_error("Queue is empty.");
        return {head->hostname, head->depth};
    }

    void pop() {
        if (!head) return;
        SiteNode* temp = head;
        head = head->next;
        delete temp;
        if (!head) tail = nullptr;
    }

    bool empty() const {
        return head == nullptr;
    }

    void clear() {
        while (!empty()) pop();
    }
};

//---------------------------------------------------------------------------
// Manually implemented Linked List for storing start URLs
//---------------------------------------------------------------------------
struct UrlNode {
    string url;
    UrlNode* next;

    UrlNode(string u) : url(u), next(nullptr) {}
};

class UrlList {
private:
    UrlNode* head;

public:
    UrlList() : head(nullptr) {}

    ~UrlList() {
        clear();
    }

    void add(string url) {
        UrlNode* newNode = new UrlNode(url);
        if (!head) {
            head = newNode;
        } else {
            UrlNode* temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newNode;
        }
    }

    UrlNode* getHead() const {
        return head;
    }

    void clear() {
        while (head) {
            UrlNode* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

// Config Struct with default settings.
typedef struct {
    int crawlDelay = 1000;
    int maxThreads = 10;
    int depthLimit = 10;
    int pagesLimit = 10;
    int linkedSitesLimit = 10;
    UrlList startUrls; // Replacing vector<string> with UrlList
} Config;

// CrawlerState for storing necessary info of the Crawler
struct CrawlerState {
    int threadsCount;
    SiteQueue pendingSites;             // Replacing queue with SiteQueue
    map<string, bool> discoveredSites;
};

// Variables
Config config;
CrawlerState crawlerState;
mutex m_mutex;
condition_variable m_condVar;
bool threadFinished;

Config readConfigFile();
void initialize();
void scheduleCrawlers();
void startCrawler(string baseUrl, int currentDepth, CrawlerState& crawlerState);

int main(int argc, const char* argv[]) {
    config = readConfigFile();
    initialize();
    scheduleCrawlers();
    return 0;
}

//***************************************************************************
// Functions' Implementation
//***************************************************************************

//---------------------------------------------------------------------------
// Read and Process the config file.
//---------------------------------------------------------------------------
Config readConfigFile() {
    try {
        ifstream cfFile("config.txt");
        string var, val, url;
        Config cf;
        while (cfFile >> var >> val) {
            if (var == "crawlDelay") cf.crawlDelay = stoi(val);
            else if (var == "maxThreads") cf.maxThreads = stoi(val);
            else if (var == "depthLimit") cf.depthLimit = stoi(val);
            else if (var == "pagesLimit") cf.pagesLimit = stoi(val);
            else if (var == "linkedSitesLimit") cf.linkedSitesLimit = stoi(val);
            else if (var == "startUrls") {
                for (int i = 0; i < stoi(val); i++) {
                    cfFile >> url;
                    cf.startUrls.add(url);
                }
            }
        }
        cfFile.close();
        return cf;
    } catch (exception& error) {
        cerr << "Exception (@readConfigFile): " << error.what() << endl;
        exit(1);
    }
}

//---------------------------------------------------------------------------
// Initialize the Crawler.
//---------------------------------------------------------------------------
void initialize() {
    // Set threads count to 0
    crawlerState.threadsCount = 0;

    // Add starting URLs
    UrlNode* urlNode = config.startUrls.getHead();
    while (urlNode) {
        string url = urlNode->url;
        crawlerState.pendingSites.push(getHostnameFromUrl(url), 0);
        crawlerState.discoveredSites[getHostnameFromUrl(url)] = true;
        urlNode = urlNode->next;
    }
}

//---------------------------------------------------------------------------
// Schedule crawlers in multithreads. Each thread discovers a specific host.
//---------------------------------------------------------------------------
void scheduleCrawlers() {
    while (crawlerState.threadsCount != 0 || !crawlerState.pendingSites.empty()) {
        m_mutex.lock();
        threadFinished = false;
        while (!crawlerState.pendingSites.empty() && crawlerState.threadsCount < config.maxThreads) {
            // Get the next site to fetch
            auto nextSite = crawlerState.pendingSites.front();
            crawlerState.pendingSites.pop();
            crawlerState.threadsCount++;

            // Start a new thread for crawling
            thread t = thread(startCrawler, nextSite.first, nextSite.second, ref(crawlerState));
            if (t.joinable()) t.detach();
        }
        m_mutex.unlock();

        // Wait for some thread to finish, then request a lock & try scheduling again
        unique_lock<mutex> m_lock(m_mutex);
        while (!threadFinished) m_condVar.wait(m_lock);
    }
}

//---------------------------------------------------------------------------
// Start a crawler to discover a specific website.
//---------------------------------------------------------------------------
void startCrawler(string hostname, int currentDepth, CrawlerState& crawlerState) {
    // Create socket, discover this website
    ClientSocket clientSocket = ClientSocket(hostname, 80, config.pagesLimit, config.crawlDelay);
    SiteStats stats = clientSocket.startDiscovering();

    // Output all statistics of the website
    m_mutex.lock();
    cout << "----------------------------------------------------------------------------" << endl;
    cout << "Website: " << stats.hostname << endl;
    cout << "Depth (distance from the starting pages): " << currentDepth << endl;
    cout << "Number of Pages Discovered: " << stats.discoveredPages.size() << endl;
    cout << "Number of Pages Failed to Discover: " << stats.numberOfPagesFailed << endl;
    cout << "Number of Linked Sites: " << stats.linkedSites.size() << endl;

    // Only discover more if depthLimit not reached
    if (currentDepth < config.depthLimit) {
        for (int i = 0; i < min(int(stats.linkedSites.size()), config.linkedSitesLimit); i++) {
            string site = stats.linkedSites[i];
            if (!crawlerState.discoveredSites[site]) {
                crawlerState.pendingSites.push(site, currentDepth + 1);
                crawlerState.discoveredSites[site] = true;
            }
        }
    }
    crawlerState.threadsCount--;
    threadFinished = true;
    m_mutex.unlock();

    // Notify the master thread
    m_condVar.notify_one();
}

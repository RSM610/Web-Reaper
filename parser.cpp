/*
 * ----------------------------------------------------------------------------
 *  Parser Functions for Web Reaper
 * ----------------------------------------------------------------------------
 *  These functions handle URL extraction, verification, and string processing
 *  for a web crawler. The main operations include:
 *  - Extracting URLs from an HTML response.
 *  - Validating URLs to ensure they are of the correct type and domain.
 *  - Handling LinkedList operations for storing discovered URLs.
 *
 *  Optimizations include:
 *  - Static string constants to reduce string creation
 *  - Efficient string operations
 *  - Improved memory management
 * ----------------------------------------------------------------------------
 */

#include "parser.h"
#include <map>
#include <stdexcept>
#include <array>

// LinkedList member function implementations
void LinkedList::add(string url, string metadata) {
    Node* newNode = new Node(url, metadata);
    if (!head) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
    size_++;
}

void LinkedList::clear() {
    while (head) {
        Node* temp = head;
        head = head->next;
        delete temp;
    }
    tail = nullptr;
    size_ = 0;
}

string LinkedList::front() const {
    if (!head) throw runtime_error("List is empty");
    return head->url;
}

void LinkedList::pop() {
    if (!head) return;
    Node* temp = head;
    head = head->next;
    if (!head) tail = nullptr;
    delete temp;
    size_--;
}

// URL Processing Functions
string getHostnameFromUrl(const string& url) {
    static const string https = "https://";
    static const string http = "http://";

    size_t offset = 0;
    if (url.compare(0, https.length(), https) == 0)
        offset = https.length();
    else if (url.compare(0, http.length(), http) == 0)
        offset = http.length();

    size_t pos = url.find('/', offset);
    return url.substr(offset, pos == string::npos ? string::npos : pos - offset);
}

string getHostPathFromUrl(const string& url) {
    static const string https = "https://";
    static const string http = "http://";

    size_t offset = 0;
    if (url.compare(0, https.length(), https) == 0)
        offset = https.length();
    else if (url.compare(0, http.length(), http) == 0)
        offset = http.length();

    size_t pos = url.find('/', offset);
    if (pos == string::npos) return "/";

    string path = url.substr(pos);
    pos = path.find_first_not_of('/');
    return pos == string::npos ? "/" : path.erase(0, pos - 1);
}

LinkedList extractUrls(const string& httpText) {
    string httpRaw = reformatHttpResponse(httpText);
    static const array<string, 4> urlStart = {"href=\"", "href = \"", "http://", "https://"};
    static const string urlEndChars = "\"#?, ";
    LinkedList extractedUrls;

    for (const auto& startText : urlStart) {
        size_t pos = 0;
        while ((pos = httpRaw.find(startText, pos)) != string::npos) {
            pos += startText.length();
            size_t endPos = httpRaw.find_first_of(urlEndChars, pos);
            if (endPos == string::npos) break;

            string url = httpRaw.substr(pos, endPos - pos);
            if (verifyUrl(url)) {
                extractedUrls.add(getHostnameFromUrl(url), getHostPathFromUrl(url));
            }
            pos = endPos;
        }
    }
    return extractedUrls;
}

bool verifyUrl(const string& url) {
    if (url.empty()) return false;
    string urlDomain = getHostnameFromUrl(url);
    if (urlDomain.empty() || !verifyDomain(urlDomain)) return false;
    if (!verifyType(url)) return false;
    if (url.find("mailto:") != string::npos) return false;
    return true;
}

bool verifyType(const string& url) {
    static const array<string, 7> forbiddenTypes = {
        ".css", ".js", ".pdf", ".png", ".jpeg", ".jpg", ".ico"
    };

    for (const auto& type : forbiddenTypes)
        if (url.find(type) != string::npos) return false;
    return true;
}

bool verifyDomain(const string& url) {
    static const array<string, 7> allowedDomains = {
        ".com", ".pk", ".edu", ".net", ".co", ".org", ".me"
    };

    for (const auto& domain : allowedDomains)
        if (hasSuffix(url, domain)) return true;
    return false;
}

bool hasSuffix(const string& str, const string& suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

string reformatHttpResponse(const string& text) {
    static const string allowedChrs = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.,/\":#?+-_= ";
    static const map<char, char> charMap = []() {
        map<char, char> m;
        for (char ch : allowedChrs) m[ch] = ch;
        m['\n'] = ' ';
        return m;
    }();

    string result;
    result.reserve(text.length());
    for (char ch : text) {
        auto it = charMap.find(ch);
        if (it != charMap.end()) {
            result += tolower(it->second);
        }
    }
    return result;
}
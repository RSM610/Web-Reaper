/*
* ----------------------------------------------------------------------------
 *  Parser Header - URL Processing and Data Structures
 * ----------------------------------------------------------------------------
 *  This header defines the core data structures and functions used for URL
 *  processing in the web crawler. It includes:
 *  - Queue template class for efficient FIFO operations
 *  - LinkedList class for URL storage and management
 *  - URL parsing and validation functions
 *
 *  The implementations focus on memory safety, efficiency, and proper resource
 *  management through RAII principles.
 * ----------------------------------------------------------------------------
 */

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <stdexcept>
#include <map>

using namespace std;

// Forward declarations
struct Node;
class LinkedList;
template<typename T> class Queue;

// ----------------------------------------------------------------------------
// Node structure for URL storage
// ----------------------------------------------------------------------------
struct Node {
    string url;        // URL string
    string metadata;   // Metadata associated with the URL
    Node* next;        // Pointer to the next node in the list

    // Constructor to initialize the URL, metadata, and set the next pointer to nullptr
    Node(string u, string m) : url(u), metadata(m), next(nullptr) {}
};

// ----------------------------------------------------------------------------
// Queue template class for efficient FIFO operations
// ----------------------------------------------------------------------------
template<typename T>
class Queue {
private:
    // Internal node structure for the queue
    struct QNode {
        T data;
        QNode* next;
        QNode(const T& value) : data(value), next(nullptr) {}
    };
    
    QNode* front;       // Pointer to front of queue
    QNode* rear;        // Pointer to rear of queue
    size_t size_;       // Current size of queue

public:
    // Constructor initializes an empty queue
    Queue() : front(nullptr), rear(nullptr), size_(0) {}
    
    // Deep copy constructor ensures proper resource management
    Queue(const Queue& other) : front(nullptr), rear(nullptr), size_(0) {
        QNode* curr = other.front;
        while (curr) {
            push(curr->data);
            curr = curr->next;
        }
    }
    
    // Assignment operator with proper cleanup and deep copy
    Queue& operator=(const Queue& other) {
        if (this != &other) {
            // Clear existing queue
            while (!empty()) {
                pop();
            }
            
            // Copy from other queue
            QNode* curr = other.front;
            while (curr) {
                push(curr->data);
                curr = curr->next;
            }
        }
        return *this;
    }
    
    // Destructor ensures proper cleanup
    ~Queue() {
        while (!empty()) {
            pop();
        }
    }
    
    // Adds an element to the rear of the queue
    void push(const T& value) {
        QNode* newNode = new QNode(value);
        if (empty()) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        size_++;
    }
    
    // Removes the front element from the queue
    void pop() {
        if (empty()) return;
        
        QNode* temp = front;
        front = front->next;
        if (!front) rear = nullptr;
        delete temp;
        size_--;
    }
    
    // Returns reference to front element
    T& peek() {
        if (empty()) throw std::runtime_error("Queue is empty");
        return front->data;
    }
    
    // Returns const reference to front element
    const T& peek() const {
        if (empty()) throw std::runtime_error("Queue is empty");
        return front->data;
    }
    
    // Checks if queue is empty
    bool empty() const {
        return front == nullptr;
    }
    
    // Returns current size of queue
    size_t size() const {
        return size_;
    }
};

// ----------------------------------------------------------------------------
// LinkedList class: A unified linked list for managing URLs and metadata
// ----------------------------------------------------------------------------
class LinkedList {
private:
    Node* head;        // Pointer to the first node in the list
    Node* tail;        // Pointer to the last node in the list
    size_t size_;      // Current size of the list

public:
    // Constructor initializes an empty list
    LinkedList() : head(nullptr), tail(nullptr), size_(0) {}
    
    // Deep copy constructor
    LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr), size_(0) {
        Node* curr = other.head;
        while (curr) {
            add(curr->url, curr->metadata);
            curr = curr->next;
        }
    }
    
    // Assignment operator
    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            clear();  // Clear existing list
            Node* curr = other.head;
            while (curr) {
                add(curr->url, curr->metadata);
                curr = curr->next;
            }
        }
        return *this;
    }
    
    // Destructor ensures proper cleanup
    ~LinkedList() { clear(); }
    
    // Adds a new URL with metadata to the list
    void add(string url, string metadata);
    
    // Clears the entire list
    void clear();
    
    // Checks if the list is empty
    bool empty() const { return head == nullptr; }
    
    // Returns pointer to head node
    Node* getHead() const { return head; }
    
    // Returns URL of first node
    string front() const;
    
    // Removes first node from list
    void pop();
    
    // Returns current size of list
    size_t size() const { return size_; }
};

// ----------------------------------------------------------------------------
// Function Declarations for URL Processing
// ----------------------------------------------------------------------------

// Extracts hostname from URL (e.g., "http://example.com/path" -> "example.com")
string getHostnameFromUrl(const string& url);

// Extracts path from URL (e.g., "http://example.com/path" -> "/path")
string getHostPathFromUrl(const string& url);

// Extracts valid URLs from HTTP response text
LinkedList extractUrls(const string& httpText);

// Validates URL based on domain and type
bool verifyUrl(const string& url);

// Verifies domain is in allowed list
bool verifyDomain(const string& url);

// Verifies URL is not of a forbidden type
bool verifyType(const string& url);

// Checks if string ends with given suffix
bool hasSuffix(const string& str, const string& suffix);

// Reformats HTTP response text for processing
string reformatHttpResponse(const string& text);

#endif
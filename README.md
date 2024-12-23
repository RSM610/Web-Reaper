# Web Reaper: Harvesting Digital Footprints 🕷️

![Security](https://img.shields.io/badge/Security-FF0000?style=for-the-badge&logo=security&logoColor=white)
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)
![DSA](https://img.shields.io/badge/DSA-4B275F?style=for-the-badge&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAYAAAAfSC3RAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAGwSURBVDhPY4CB////M+zdu3cnEwMDQx4QCwHxL0awKBDMnz9/5p49e+YD2X+B+D8LED948OBGoOqJQMwqICCgDcRg8OPHD7BCoHqQGhEgtgDiX0yPHj36ClSQAcRMQAwCQUFBDGFhYQwgNhAzA7E4EP9kef78+Xeg4BQgZgViEAAqBmFmIH6rhyqk6/9/Bm4gWxyIfzEzMzMzABXmAjEzEIPAly9fGF6/fg3WYGH4vziba0o/F4jNDMQ/gG4UUVdX5wkPD2cICQlhCA0NZfDz82PQ0dH5v3PrH4bS1l0MIiIiYMwKxL+ZgQpFent7/8+aNYtBQECAQVxcnEFWVhasm4mJiaGzs5NBUVERrBuIf7K8e/fuN1DRBCBmBWIwADkRaN0fIGYASgHxH6Bt74CGpQMxOxCDAchy2Y0s/A6lv1n+MPxjAbqHBYhBgAVK/2BiYmL4+vUrw7t37xi+f//O8OPHDzD49esXw+/fv8HyIPwbiH8wA+3/BMTlQPwTiEGKQeA3EP8B4t9AzADE/6qq/jE8efL0DxD/Q8KM//+zMICcAgBZUqaJHnpTFgAAAABJRU5ErkJggg==)

A sophisticated web crawling system developed as part of our Data Structures and Algorithms course, implementing efficient data structures and algorithms for web crawling and analysis.

## Project Structure

The project consists of the following core files:

```plaintext
├── clientSocket.cpp/h   # Network communication and crawling logic
├── parser.cpp/h         # URL processing and data structures
├── crawler.cpp          # Main program and thread management
├── Makefile            # Build configuration
└── config.txt          # Runtime configuration
```

## Data Structures & Algorithms Implementation

Our project showcases various DSA concepts:

### Data Structures
1. **Custom LinkedList Implementation**
    - Efficient node-based storage for URLs and metadata
    - RAII principles for memory management
    - Thread-safe operations

2. **Generic Queue Template**
    - FIFO operations for crawling queue
    - Custom iterator implementation
    - Exception-safe design

3. **Hash Maps**
    - O(1) lookup for discovered pages
    - Efficient URL deduplication
    - Memory-optimized storage

### Algorithms
1. **Graph Traversal**
    - Breadth-first crawling strategy
    - Depth-limited exploration
    - Cycle detection for loops

2. **String Processing**
    - Efficient URL parsing
    - Pattern matching for link extraction
    - Domain validation

3. **Threading Algorithm**
    - Work-stealing queue implementation
    - Load balancing across threads
    - Resource pooling

## Robust Design Features

### Memory Management
- RAII principles throughout
- Smart pointer usage
- Proper cleanup in destructors
- Exception-safe resource handling

### Thread Safety
- Mutex-protected shared resources
- Lock guards for exception safety
- Atomic operations where appropriate
- Condition variables for synchronization

### Error Handling
- Comprehensive exception hierarchy
- Resource cleanup on errors
- Graceful degradation
- Detailed error reporting

## Team Members

Our dedicated development team consists of:

- **Momna Jamil**  - Lead Developer
- **Rida Shahid Malik**  - Testing Specialist
- **Muhammad Sofia Faisal**  - Documentation Lead

## Build & Run

1. Clone the repository
2. Ensure MinGW with G++ is installed
3. Build using make:
```bash
mingw32-make clean
mingw32-make
```

## Configuration

Example `config.txt`:
```plaintext
crawlDelay 1000
maxThreads 10
depthLimit 3
pagesLimit 10
linkedSitesLimit 5
startUrls 1
http://example.com
```

## License

This project is licensed under the MIT License.

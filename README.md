# Web Reaper: Harvesting Digital Footprints 🕷️

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)
![Security](https://img.shields.io/badge/Security-FF0000?style=for-the-badge&logo=security&logoColor=white)

A sophisticated web crawling system designed specifically for cybersecurity applications, implementing efficient data structures and algorithms for threat intelligence gathering, vulnerability assessment, and security monitoring.

## Team Members 

Our dedicated development team consists of:

- **Momna Jamil** (2023336) - Lead Developer
- **Rida Shahid Malik** (2023610) - Testing Specialist
- **Muhammad Sofia Faisal** (2023519) - Documentation Lead

## Features 

Web Reaper comes packed with powerful features designed for cybersecurity professionals:

- Multi-threaded architecture for efficient crawling
- Real-time response analysis and anomaly detection
- Configurable crawling parameters and depth limits
- Robust error handling and recovery mechanisms
- Thread-safe data structures for concurrent operations

## System Requirements 

### Hardware Requirements

- Processor: Multi-core processor (Intel i5 or equivalent)
- RAM: Minimum 8GB
- Storage: 1GB free space
- Network: Stable internet connection

### Software Requirements

- Windows 10/11
- MinGW with G++ (C++11 support)
- Make build system
- WinSock2 library

## Installation 🔧

Follow these steps to get Web Reaper up and running:

1. Clone the repository:
```bash
git clone https://github.com/RSM610/web-reaper.git
cd web-reaper
```

2. Install dependencies:
```bash
# Install MinGW with G++
# Add MinGW to system PATH
# Install Make for Windows
```

3. Build the project:
```bash
mingw32-make clean
mingw32-make
```
```plaintext
or 
just click on the:

webreaper.exe 
```

## Configuration 

Create a `config.txt` file in the project root with the following parameters:

```plaintext
crawlDelay 1000
maxThreads 10
depthLimit 3
pagesLimit 10
linkedSitesLimit 5
startUrls 1
http://example.com
```

## Architecture 

Web Reaper is built on three core components:

### 1. ClientSocket Module
Handles all network communication including socket management, HTTP requests, and response processing.

### 2. Parser Module
Manages data extraction and processing, including URL validation and content analysis.

### 3. Crawler Module
Coordinates the crawling process with multi-threaded operations and resource management.

## Security Applications 

Web Reaper supports various cybersecurity applications:

### Threat Intelligence Collection
- Continuous monitoring of security forums
- Automated data collection from multiple sources
- Pattern recognition for emerging threats

### Vulnerability Assessment
- Discovery of exposed sensitive files
- Detection of server misconfigurations
- Identification of outdated software versions

### Incident Response Support
- Rapid data gathering during security incidents
- Automated evidence collection
- Timeline reconstruction capabilities

## Usage Examples 

Basic usage to start crawling:

```cpp
// Initialize the crawler
ClientSocket crawler("example.com", 80, 100, 1000);

// Start the discovery process
SiteStats stats = crawler.startDiscovering();

// Access the results
cout << "Pages discovered: " << stats.discoveredPages.size() << endl;
cout << "Average response time: " << stats.averageResponseTime << "ms" << endl;
```

## Testing 

The system includes comprehensive testing:

- Unit tests for all core components
- Integration tests for module interactions
- Performance testing under various loads
- Security testing for common vulnerabilities

## Future Enhancements 

We plan to implement the following features:

- Integration with popular threat intelligence platforms
- Enhanced pattern recognition using machine learning
- Automated report generation
- Advanced visualization of crawl results
- API integration capabilities


## License 

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments 

Special thanks to:
- Our course instructor for guidance and support
- The open-source community for valuable resources
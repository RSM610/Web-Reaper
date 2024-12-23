# Compiler settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -lws2_32

# Source files
SOURCES = crawler.cpp clientSocket.cpp parser.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Output executable
TARGET = webreaper.exe

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	del /F /Q $(OBJECTS) $(TARGET)

.PHONY: all clean
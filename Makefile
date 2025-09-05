# Makefile for Educational Triggerbot

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pedantic
LDFLAGS = -lgdi32 -luser32

# Directories
SRC_DIR = .
BUILD_DIR = build
BIN_DIR = bin

# Target executable
TARGET = triggerbot
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Create directories if they don't exist
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

# Build the main executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $(BIN_DIR)/$(TARGET).exe $(LDFLAGS)
	@echo "Build complete: $(BIN_DIR)/$(TARGET).exe"

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	del /Q *.o 2>nul || true
	del /Q $(BIN_DIR)\*.exe 2>nul || true
	rmdir $(BIN_DIR) 2>nul || true

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Release build with optimizations
release: CXXFLAGS += -O3 -DNDEBUG
release: $(TARGET)

# Install (copy to a common location)
install: $(TARGET)
	copy $(BIN_DIR)\$(TARGET).exe C:\Tools\ 2>nul || echo "Install failed - check permissions"

# Run the program
run: $(TARGET)
	$(BIN_DIR)\$(TARGET).exe

# Show help
help:
	@echo "Available targets:"
	@echo "  all     - Build the triggerbot (default)"
	@echo "  debug   - Build with debug symbols"
	@echo "  release - Build optimized release version"
	@echo "  clean   - Remove build artifacts"
	@echo "  run     - Build and run the program"
	@echo "  install - Install to C:\Tools\"
	@echo "  help    - Show this help message"

.PHONY: all clean debug release install run help

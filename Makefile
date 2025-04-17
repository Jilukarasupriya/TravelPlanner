CXX = g++
CXXFLAGS = -std=c++17 -O2
TARGET = travelPlanner.exe

all: $(TARGET)

$(TARGET): travelPlanner.cpp
	$(CXX) $(CXXFLAGS) $< -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean

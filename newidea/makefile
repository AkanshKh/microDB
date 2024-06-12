CXX = g++

CXXFLAGS =  -std=c++11

TARGET = tot

SRCS = main.cpp start.cpp

OBJS = main.o start.o

all: $(TARGET)
	@echo "Executed"

$(TARGET): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

start.o: start.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) $(TARGET)
	@rm -f heh.txt
	@echo "Cleared"

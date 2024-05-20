CXX = g++

CXXFLAGS =  -std=c++11

TARGET = tot

SRCS = main.cpp includes/containers.cpp

OBJS = main.o includes/containers.o

all: $(TARGET)
	@echo "Executed"

$(TARGET): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

includes/containers.o: includes/containers.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) $(TARGET)
	@echo "Cleared"

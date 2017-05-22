CXX=g++
CXXFLAGS=-g -std=c++11 -lpthread

SRCS=server.cpp client.cpp
main: $(SRCS)
	$(CXX) $(CXXFLAGS) client.cpp -o client
	$(CXX) $(CXXFLAGS) server.cpp -o server

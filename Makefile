CXX=g++
CXXFLAGS=-std=c++11
LDFLAGS=-pthread

all: web-server web-client

web-server: httpconst.o httpmsg.o util.o httpserver.o server.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) httpconst.o httpmsg.o util.o httpserver.o server.o -o web-server

web-client: httpconst.o httpmsg.o util.o httpclient.o client.o
	$(CXX) $(CXXFLAGS) httpconst.o httpmsg.o util.o httpclient.o client.o -o web-client

httpconst.o: src/httpconst.cpp src/httpconst.h
	$(CXX) $(CXXFLAGS) -c src/httpconst.cpp

httpmsg.o: src/httpmsg.cpp src/httpmsg.h
	$(CXX) $(CXXFLAGS) -c src/httpmsg.cpp

util.o: src/util.cpp src/util.h
	$(CXX) $(CXXFLAGS) -c src/util.cpp

httpserver.o: src/httpserver.cpp src/httpserver.h
	$(CXX) $(CXXFLAGS) -c src/httpserver.cpp

server.o: src/server.cpp
	$(CXX) $(CXXFLAGS) -c src/server.cpp

httpclient.o: src/httpclient.cpp src/httpclient.h
	$(CXX) $(CXXFLAGS) -c src/httpclient.cpp

client.o: src/client.cpp
	$(CXX) $(CXXFLAGS) -c src/client.cpp

dist:
	tar cvzf proj1-zhanyang.tar.gz src/ Makefile README.md

clean:
	rm *.o web-client web-server
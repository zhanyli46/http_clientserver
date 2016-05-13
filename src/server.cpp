#include <iostream>
#include <signal.h>
#include "httpserver.h"

#define NWORKTHRD 16

static volatile void *SERV;

void usage()
{
	std::cout << "Usage: ./web-server [hostname] [port] [file-dir]\n";
}


// This handler is really, really ugly; I really want to replace it if possible
void sigintHandler(int signal)
{
	// The user wants to quits
	HttpServer serv = *(HttpServer *)SERV;
	serv.terminate();
	exit(0);
}


int main(int argc, char** argv)
{	
	int status = 0;
	signal(SIGINT, sigintHandler);
	HttpServer serv;

	if (argc != 4) {
		usage();
		return 1;
	}
 
	// initialize a new HTTP server
	SERV = (void *)&serv;
	if ((status = serv.init(argv[1], argv[2], argv[3])) < 0) {
		if (status == -1)
			std::cerr << "Init error: cannot create socket\n";
		else if (status == -2)
			std::cerr << "Init error: cannot bind to socket\n";
		else
			std::cerr << "Init error: cannot listen to socket\n";
		return 1;
	}
	
	if (serv.createThreads(NWORKTHRD) < 0) {
		std::cerr << "Thread error: cannot create threads\n";
		return 1;
	}

	// infinite loop to handle incoming requests
	
	while (true) {
		if (serv.handleRequests() < 0)
			std::cerr << "Thread error: cannot handle request" << std::endl;
	}

	return 0;
}

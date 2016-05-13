#ifndef _SERVER_H_
#define _SERVER_H_

#include "util.h"
#include "httpmsg.h"
#include <thread>

static const std::string SERVNAME = "cs118-simpleserver";
static const unsigned NTHREADMAX = 512;
static const int NRETRY = 10;
static const size_t BUFSIZE = 1024;

enum { T_RUNNABLE, T_NEWTASK, T_WORKING, T_SHUTDOWN };

class HttpServer {
	int sockfd;
	sockaddr_in *address;
	socklen_t address_len;
	std::thread *threads;
	unsigned nthrds;
	int *thrdstat;
	int *workfd;
	int *persistent;

	void serveRequest(int i);
	void collectThreads();
	int setTitle(std::string title, HttpRequest *req);
	int setHeaders(std::string headers, HttpRequest *req);
	int decomposeRequest(std::string msg, HttpRequest *req);
	HttpResponse createResponse(HttpRequest *req);
	HttpResponse errorResponse(int errcode, std::string version);
	
 public:
	HttpServer();
	int getSockfd() { return sockfd; }
	int init(char *host, char *port, char *filedir);
	int createThreads(unsigned nthreads);
	int handleRequests();
	void terminate();
};



#endif

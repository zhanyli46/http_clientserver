#ifndef _HTTPCLIENT_H_
#define _HTTPCLIENT_H_

#include "httpmsg.h"
#include "util.h"

static const std::string CLIENTNAME = "cs118-simpleclient";
static const size_t BUFSIZE = 2048;
static const double TIMEOUT = 5.0;


class HttpClient {
	std::string scheme;
	std::string hostname;
	std::string filedir;
	std::string ip;
	std::string port;
	int parsePath(std::string filepath, std::string *hostname, 
				  std::string *port, std::string *filedir);
	HttpRequest createHttpRequest();

public:
	std::string getHostName() { return hostname; }
	std::string getIp() { return ip; }
	std::string getFiledir() { return filedir; }
	int init(char *url);
	int requestWebpage(std::string *msgres);
	int interpretResponse(std::string *response);

};

#endif
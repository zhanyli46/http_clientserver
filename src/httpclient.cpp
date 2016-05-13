#include "httpclient.h"
#include <chrono>
#include <iostream>
#include "httpconst.h"

// Private member functions
int HttpClient::parsePath(std::string filepath, std::string *hostname, 
						  std::string *port, std::string *filedir)
{
	// find hostname and port from url
	int slash = filepath.find("/");
	int colon = filepath.find(":");
	int pathlen = filepath.length();
	if (slash == std::string::npos) {
		if (colon == std::string::npos) {
			// filepath contains only hostname
			*hostname = filepath;
			*port = "";
			*filedir = "/";
		} else {
			// filepath contains hostname and filedir
			*hostname = filepath.substr(0, colon);
			*port = filepath.substr(colon + 1, pathlen - colon - 1);
			*filedir = "/";
		}
	} else {
		if (colon == std::string::npos) {
			// filepath contains hostname and filedir
			*hostname = filepath.substr(0, slash);
			*port = "";
			*filedir = filepath.substr(slash, pathlen - slash);
		} else {
			// filepath contains hostname, port and filedir
			*hostname = filepath.substr(0, colon);
			*port = filepath.substr(colon + 1, slash - hostname->length() - 1);
			*filedir = filepath.substr(slash, pathlen - slash);
		}
	}

	// check validity
	if (*port != "" && !isNumeric(*port)) {
		// invalid port number
		return -1;
	}
	return 0;
}

HttpRequest HttpClient::createHttpRequest()
{
	// default: HTTP/1.0 + GET
	HttpRequest req;
	req.headers["User-agent"] = CLIENTNAME;
	req.setMethod("GET");
	req.setFiledir(filedir);
	req.setVersion("HTTP/1.0");
	req.headers["Connection"] = (req.getVersion() == "HTTP/1.1") ? "keep-alive" : "close";
	req.headers["Host"] = hostname;
	req.setMsgBody("");
	return req;
}

// Public member functions
int HttpClient::init(char *url)
{
	std::string surl = url;
	int index;
	
	// find scheme from url
	index = surl.find("://");
	if (index == std::string::npos) {
		// cannot find scheme
		return -1;
	}	
	scheme = surl.substr(0, index);
	surl = surl.substr(index + 3, surl.length() - 3);

	if (parsePath(surl, &hostname, &port, &filedir) < 0) {
		// file path invalid
		return -2;
	}

	std::string defport = defaultPort(scheme);
	if (defport == "")
		// unrecognized scheme
		return -3;
	if (port == "")
		port = defport;
	
	return 0;
}

int HttpClient::requestWebpage(std::string *response)
{
	struct hostent *hent;
	hent = gethostbyname(hostname.c_str());
	if (hent == NULL) {
		// cannot resolve host
		return -1;
	}

	struct in_addr *in = (struct in_addr *)hent->h_addr_list[0];
	ip = inet_ntoa(*in);

	// create socket
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		//cannot create socket
		return -2;
	}

	// connect to server host
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(atoi(port.c_str()));
	servAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	memset(servAddr.sin_zero, 0, sizeof(servAddr.sin_zero));
	socklen_t addrLen = sizeof(servAddr);

	if (connect(sockfd, (struct sockaddr *)&servAddr, addrLen) < 0) {
		// cannot connect to server
		return -3;
	}

	// send Http request to server
	HttpRequest req = createHttpRequest();
	std::string msgreq = req.toString();
	const char* outbuf = msgreq.c_str();
	size_t buflen = msgreq.length();
	ssize_t bytesSend;
	if ((bytesSend = send(sockfd, outbuf, buflen, 0)) < 0) {
		return -4;
	}

	// receive HTTP response
	char inbuf[BUFSIZE];
	std::string s;
	ssize_t bytesReceived;
	auto start = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed;

	while (true) {
		if (recv(sockfd, inbuf, BUFSIZE, MSG_PEEK) == 0) {
			// data not arrived yet; check timeout
			auto now = std::chrono::steady_clock::now();
			elapsed = now - start;
			if (elapsed.count() > TIMEOUT) {
				return -6;
			}
		}
		memset(inbuf, 0, BUFSIZE);
		if ((bytesReceived = recv(sockfd, inbuf, BUFSIZE, 0)) > 0) {
			// append received data
			s.append(inbuf, bytesReceived);
		} else if (bytesReceived == 0) {
			// nothing received
			break;
		} else {
			// error
			return -5;
		}
	}

	*response = s;
	return 0;
}

int HttpClient::interpretResponse(std::string *response)
{
	int firstSpace = response->find(" ");
	int secondSpace = response->find(" ", firstSpace + 1);
	std::string code = response->substr(firstSpace + 1, secondSpace - firstSpace - 1);
	if (code == "200") {
		int msgIndex = response->find("\r\n\r\n");
		*response = response->substr(msgIndex + 4, response->length() - msgIndex - 4);
	} else {
		*response = code;
		return -1;
	}
	return 0;
}
#include "httpserver.h"
#include "httpconst.h"
#include <fstream>
#include <sstream>
#include <time.h>


/* Private member functions */

void HttpServer::serveRequest(int tid) {

	while (true) {
		if (thrdstat[tid] == T_SHUTDOWN)
			return;
		if (thrdstat[tid] == T_NEWTASK) {
			thrdstat[tid] = T_WORKING;

			//handle individual request
			std::stringstream ss;
			char inbuf[BUFSIZE];
			if (recv(workfd[tid], inbuf, sizeof(inbuf), 0) == -1) {
				// cannot receive data from the client; close connection
				shutdown(workfd[tid], SHUT_RDWR);
				close(workfd[tid]);
				workfd[tid] = -1;
				thrdstat[tid] = T_RUNNABLE;
				continue;
			}

			ss << inbuf;
			std::string msg = ss.str();

			HttpRequest req;
			HttpResponse res;

			// create HttpResponse from the HttpRequest
			if (decomposeRequest(msg, &req) < 0) {
				res = errorResponse(STAT_BADREQ, req.getVersion());
			} else {
				res = createResponse(&req);
			}

			// create string ready for data transmission
			msg = res.toString();
			const char *outbuf = msg.c_str();
			size_t buflen = msg.length();

			ssize_t bytesSent;
			int count = 0;
			do {
				if ((bytesSent = send(workfd[tid], outbuf, buflen, 0)) < 0) {
					count++;
					sleep(1);
				} else {
					// message successfully sent, prepare to exit
					break;
				}
			}
			while (count < NRETRY);

			if (res.headers["Connection"] == "keep-alive") {
				// add current socket to saved sockets and wait for next request
				
			} else {
				// invalidate socket and reset working thread status
				shutdown(workfd[tid], SHUT_RDWR);
				close(workfd[tid]);
				workfd[tid] = -1;
				thrdstat[tid] = T_RUNNABLE;
			}
		}
	}
}

void HttpServer::collectThreads()
{
	int i;
	for (i = 0; i < nthrds; i++) {
		thrdstat[i] = T_SHUTDOWN;
	}
	for (i = 0; i < nthrds; i++) {
		threads[i].join();
	}
}

int HttpServer::setTitle(std::string title, HttpRequest *req)
{
	std::size_t index;

	// set method
	index = title.find(" ");
	if (index == std::string::npos)
		return -STAT_BADREQ;
	req->setMethod(title.substr(0, index));
	title = title.substr(index + 1, title.length() - index - 1);

	// set url
	index = title.find(" ");
	if (index == std::string::npos)
		return -STAT_BADREQ;
	req->setFiledir(title.substr(0, index));

	// set version
	req->setVersion(title.substr(index + 1, title.length() - index - 1));	

	return 0;
}

int HttpServer::setHeaders(std::string headers, HttpRequest *req)
{
	std::size_t	index, subind;
	std::string header;
	std::string key, val;
	int strlen = headers.length();

	// while (true)
	while (index = headers.find("\r\n")) {
		// no more header; break
		if (index == std::string::npos)
			break;

		// handle header once in each loop
		header = headers.substr(0, index);
		strlen -= index + 2;
		headers = headers.substr(index + 2, strlen);
		
		subind = header.find(": ");
		if (subind == std::string::npos)
			return -STAT_BADREQ;
		key = header.substr(0, subind);
		val = header.substr(subind + 2, header.length());
		req->headers[key] = val;
	}

	return 0;
}

int HttpServer::decomposeRequest(std::string msg, HttpRequest *req)
{

	std::size_t index;
	std::string str;
	int msglen = msg.length();

	// separate the HTTP title
	index = msg.find("\r\n");
	if (index == std::string::npos)
		return -STAT_BADREQ;
	str = msg.substr(0, index);
	
	// get the title fields
	if (setTitle(str, req) < 0) {
		// error dealing with title
		return -STAT_BADREQ;
	}

	// seperate the HTTP message headers
	msg = msg.substr(index + 2, msglen - index - 2);
	msglen = msg.length();
	index = msg.find("\r\n\r\n");
	if (index == std::string::npos)
		return -STAT_BADREQ;
	index += 2;
	str = msg.substr(0, index);

	// get all headers
	if (setHeaders(str, req) < 0) {
		// error setting headers
		return -STAT_BADREQ;
	}

	// get HTTP message body
	str = msg.substr(index + 2, msglen - index - 2);
	req->setMsgBody(str);
	return 0;
}

HttpResponse HttpServer::createResponse(HttpRequest *req)
{
	HttpResponse res;
	int fd;

	// set server name and response date/time 
	res.headers["Server"] = SERVNAME;
	char buf[1000];
	time_t now = time(0);
	struct tm time = *gmtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &time);
	std::string date = buf;
	res.headers["Date"] = date;

	if (req->getMethod() == "GET") {
		if (req->getVersion() == "HTTP/1.1") {
			// if client wants to use HTTP/1.1, then the Host header must exist
			if (req->headers["Host"] == "")
				return errorResponse(STAT_BADREQ, req->getVersion());

			res.headers["Connection"] = "keep-alive";
		} else {
			res.headers["Connection"] = "close";
		}

		res.setVersion(req->getVersion());

		// retrieve the absolute file path for the requested file.
		// if the url given is a directory, the "index.html" file
		// 	under that directory will be chosen instead.
		std::string fname = getpwd() + req->getFiledir();
		if (req->getFiledir() == "/")
			fname += "index.html";
		std::ifstream ifs;
		struct stat fileinfo;
		ifs.open(fname, std::ifstream::in);
		// try to open the file
		if (!ifs.is_open()) {
			// file not found; error
			return errorResponse(STAT_NOTFOUND, req->getVersion());
		} else {
			// file found in curdir, set code and phrase
			res.setCode(std::to_string(STAT_OK));
			res.setPhrase(toPhrase(STAT_OK));
			// record file metadata (normally succeed)
			if (stat(fname.c_str(), &fileinfo) != 0) {
				return errorResponse(STAT_SERVERR, req->getVersion());
			}
			// find last modified time
			time_t mtime = fileinfo.st_mtime;
			time = *gmtime(&mtime);
			memset(buf, 0, sizeof(buf));
			strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &time);
			std::string lmtime = buf;
			res.headers["Last-Modified"] = lmtime;
			// copy file content in to HTTP message body and add 
			// corresponding headers
			std::stringstream ss;
			ss << ifs.rdbuf();
			std::string msg = ss.str();
			res.setMsgBody(msg);
			// set the file size and MIME type of requested file
			res.headers["Content-Length"] = std::to_string(fileinfo.st_size);
			int dot = fname.rfind(".");
			std::string ext;
			if (dot == std::string::npos) {
				// unknown file extension
				ext = ".unknown";
			} else {
				ext = fname.substr(dot, fname.length() - dot);
			}
			res.headers["Content-Type"] = findMimeFromExt(ext);
		}

	} else {
		// UNIMPLEMENTED: HEAD, POST, DELETE and others
	}
	return res;
}

HttpResponse HttpServer::errorResponse(int errcode, std::string version)
{
	HttpResponse res;
	res.setVersion(version);
	res.setCode(std::to_string(errcode));
	res.setPhrase(toPhrase(errcode));
	res.headers["Server"] = SERVNAME;

	char buf[1000];
	time_t now = time(0);
	struct tm time = *gmtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &time);
	std::string date = buf;
	res.headers["Date"] = date;

	return res;
}


/* Public member functions */

HttpServer::HttpServer()
{
	sockfd = -1;
	threads = nullptr;
	thrdstat = nullptr;
	workfd = nullptr;
	persistent = nullptr;
	address = new sockaddr_in();
	address_len = sizeof(*address);
	memset(address, 0, address_len);
}


int HttpServer::init(char *host, char *port, char *filedir)
{
	int r = 0;
	// create empty socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		r = -1;

	// bind socket with localhost ip and port number
	address->sin_family = AF_INET;
	address->sin_port = htons(4000);
	address->sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(address->sin_zero, 0, 8);
	struct sockaddr *addr = (struct sockaddr *)address;
	if (bind(sockfd, addr, address_len) == -1)
		r = -2;

	// listen to the socket
	if (listen(sockfd, 0) == -1)
		r = -3;
	
	return r;
}

int HttpServer::createThreads(unsigned nthreads)
{
	int i;
	
	if (nthreads <= 0 || nthreads > NTHREADMAX)
		return -1;

	nthrds = nthreads;
	threads = new std::thread[nthrds];
	thrdstat = new int[nthrds];
	workfd = new int[nthrds];
	persistent = new int[nthrds];
	for (i = 0; i < nthrds; i++) {
		// use C++11 lambda to create functions with indexices
		threads[i] = std::thread([=] {this->serveRequest(i);} );
		thrdstat[i] = T_RUNNABLE;
		workfd[i] = -1;
		persistent[i] = 0;
	}
}

int HttpServer::handleRequests()
{
	int workfd, i = 0;
	sockaddr_in cliaddr;
	socklen_t clilen = sizeof(cliaddr);
	memset(&cliaddr, 0, clilen);

	if ((workfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
		return -1;
	else {
		// assign work to worker thread; block if all threads are busy running
		while (thrdstat[i] != T_RUNNABLE)
			i = i + 1 % nthrds;
		// give worker thread necessary information (socket number)
		this->workfd[i] = workfd;
		thrdstat[i] = T_NEWTASK;
	}
}

void HttpServer::terminate()
{
	collectThreads();
	delete workfd;
	delete thrdstat;
	delete[] threads;
	delete address;
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	return;
}

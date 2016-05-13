#ifndef _HTTPMSG_H_
#define _HTTPMSG_H_

#include <string>
#include <string.h>
#include <map>


/* HTTP message class */
/* This class is the parent class for more specific HTTP request
    and HTTP response class */
class HttpMessage {
	std::string msgbody;
public:
	std::map<std::string, std::string> headers;
	void setMsgBody(std::string body) { msgbody = body; }
	std::string getMsgBody() { return msgbody; }
};

/* HTTP request class */
class HttpRequest : public HttpMessage {
	std::string method;
	std::string filedir;
	std::string version;

 public:
	void setMethod(std::string method) { this->method = method; }
	std::string getMethod() { return method; }
	void setFiledir(std::string filedir) { this->filedir = filedir; }
	std::string getFiledir() { return filedir; }
	void setVersion(std::string version) { this->version = version; }
	std::string getVersion() { return version; }
	std::string toString();
};

/* HTTP response class */
class HttpResponse : public HttpMessage {
	std::string version;
	std::string code;
	std::string phrase;
 public:
	void setVersion(std::string version) { this->version = version; }
	std::string getVersion() { return version; }
	void setCode(std::string code) { this->code = code; }
	std::string getCode() { return code; }
	void setPhrase(std::string phrase) { this->phrase = phrase; }
	std::string getPhrase() { return phrase; }
	std::string toString();
};



#endif

#include "httpmsg.h"

std::string HttpResponse::toString()
{
	std::string msgres = this->getVersion();
	msgres = msgres + " " + this->getCode();
	msgres = msgres + " " + this->getPhrase();
	msgres += "\r\n";

	for (auto it = this->headers.begin(); it != this->headers.end(); it++) {
		msgres = msgres + it->first + ": " + it->second + "\r\n";
	}
	msgres += "\r\n";

	msgres += this->getMsgBody();
	return msgres;
}

std::string HttpRequest::toString()
{
	std::string msgreq = getMethod();
	msgreq = msgreq + " " + getFiledir();
	msgreq = msgreq + " " + getVersion();
	msgreq += "\r\n";

	for (auto it = headers.begin(); it != headers.end(); it++) {
		msgreq = msgreq + it->first + ": " + it->second + "\r\n";
	}
	msgreq += "\r\n";
	msgreq += getMsgBody();
	return msgreq;
}
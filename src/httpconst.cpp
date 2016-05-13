#include "httpconst.h"

std::string toPhrase(int statcode)
{
	switch (statcode) {
		case STAT_OK:
			return "OK";
		case STAT_BADREQ:
			return "Bad Request";
		case STAT_NOTFOUND:
			return "Not Found";
		case STAT_SERVERR:
			return "Internal Error";
	}
	return "Unknown Code";
}

std::string defaultPort(std::string scheme)
{
	if (scheme == "http") {
		return "80";
	} else {
		return "";
	}
}
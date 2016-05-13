#ifndef _HTTPCONST_H_
#define _HTTPCONST_H_

#include <string>

#define STAT_OK			200
#define STAT_BADREQ		400
#define STAT_NOTFOUND	404
#define STAT_SERVERR	500

std::string toPhrase(int statcode);
std::string defaultPort(std::string scheme);

#endif
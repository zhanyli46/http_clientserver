#include "util.h"
#include <limits.h>
#include <algorithm>

std::string getpwd()
{
  char result[ PATH_MAX ];
  ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
  std::string str( result, (count > 0) ? count : 0 );
  size_t i = str.rfind("/");
  return str.substr(0, i);
}

std::string findMimeFromExt(const std::string ext)
{
	if (ext == "htm" | ext == ".html")
		return "text/html";
	else if (ext == ".jpg" | ext == ".jpeg")
		return "image/jpeg";
	else if (ext == ".gif")
		return "image/gif";
	else if (ext == ".unknown")
		return "application/unknown";
	else
		return "text/plain";
}

bool isNumeric(const std::string s)
{
	return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

bool fileExists(const std::string& name)
{
	struct stat buffer;   
	return (stat (name.c_str(), &buffer) == 0); 
}
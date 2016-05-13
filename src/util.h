#ifndef _UTIL_H_
#define _UTIL_H_

#include <sys/socket.h>
#include <cstdio>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>

std::string getpwd();
std::string findMimeFromExt(const std::string ext);
bool isNumeric(std::string str);
bool fileExists(const std::string& name);

#endif

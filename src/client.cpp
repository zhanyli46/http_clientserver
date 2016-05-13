#include "httpclient.h"
#include <iostream>
#include <fcntl.h>

void usage()
{
	std::cout << "Usage: ./web-client [url]\n";
}

int main(int argc, char** argv)
{
	int retval = 0;

	if (argc != 2)
	{
		usage();
	}

	HttpClient client;
	if ((retval = client.init(argv[1])) < 0) {
		if (retval == -1)
			std::cerr << "Init error: missing scheme in url\n";
		else if (retval == -2)
			std::cerr << "Init error: invalid file path\n";
		else
			std::cerr << "Init error: unrecognized scheme\n";
		return 1;
	}

	std::string response;
	if ((retval = client.requestWebpage(&response)) < 0) {
		switch (retval) {
			case -1:
				std::cerr << "Request error: cannot resolve host " << client.getHostName() << std::endl;
				break;
			case -2:
				std::cerr << "Request error: cannot create socket\n";
				break;
			case -3:	
				std::cerr << "Request error: cannot connect to server at " << client.getIp() << std::endl;
				break;
			case -4:
				std::cerr << "Communication error: cannot send data to " << client.getIp() << std::endl;
				break;
			case -5:
				std::cerr << "Communication error: cannot receive data from " << client.getIp() << std::endl;
				break;
			case -6:
				std::cerr << "Communication error: request to " << client.getIp() << " timeout\n";
				break;
			default:
				std::cerr << "Unknown error\n";
		}
		return 1;
	}

	if ((retval = client.interpretResponse(&response)) < 0) {
		std::cerr << "Response " << response << ", exiting.\n";
		return 1;
	}

	std::string filename;
	std::string filedir = client.getFiledir();
	if (filedir[filedir.length() - 1] == '/') {
		filename = "index.html";
	} else {
		int dirIndex = filedir.rfind("/");
		filename = filedir.substr(dirIndex + 1, filedir.length() - 1);
	}

	int i = 1;
	int dotIndex;
	do {
		if (fileExists(filename)) {
			if (i == 1) {
				filename = filename + "." + std::to_string(i);
				i++;
			} else {
				filename = filename.substr(0, filename.rfind(".") + 1) + std::to_string(i);
				i++;
			}
		} else {
			break;
		}
	} while (true);


	int fd;
	if ((fd = open(filename.c_str(), O_WRONLY | O_CREAT, 0664)) < 0) {
		std::cerr << "File error: cannot create file " << filename << std::endl;
		return 1;
	}

	if (write(fd, response.c_str(), response.length()) < 0) {
		std::cerr << "File error: cannot save data to \"" << filename << "\"\n";
		return 1;
	}

	close(fd);
	return 0;
}
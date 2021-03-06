#include "esock.h"

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <thread>
#include <string>
#include <string.h>
#include <unordered_map>
#include <iostream>

using std::string;
using std::thread;

esock::esock(const conn_type type, const int portno) : type(type), portno(portno) {
	error = false;
	thread_running = false;
	halt_thread = false;
	backlog = 5;

	serv_buff_len = 256;
	serv_buffer = new char[serv_buff_len];

	sockfd = -1;
}

esock::esock(const conn_type type, const int portno, const int backlog) : type(type), portno(portno), backlog(backlog) {
	error = false;
	thread_running = false;
	halt_thread = false;

	serv_buff_len = 256;
	serv_buffer = new char[serv_buff_len];

	sockfd = -1;
}

esock::esock(const conn_type type, const int portno, const int backlog, const int buff_len) : type(type), portno(portno), backlog(backlog) {
	error = false;
	thread_running = false;
	halt_thread = false;

	serv_buff_len = buff_len;
	serv_buffer = new char[serv_buff_len];

	sockfd = -1;
}

esock::~esock() {
	delete serv_buffer;
}

bool esock::accept_error() const {
	return error;
}

bool esock::bind(string trigger, const bind_func function) {
	// collsion <or> already added trigger check
	if ( thread_running ) return false;
	for (auto& kv : func_table) {
		auto trig = kv.first;
		if(trig.size() > trigger.size()) {
			if (trig.compare(0, trigger.size(), trigger) == 0 )
				return false;
		} else {
			if (trigger.compare(0, trig.size(), trig) == 0 )
				return false;
		}
	}

	func_table[trigger] = function;
	return true;
}

bool esock::start() {
	if (thread_running)
		return false;

	struct sockaddr_in serv_addr;

	// TODO: make this line dependent on the type of connection
	switch(type) {
		case tcp: sockfd = ::socket(AF_INET, SOCK_STREAM, 0); break;
		case udp: sockfd = ::socket(AF_INET, SOCK_DGRAM, 0); break;
	}

	if (sockfd < 0)
	{
		// throw error opening socket error instead?
		return false;
	}

	memset((char *) &serv_addr, 0, sizeof(serv_addr));

	// TODO: make this block dependent on the type of connection
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (::bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		// throw error on binding port error instead?
		return false;
	}

	listen(sockfd, backlog);

	// this point onward should get thrown into a thread
	// TODO: what if I want to launch this thread again?
	halt_thread = false;
	server_thread = thread(&esock::run, this);
	thread_running = true;

	return true;
}

void esock::run() {
	int newsockfd;
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);

	while( not halt_thread ) {
		newsockfd = ::accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
		{
			// since the thread had to be launched, the thread will simply break
			// raise a flag, and wait to be joined

			// TODO: check errono for an error that is fixable if so,
			// continue instead in hopes that it is eventually fixed

			error = true;
			break;
		}

		memset(serv_buffer, 0, serv_buff_len);
		
		if( ::read(newsockfd, serv_buffer, serv_buff_len) < 0 ) {
			// client dropped the connection for some reason
			close(newsockfd);
			continue;
		}

		string trigger(serv_buffer);

		for (auto &pair: func_table) {
			auto trig = pair.first;
			if (trigger.compare(0, trig.size(), trig) == 0 ) {
				string send = pair.second(trigger);
				if (not send.empty()) {
					::write(newsockfd, send.c_str(), send.size());
				}
				break;
			}
		}

		::close(newsockfd);
	}

	::close(sockfd);
}

bool esock::halt() {
	if ( not thread_running ) return false;
	if ( not server_thread.joinable() ) return false;

	::shutdown(sockfd, 0);
	halt_thread = true;

	server_thread.join();
	thread_running = false;
	sockfd = -1;

	return true;
}

#include "esock.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

using std::string;

esock::esock(const conn_type type, const int portno) : type(type), portno(portno) {
	accept_error = false;
	thread_running = false;
	backlog = 5;

	serv_buff_len = 256;
	serv_buffer = new char[serv_buff_len];

	sockfd = -1;
}

esock::esock(const conn_type type, const int portno, const int backlog) : type(type), portno(portno), backlog(backlog) {
	accept_error = false;
	thread_running = false;

	serv_buff_len = 256;
	serv_buffer = new char[serv_buff_len];

	sockfd = -1;
}

esock::esock(const conn_type type, const int portno, const int backlog, const int buff_len) : type(type), portno(portno), backlog(backlog) {
	accept_error = false;
	thread_running = false;

	serv_buff_len = buff_len;
	serv_buffer = new char[serv_buff_len];

	sockfd = -1;
}

esock::~esock() {
	delete serv_buffer;
}

esock::accept_error() const {
	return accept_error;
}

bool esock::bind(string trigger, string (*fcnPtr)(string) function) {
	return true;
}

bool esock::start() {
	if (thread_running)
		return false;

	struct sockaddr_in serv_addr;

	// TODO: make this line dependent on the type of connection
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
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

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		// throw error on binding port error instead?
		return false;
	}

	listen(sockfd, backlog);

	// this point onward should get thrown into a thread
	// TODO: what if I want to launch this thread again?
	halt_thread = false;
	server_thread (&esock::run, this);

	return true;
}

void esock::run() {
	int newsockfd;
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);

	while( not halt_thread ) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
		{
			// since the thread had to be launched, the thread will simply break
			// raise a flag, and wait to be joined

			// TODO: check errono for an error that is fixable if so,
			// continue instead in hopes that it is eventually fixed
			accept_error = true;
			break;
		}

		// TODO: implement this stuff

		// get entire message and load it into a string
		// compare against all the key-value pairs in the map
		// if there is a match, then call the function,
		// if there response is not null then send the repsonse

		close(newsockfd);
	}

	close(sockfd);
}

bool esock::halt() {
	if ( not server_thread.joinable() )
		return false

	close(sockfd);
	halt_thread = true;
	server_thread.join();
}

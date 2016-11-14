#pragma once

#include <string>
#include <thread>
#include <unordered_map>

using std::string;
using std::thread;
using std::unordered_map;

typedef string (*bind_func)(string);

class esock {
public:
	enum conn_type {
		tcp
	};

	esock(const conn_type type, const int portno);
	esock(const conn_type type, const int portno, const int backlog);
	esock(const conn_type type, const int portno, const int backlog, const int buff_len);
	~esock();

	// returns true if the server thread encountered an accept error
	// and the thread is waiting to terminate
	// this should be checked a few ms after start is called
	bool accept_error() const;

	// launches the server, at this point all events should
	// have been bound with the bind function
	bool start();

	// bind functions based on trigger string
	// the function is called if the message recived starts with
	// the trigger string
	bool bind(string trigger, bind_func function);

	// makes the thread halt and ends the server process
	// also closes the port and the socket
	bool halt();

	// sends a message to a target machine
	// this can be called statically without instantizing the class
	// static string send_message(const conn_type conn, const int portno, const string hostname, const string message) const;

private:
	conn_type type;
	int portno;
	int sockfd;
	int backlog;
	int serv_buff_len;

	thread server_thread;
	bool thread_running;
	bool halt_thread;
	bool error;

	unordered_map <string, bind_func> func_table;

	char* serv_buffer;

	// the thread function that handles the accept calls
	void run();
};

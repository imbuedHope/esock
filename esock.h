#pragma once

using std::string;
using std::thread;

enum conn_type {
	tcp
};

class esock {
	bool active;
public:
	esock(const conn_type type, const int portno);
	esock(const conn_type type, const int portno, const int backlog);
	esock(const conn_type type, const int portno, const int backlog, const int buff_len);

	~esock();

	// returns true if the server thread encountered an accept error
	// and the thread is waiting to terminate
	bool accept_error() const;

	// launches the server, at this point all events should
	// have been bound with the bind function
	bool start();

	// bind functions based on trigger string
	// the function is called if the message recived starts with
	// the trigger string
	bool bind(string trigger, string (*fcnPtr)(string) function);

	// makes the thread halt and ends the server process
	// also closes the port and the socket
	void halt();

	// sends a message to a target machine
	// this can be called statically without instantizing the class
	static string send_message(const conn_type conn, const int portno, const string hostname, const string message) const;

private:
	bool accept_error;
	conn_type type;
	int portno;
	thread server_thread;
	bool thread_running;
	int sockfd;
	
	// the thread function that handles the accept calls
	void run();
};

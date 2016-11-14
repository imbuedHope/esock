#include "../esock.h"

#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::endl;
using std::cin;

string handle_asdf(string input) {
	cout << input << endl;
	return "pong";
}

int main() {
	string in;

	esock socket(esock::tcp, 50000);

	if (not socket.bind("ping", handle_asdf) ) {
		cout << "error binding" << endl;
		return -1;
	}

	if (not socket.start() ) {
		cout << "error starting" << endl;
		return -1;
	}

	cout << "waiting on input to crash" << endl;
	cin >> in;

	if (not socket.halt() ) {
		cout << "error halting" << endl;
		return -1;
	}

	return 0;
}
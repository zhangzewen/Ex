#include <iostream>

using namespace std;
using std::cin;
using std::cout;

class Logging_Acceptor: public Event_Handler
// = TITLE
// 	Handles client connection requests.
{
public:
	
	//Initialize the acceptor_endpoint and
	// register with the Initiation Dispatcher.
	Logging_Acceptor (const INET_Addr &addr);
	
	//Factory method tha accepts a new
	//SOCK_Stream connection and creates a
	//Logging_Handler object to handle logging
	//records set using the connection.

	virtual void handle_event (Event_Type et);
	
	//Get the I/O Handle (called by the
	//Initiation Dispatcher when
	//Logging_Acceptor is registered).
	virtual HANDLE get_handle (void) const
	{
		return acceptor._get_handle();
	}

private:
	// Socket factory that accepts client
	// connections.
	SOCK_Acceptor acceptor_;
};

#include <iostream>

using std::cin;
using std::cout;

Logging_Acceptor::Logging_Acceptor(const INET_Addr &addr) : acceptor_ (addr)
{
	// Register acceptor with the Initiation
	// Dispatcher, which "double dispatcher"
	// the Logging_Acceptor::get_handle()method
	// to obtain the HANDLE
	Initiation_Dispatcher::instance()->register_handler(this, ACCEPT_EVENT);
}

void Logging_Acceptor::handle_event(Event_Type et)
{
	//Can only be called for an ACCEPT event.
	
	assert( et == ACCEPT_EVENT);

	SOCK_Stream new_connection;
	
	//Accept the connection
	acceptor_.accept (new_connection);
	
	//Create a new Logging Handler.

	Logging_Handler *handler = new Logging_Handler(new_connection);
}

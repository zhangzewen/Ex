#include <iostream>

using namespace std;
using std::cin;
using std::cout;

enum Event_Type
// = TITLE
//	Types of events handled by the 
//	Initiation_Dispatcher
{
	ACCEPT_EVENT = 01,
	READ_EVENT = 02,
	WRITE_EVENT = 04,
	TIMEOUT_EVENT = 010,
	SIGNAL_EVENT = 020,
	CLOSE_EVENT = 040
};


class Initiation_Dispatcher
// = TITLE
//	Demultiplex and dispatch Event_Handlers
//	in response to client requests
{
public:
	//Register an Event_Handler of a particular
	//Event_Type(e.g., READ_EVENT, ACCEPT_EVENT,
	//etc.).
	int register_handler(Event_Handler *eh, Event_Type et);

	// Remove an Event_Handler of a particular
	//Event_Type
	int remove_handler(Event_Handler *eh, Event_Type et);

	// Entry point into the reactive event loop
	int handle_events(Time_Value *timeout = 0);
}

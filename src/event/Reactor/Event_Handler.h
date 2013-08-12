#include <iostream>
using namespace std;
using std::cin;
using std::cout;

#if 0
//version 1
class Event_Handler
// = TITLE
//	Abstract base class that servers as the 
//	target of the Initiation_Dispatcher.
public:
	//Hook method that is called back by the
	// Initiation_Dispatcher to handle events.
	virtual int handle_event(Event_Type et) = 0;
	
	// Hook method that returns the underlying
	// I/O Handle.
	virtual Handle get_handle (void) const = 0;
};
#endif

class Event_Handler
{
public:
	// Hock methods that are called back by
	// the Initiation_Dispatcher to handle
	// particular types of events
	
	virtual int handle_accept (void) = 0;
	virtual int handle_input (void) = 0;
	virtual int handle_output (void) = 0;
	virtual int handle_timeout (void) = 0;
	virtual int handle_close (void) = 0;

	// Hook method that returns the underlying
	// I/O Handle.

	virtual Handle get_handle (void) const = 0;
}

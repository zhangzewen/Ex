#include <iostream>

using namespace std;
using std::cin;
using std::cout;

cladd Logging_Handler : public Event_Handler
// = TITLE
//		Receive and process logging records
//		sent by a client application
{
public:
	// Initialize the client stream.
	Logging_Handler (SOCK_Stream &cs);
	
	// Hook method that handles the reception
	// of logging records from clients

	virtual void handle_event(Event_Type et);
	
	//Get the I/O Handle (called by the
	//Initiation Dispatcher when
	//Logging_Handler is registered

	virtual HANDLE get_handle (void) const
	{
		return peer_stream_.get_handle();
	}
private:
	//Recevies logging records from a client.
	SOCK_Stream peer_stream_;
};

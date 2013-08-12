#include <iostream>

using namespace std;
using std::cin;
using std::cout;

void Logging_Handler::handle_event (Event_Type et)
{
	if (et == READ_EVENT) {
		Log_Record log_record;
		
		peer_stream_.recv((void *)log_record, sizeof(log_record));
		
		//Write logging record to standard output.

		log_record.write(STDOUT);
	} else if(et == CLOSE_EVENT){
		peer_stream_.close();
		delete (void *) this;
	}
}



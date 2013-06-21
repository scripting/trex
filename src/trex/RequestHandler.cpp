#include "RequestHandler.h"

namespace Trex {

	RequestHandler::RequestHandler(Runtime* runtime){
	    _runtime = runtime;
	}

	RequestHandler::~RequestHandler(){
	    
	}

	Response* RequestHandler::process(Request* request){
	    return _runtime->handle(request);
	}

}

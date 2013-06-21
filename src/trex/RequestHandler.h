#ifndef __TrexRequestHandler__
#define __TrexRequestHandler__

#include "Request.h"
#include "Response.h"
#include "Runtime.h"

namespace Trex {

	class RequestHandler {
	private:
	    Runtime* _runtime;
	public:
	    explicit RequestHandler(Runtime* runtime);
	    virtual ~RequestHandler();
	    Response* process(Request* request);
	};

}

#endif

#ifndef __TrexRuntime__
#define __TrexRuntime__

#include <v8.h>
#include "../libxmljs/libxmljs.h"
#include "Request.h"
#include "Response.h"

using namespace v8;
using namespace Trex;

namespace Trex {
	class Runtime{
	private:
	    Persistent<Context> context;
	    Isolate* isolate;
	    Locker * locker;
	    libxmljs::LibXMLJS* _libxmljs;
	    bool busy;
	    bool stop;
	    void ReportException(v8::TryCatch* try_catch);
	public:
		  bool compileError;
			static void restart();
			static void killRuntime(long threadId, Runtime* runtime);
			static Runtime* instance();
			static Runtime* instanceForThreadId(long threadId);
			static void cleanup();
	    Response* handle(Request* request);
	    bool isBusy();
	    void stopRuntime();
	    void terminateExecution();
	    bool shouldStop();
	    explicit Runtime();
	    virtual ~Runtime();
	protected:
			
	};
}

#endif

#ifndef __TrexResponse__
#define __TrexResponse__

#include <string>
#include <map>

using namespace std;

namespace Trex {

	class Response {
	private:
	    string _body;
	    unsigned int _code;
	    map<string, string> _headers;
	public:
	    explicit Response(unsigned int code, string body, map<string,string> headers);
	    virtual ~Response();
	    int code();
	    string body();
	    map<string,string> headers();
	};

}

#endif

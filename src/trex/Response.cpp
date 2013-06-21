#include "Response.h"

namespace Trex {

	Response::Response(unsigned int code, string body, map<string,string> headers){
	    _code=code;
	    _body = body;
	    _headers = headers;
	}

	Response::~Response(){

	}

	int Response::code(){
	    return _code;
	}

	string Response::body(){
	    return _body;
	}

	map<string,string> Response::headers(){
	    return _headers;
	}

}
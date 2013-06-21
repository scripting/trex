#ifndef __TrexRequest__
#define __TrexRequest__

#include <iostream>
#include <string>
#include <map>
#include "../mongoose/mongoose.h"

using namespace std;

namespace Trex {

    class Request {
    private:
        struct mg_connection *_conn;
        const struct mg_request_info* _request;
        string _body;
    public:
        explicit Request(struct mg_connection *conn);
        virtual ~Request();
        string uri();
        string query_string();
        string method();
        map<string,string> headers();
        string body();
    };

}

#endif

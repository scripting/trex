#include "Request.h"

namespace Trex {

    Request::Request(struct mg_connection *conn){
        _conn=conn;
        _request=mg_get_request_info(_conn);
        char post_data[4096];
        int post_data_len;
        while((post_data_len = mg_read(_conn, post_data, sizeof(post_data)))>0){
            _body.append(post_data, post_data_len);
        }
    }

    Request::~Request(){
    }

    string Request::uri(){
        return string(_request->uri);
    }

    string Request::query_string(){
        if(_request->query_string){
            return string(_request->query_string);
        }
        return string();
    }

    string Request::method(){
        return string(_request->request_method);
    }

    map<string,string> Request::headers(){
        map<string,string> _headers;
        for(int i=0; i < _request->num_headers; i++){
            string headerName(_request->http_headers[i].name);
            string headerValue(_request->http_headers[i].value);
            _headers.insert ( std::pair<string,string>(headerName,headerValue) );
        }
        return _headers;
    }

    string Request::body(){
        return _body;
    }

}
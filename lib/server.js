var server = {
    respondersOrder: [],
    responders: {},
    addResponder: function(name, cb){
        if(!server.responders[name]){
            server.respondersOrder.push(name);
        }
        server.responders[name] = cb;
    },
    removeResponder: function(name){
        var i = server.respondersOrder.indexOf(name);
        if(i>=0){
            delete server.respondersOrder[i];
        }
        delete server.responders[name];
    },
    notFoundResponse: function(){
        var notFound = {};
        notFound["code"]=404;
        var notFoundBody = "<html><body><h1>NOT FOUND</h1></body></html>";
        notFound["body"]=notFoundBody;
        notFound["headers"] = {"Content-Type":"text/html", "Content-Length": ""+notFoundBody.length};
        return notFound;
    },
    exceptionResponse: function(tryerror){
        var errorResponse = {};
        errorResponse["code"]=500;
        var errorBody = "<html><head><title>Error</title></head><body><h1>Error</h1><p>Trex couldn't complete your request because an exception was thrown.</p><code>" + tryerror + "</code></body></html>";
        errorResponse["body"]=errorBody;
        errorResponse["headers"] = {"Content-Type":"text/html", "Content-Length": ""+errorBody.length};
        return errorResponse;
    },
    handle: function(pagetable){
        try{
            pagetable.response = { "code" : 200, "body" : "", "headers" : {"Content-Type" : "text/html"}};
            pagetable.request.params = {};
            pagetable.request.query_string.replace(
                                         new RegExp("([^?=&]+)(=([^&]*))?", "g"),
                                         function($0, $1, $2, $3) { pagetable.request.params[$1] = decodeURIComponent($3); }
                                         );
            var routeHandled = false;

            for(var i in server.respondersOrder){
                var responderName = server.respondersOrder[i];
                if(responderName){
                    var responder = server.responders[responderName];
                    if(responder){
                        var result = responder(pagetable);
                        if(result){
                            routeHandled=true;
                            break;
                        }
                    }
                }
            }
            if(routeHandled){
                return pagetable.response;
            }else{
                return server.notFoundResponse();
            }
        }catch(tryerror){
            return server.exceptionResponse(tryerror);
        }
    }
};
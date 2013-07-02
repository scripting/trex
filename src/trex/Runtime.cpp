#include "Runtime.h"

#include <v8.h>
#include <string>
#include <map>
#include <pthread.h>
#include <cstdlib>

#include "trex.h"
#include "global/Include.h"
#include "global/Log.h"
#include "global/File.h"
#include "global/Leveldb.h"
#include "global/Http.h"
#include "global/Crypto.h"
#include "global/Email.h"

using namespace std;
using namespace v8;

namespace Trex {
    // Extracts a C string from a V8 Utf8Value.
    const char* ToCString(const v8::String::Utf8Value& value) {
        return *value ? *value : "<string conversion failed>";
    }

    void Runtime::ReportException(v8::TryCatch* try_catch) {
        compileError=true;
      v8::HandleScope handle_scope(isolate);
      v8::String::Utf8Value exception(try_catch->Exception());
      const char* exception_string = ToCString(exception);
      v8::Handle<v8::Message> message = try_catch->Message();
      if (message.IsEmpty()) {
        // V8 didn't provide any extra information about this error; just
        // print the exception.
        fprintf(stderr, "%s\n", exception_string);
      } else {
        // Print (filename):(line number): (message).
        v8::String::Utf8Value filename(message->GetScriptResourceName());
        const char* filename_string = ToCString(filename);
        int linenum = message->GetLineNumber();
        fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
        // Print line of source code.
        v8::String::Utf8Value sourceline(message->GetSourceLine());
        const char* sourceline_string = ToCString(sourceline);
        fprintf(stderr, "%s\n", sourceline_string);
        // Print wavy underline (GetUnderline is deprecated).
        int start = message->GetStartColumn();
        for (int i = 0; i < start; i++) {
          fprintf(stderr, " ");
        }
        int end = message->GetEndColumn();
        for (int i = start; i < end; i++) {
          fprintf(stderr, "^");
        }
        fprintf(stderr, "\n");
        v8::String::Utf8Value stack_trace(try_catch->StackTrace());
        if (stack_trace.length() > 0) {
          const char* stack_trace_string = ToCString(stack_trace);
          fprintf(stderr, "%s\n", stack_trace_string);
        }
      }
    }

    Runtime::Runtime(){  
        busy=false;  
        stop=false;
        compileError=false;

        int length;
        char * buffer;
        
        ifstream is;
        string path(getenv("TREX_DIR"));
        path.append("/lib/trex.js");
        is.open (path.c_str());
        
        // get length of file:
        is.seekg (0, ios::end);
        length = is.tellg();
        is.seekg (0, ios::beg);
        
        // allocate memory:
        buffer = new char [length];
        
        // read data as a block:
        is.read (buffer,length);
        is.close();

        isolate = v8::Isolate::New();
        {
            v8::Isolate::Scope isolate_scope(isolate);
            v8::Locker v8ThreadLock(isolate);
            HandleScope handle_scope(isolate);

            Handle<ObjectTemplate> global = ObjectTemplate::New();
            global->Set(String::New("log"), FunctionTemplate::New(Trex::Global::Log));
            
            Handle<ObjectTemplate> console = ObjectTemplate::New();
            console->Set(String::New("log"), FunctionTemplate::New(Trex::Global::Log));
            global->Set(String::New("console"), console);

            global->Set(String::New("include"), FunctionTemplate::New(Trex::Global::Include));

            Handle<ObjectTemplate> fs = ObjectTemplate::New();
            fs->Set(String::New("read"), FunctionTemplate::New(Trex::Global::File::Read));
            global->Set(String::New("file"), fs);
            
            Handle<ObjectTemplate> http = ObjectTemplate::New();
            http->Set(String::New("get"), FunctionTemplate::New(Trex::Global::Http::Get));
            http->Set(String::New("post"), FunctionTemplate::New(Trex::Global::Http::Post));
            http->Set(String::New("request"), FunctionTemplate::New(Trex::Global::Http::Request));
            global->Set(String::New("http"), http);
            
            Handle<ObjectTemplate> libxml = ObjectTemplate::New();
            global->Set(String::New("libxml"), libxml);
            
            Handle<ObjectTemplate> leveldb = ObjectTemplate::New();
            leveldb->Set(String::New("get"), FunctionTemplate::New(Trex::Global::Leveldb::Get));
            leveldb->Set(String::New("put"), FunctionTemplate::New(Trex::Global::Leveldb::Put));
            leveldb->Set(String::New("delete"), FunctionTemplate::New(Trex::Global::Leveldb::Delete));
            global->Set(String::New("leveldb"), leveldb);

            Handle<ObjectTemplate> crypto = ObjectTemplate::New();
            crypto->Set(String::New("PBKDF2hex"), FunctionTemplate::New(Trex::Global::Crypto::PBKDF2hex));
            global->Set(String::New("crypto"), crypto);

            Handle<ObjectTemplate> email = ObjectTemplate::New();
            email->Set(String::New("send"), FunctionTemplate::New(Trex::Global::Email::Send));
            global->Set(String::New("email"), email);

            char* trexEnv;
            trexEnv = getenv("TREX_ENV");
            if(trexEnv==NULL){
                trexEnv = "development";
            }
            global->Set(String::New("TREX_ENV"), String::New(trexEnv, strlen(trexEnv)));

            char *opml = getenv("TREX_OPML");
            global->Set(String::New("TREX_OPML"), String::New(opml, strlen(opml)));
            
            context = Context::New(NULL, global);

            {
                Context::Scope context_scope(context);

                _libxmljs = new libxmljs::LibXMLJS(context->Global()->Get(String::New("libxml"))->ToObject());
                isolate->SetData(_libxmljs);

                Local<String> source = String::New(buffer, length);

                Local<Script> script = Script::Compile(source);

                v8::TryCatch try_catch;
                bool report_exceptions=true;
                if (script.IsEmpty()) {
                    // Print errors that happened during compilation.
                    if (report_exceptions)
                        ReportException(&try_catch);
                } else {
                    Local<Value> result = script->Run();
                    if (result.IsEmpty()) {
                        assert(try_catch.HasCaught());
                        // Print errors that happened during execution.
                        if (report_exceptions)
                            ReportException(&try_catch);
                    }else if(result->IsString()){
                        Local<String> tSource = result->ToString();
                        Local<Script> tScript = Script::Compile(tSource);
                        if (tScript.IsEmpty()) {
                            // Print errors that happened during compilation.
                            if (report_exceptions)
                                ReportException(&try_catch);
                        }else{
                            tScript->Run();
                        }
                    }
                }
            }

        }

        delete [] buffer;
    }

    Runtime::~Runtime(){
        gc();
        {
            v8::Isolate::Scope isolate_scope(isolate);
            v8::Locker v8ThreadLock(isolate);
            context.Dispose(isolate);
        }
        if(isolate->GetData()!=NULL){
            delete isolate->GetData();
        }
        isolate->Dispose();
    }

    void Runtime::gc(){
        // Collect Garbage
        while(!V8::IdleNotification()) {};
    }

    static map<long, Runtime*> runtimes;
    
    void Runtime::restart(){
        map<long, Runtime*>::iterator it;
        for (map<long, Runtime*>::iterator it=runtimes.begin(); it!=runtimes.end(); ++it){
            long key = it->first;
            Runtime* value = it->second;
            killRuntime(key, value);
        }
    }

    void Runtime::killRuntime(long threadId, Runtime* runtime){
        if(runtime->isBusy()){
            runtime->stopRuntime();
        }else{
            if(runtime){
                delete runtime;
            }
            runtimes.erase(threadId);
            
        }
    }

    Runtime* Runtime::instance(){
        long threadId = (long)pthread_self();
        map<long, Runtime*>::iterator it;
        it = runtimes.find(threadId);
        Runtime* runtime;
        if(it!=runtimes.end()){
            runtime = it->second;
        }else{
            runtime = new Runtime();
            runtimes[threadId] = runtime;
        }
        return runtime;
    }

    Runtime* Runtime::instanceForThreadId(long threadId){
        map<long, Runtime*>::iterator it;
        it = runtimes.find(threadId);
        Runtime* runtime = NULL;
        if(it!=runtimes.end()){
            runtime = it->second;
        }
        return runtime;
    }

    void Runtime::cleanup(){
        map<long, Runtime*>::iterator it;
        for (map<long, Runtime*>::iterator it=runtimes.begin(); it!=runtimes.end(); ++it){
            if(it->second){
                delete it->second;
            }
        }
    }

    bool Runtime::isBusy(){
        return busy;
    }

    void Runtime::stopRuntime(){
        stop=true;
    }

    bool Runtime::shouldStop(){
        return stop;
    }

    void Runtime::terminateExecution(){
        v8::V8::TerminateExecution(isolate);
    }

    Response* Runtime::handle(Request* Request){
        busy=true;
        Response* response;

        {   
            v8::Locker v8ThreadLock(isolate);
            v8::Isolate::Scope isolate_scope(isolate);

            HandleScope handle_scope(isolate);
        
            Context::Scope context_scope(context);

            v8::TryCatch try_catch;
            bool report_exceptions=true;
            
            Handle<ObjectTemplate> request = ObjectTemplate::New();
            request->Set(String::New("uri"), String::New(Request->uri().c_str()));
            request->Set(String::New("method"), String::New(Request->method().c_str()));
            request->Set(String::New("body"), String::New(Request->body().c_str()));
            request->Set(String::New("query_string"), String::New(Request->query_string().c_str()));

            Handle<Object> requestHeadersObject = Object::New();
            map<string,string> requestHeaders = Request->headers();
            map<string,string>::iterator it;
            for (map<string,string>::iterator it=requestHeaders.begin(); it!=requestHeaders.end(); ++it){
                string key = it->first;
                Handle<Value> keyValue = String::New(key.c_str(), key.length());
                string value = it->second;
                Handle<Value> valueValue = String::New(value.c_str(), value.length());
                requestHeadersObject->Set(keyValue, valueValue);
            }
            request->Set(String::New("headers"), requestHeadersObject);

            Handle<Object> pagetable = Object::New();
            Handle<Object> requestObj = request->NewInstance();
            pagetable->Set(String::New("request"), requestObj);

            Handle<Value> server_value = context->Global()->Get(String::New("server"));
            Handle<Object> serverObj = server_value->ToObject();

            Handle<Value> handle_value = serverObj->Get(String::New("handle"));
            
            Handle<Function> handle_function = Handle<Function>::Cast(handle_value);
            Handle<Value> responseValue;

            v8::Locker::StartPreemption(1000);

            Handle<Value> argv[1] = {pagetable};
            responseValue = handle_function->Call(context->Global(), 1, argv);

            v8::Locker::StopPreemption();

            if(responseValue.IsEmpty()){
                assert(try_catch.HasCaught());
                // Print errors that happened during execution.
                if (report_exceptions)
                    ReportException(&try_catch);
                response = new Response(500, string("<html><head><title>Timeout</title></head><body><h1>Timeout</h1><p>Couldn't handle your request because it timed out.</p></body></html>"), map<string,string>());
            }else{
                assert(!try_catch.HasCaught());
                Handle<Object> o = responseValue->ToObject();
                String::Utf8Value body(o->Get(String::New("body")));
                string responseBody(*body);

                Handle<Number> n = o->Get(String::New("code"))->ToNumber();
                unsigned int responseCode = (unsigned int)(n->Value());
                
                Handle<Object> headers = o->Get(String::New("headers"))->ToObject();
                map<string,string> responseHeaders;
                
                Handle<Array> propertyNames = headers->GetPropertyNames();
                for(unsigned int i = 0; i < propertyNames->Length(); i++){
                    Handle<Value> propertyNameValue = propertyNames->Get(i);
                    Handle<Value> propertyValue = headers->GetRealNamedProperty(propertyNameValue->ToString());
                    if(!propertyValue->IsUndefined() && !propertyValue->IsNull() && (propertyValue->IsString() || propertyValue->IsStringObject())){
                        String::Utf8Value propertyName(propertyNameValue->ToString());
                        String::Utf8Value value(propertyValue);
                        string headerName(*propertyName);
                        string headerValue(*value);
                        responseHeaders.insert ( std::pair<string,string>(headerName,headerValue) );
                    }
                }
                
                std::stringstream lenstream;
                lenstream << responseBody.length();
                responseHeaders.insert ( std::pair<string,string>(string("Content-Length"), lenstream.str()) );
                
                response = new Response(responseCode, responseBody, responseHeaders);
            }

        }

        busy=false;
        return response;
    }

}

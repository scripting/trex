#ifndef __TrexInclude__
#define __TrexInclude__

#include <v8.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <locale>
#include <istream>
#include <fstream>
#include "Http.h" 

using namespace v8;
using namespace std;

namespace Trex{
	namespace Global {
		static Handle<Value> Include(const Arguments& args){
            if (args.Length() < 1) return v8::Undefined();

            HandleScope scope;

            Handle<Value> arg = args[0];
            String::Utf8Value value(arg);

            string file(*value);

            if(file.find("http")==0){
                  Handle<Value> httpGetValue = Trex::Global::Http::Get(args);
                  Handle<Object> httpObject = httpGetValue->ToObject();
                  int32_t code = httpObject->Get(String::New("code"))->Int32Value();
                  if(code==200){
                        Handle<String> source = httpObject->Get(String::New("body"))->ToString();
                        Handle<Script> script = Script::Compile(source);
                        script->Run();
                        return v8::True();
                  }
            }else{
                  string path(getenv("TREX_DIR"));
                  path.append("/lib/");
                  path.append(file);
                  
                  int length;
                  char * buffer;
                  
                  ifstream is;
                  is.open (path.c_str());
                  
                  // get length of file:
                  is.seekg (0, ios::end);
                  length = is.tellg();
                  is.seekg (0, ios::beg);
                  
                  if(length<=0){
                      return v8::False();
                  }
                  
                  // allocate memory:
                  buffer = new char [length];
                  
                  // read data as a block:
                  is.read (buffer,length);
                  is.close();
                  
                  Handle<String> source = String::New(buffer, length);
                  
                  Handle<Script> script = Script::Compile(source);
                  
                  script->Run();

                  delete [] buffer;

                  return v8::True();
            }
            
            return v8::False();
        }
	}
}

#endif

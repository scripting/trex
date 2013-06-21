#ifndef __TrexFile__
#define __TrexFile__

#include <iostream>
#include <v8.h>

using namespace v8;

namespace Trex{
	namespace Global {
		namespace File {
			static Handle<Value> Read(const Arguments& args) {
			    if (args.Length() < 1) return v8::Undefined();
			    HandleScope scope;
			    Handle<Value> arg = args[0];
			    String::Utf8Value value(arg);
			    
			    int length;
			    char * buffer;
			    
			    ifstream is;
			    is.open (*value);
			    
			    // get length of file:
			    is.seekg (0, ios::end);
			    length = is.tellg();
			    is.seekg (0, ios::beg);
			    
			    if(length<0){
			        return Undefined();
			    }
			    
			    // allocate memory:
			    buffer = new char [length];
			    
			    // read data as a block:
			    is.read (buffer,length);
			    is.close();
			    
			    Handle<String> contents = String::New(buffer, length);

			    delete [] buffer;
			    
			    return scope.Close(contents);
			}
		}
	}
}

#endif
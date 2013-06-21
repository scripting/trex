#ifndef __TrexLog__
#define __TrexLog__

#include <iostream>
#include <v8.h>

using namespace v8;

namespace Trex{
	namespace Global {
		static Handle<Value> Log(const Arguments& args) {
		    if (args.Length() < 1) return v8::Undefined();
		    HandleScope scope;
		    Handle<Value> arg = args[0];
		    String::Utf8Value value(arg);
		    String::AsciiValue ascii(arg);
		    printf("%s\n", *ascii);
		    return v8::Undefined();
		}
	}
}

#endif
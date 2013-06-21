#ifndef __TrexLevelDb__
#define __TrexLevelDb__

#include <iostream>
#include <string>
#include <v8.h>

using namespace v8;
using namespace std;

namespace Trex{
	namespace Global {
		namespace Leveldb {
			static Handle<Value> Get(const Arguments& args) {
			    if (args.Length() < 1) return v8::Undefined();
			    HandleScope scope;
			    Handle<Value> arg = args[0];
			    String::Utf8Value keyValue(arg);
			    string key(*keyValue);
			    Trex::Db* db = Trex::Db::instance();
			    string value = db->Get(key);
			    if(value.empty()){
			        return Undefined();
			    }
			    Handle<String> result = String::New(value.c_str(), value.length());
			    return scope.Close(result);
			}

			static Handle<Value> Put(const Arguments& args) {
			    if (args.Length() < 2) return v8::Undefined();
			    HandleScope scope;
			    Handle<Value> arg = args[0];
			    String::Utf8Value keyValue(arg);
			    string key(*keyValue);
			    Handle<Value> arg1 = args[1];
			    String::Utf8Value valueValue(arg1);
			    string value(*valueValue);
			    Trex::Db* db = Trex::Db::instance();
			    bool success = db->Put(key, value);
			    return scope.Close(Boolean::New(success));
			}

			static Handle<Value> Delete(const Arguments& args) {
			    if (args.Length() < 1) return v8::Undefined();
			    HandleScope scope;
			    Handle<Value> arg = args[0];
			    String::Utf8Value keyValue(arg);
			    string key(*keyValue);
			    Trex::Db* db = Trex::Db::instance();
			    bool success = db->Delete(key);
			    return scope.Close(Boolean::New(success));
			}
		}
	}
}

#endif
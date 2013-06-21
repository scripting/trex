#ifndef __TrexCryptoDb__
#define __TrexCryptoDb__

#include <iostream>
#include <string>
#include <v8.h>
#include <openssl/evp.h>

using namespace v8;
using namespace std;

namespace Trex{
	namespace Global {
		namespace Crypto {
			static Handle<Value> PBKDF2hex(const Arguments& args) {
			    if (args.Length() < 4) return v8::Undefined();
			    HandleScope scope;
			    Handle<Value> arg = args[0];
			    String::Utf8Value password(arg);
			    Handle<Value> arg1 = args[1];
			    String::Utf8Value salt(arg1);
			    if (!args[2]->IsNumber()) {
			    	return Undefined();
			    }
			    if (!args[3]->IsNumber()) {
			    	return Undefined();
			    }
			    uint32_t iterations = args[2]->Uint32Value();
					uint32_t key_length = args[3]->Uint32Value();

			    const char* pwd = *password;
			    unsigned char* salt_value = (unsigned char *)*salt;

			    size_t i;
	        unsigned char *out;

	        out = (unsigned char *) malloc(sizeof(unsigned char) * key_length);

	        string c;

	        if( PKCS5_PBKDF2_HMAC_SHA1(pwd, strlen(pwd), salt_value, sizeof(salt_value), iterations, key_length, out) != 0 )
	        {
	                for(i=0;i<key_length;i++) { char buffer[4]; sprintf(buffer, "%02x", out[i]); c.append(buffer); }
	        }
	        else
	        {
	                return Undefined();
	        }

			    Handle<Value> result = String::New(c.c_str(), c.length());
			    return scope.Close(result);
			}
		}
	}
}

#endif
#ifndef __TrexHttp__
#define __TrexHttp__

#include <iostream>
#include <v8.h>
#include <curl/curl.h>
#include <vector>

using namespace v8;

namespace Trex{
	namespace Global {
		namespace Http {
			struct MemoryStruct {
			    char *memory;
			    size_t size;
			};

			static size_t
			WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
			{
			    size_t realsize = size * nmemb;
			    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
			    
			    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
			    if (mem->memory == NULL) {
			        /* out of memory! */
			        printf("not enough memory (realloc returned NULL)\n");
			        exit(EXIT_FAILURE);
			    }
			    
			    memcpy(&(mem->memory[mem->size]), contents, realsize);
			    mem->size += realsize;
			    mem->memory[mem->size] = 0;
			    
			    return realsize;
			}

			static size_t
			WriteHeaderCallback(void *contents, size_t size, size_t nmemb, void* userp)
			{
			    size_t realsize = size * nmemb;

			    vector<string>* headerArray = (vector<string>*)userp;
			    headerArray->push_back(string((char*)contents, realsize));

			    return realsize;
			}

			static Handle<Value> Get(const Arguments& args) {
			    
			    if (args.Length() < 1) return v8::Undefined();
			    HandleScope scope;
			    Handle<Value> arg = args[0];
			    String::Utf8Value value(arg);
			    
			    Handle<Object> headers;
			    if(args.Length()>=2){
			        Handle<Value> headerArg = args[1];
			        if(!headerArg->IsUndefined()){
			            headers = headerArg->ToObject();
			        }
			    }
			    
			    struct MemoryStruct chunk;
			    
			    chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
			    chunk.size = 0;    /* no data at this point */

			    CURL *curl;
			    
			    curl = curl_easy_init();
			    if(curl==NULL) {
			        return Undefined();
			    }

			        /* specify URL to get */
			        curl_easy_setopt(curl, CURLOPT_URL, *value);
			        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			        
			        struct curl_slist *headerList=NULL;
			        
			        /* headers */
			        if(!headers.IsEmpty()){
			            Handle<Array> propertyNames = headers->GetPropertyNames();
			            for(unsigned int i = 0; i < propertyNames->Length(); i++){
			                Handle<Value> propertyNameValue = propertyNames->Get(i);
			                Handle<Value> propertyValue = headers->GetRealNamedProperty(propertyNameValue->ToString());
			                if(!propertyValue->IsUndefined() && !propertyValue->IsNull() && (propertyValue->IsString() || propertyValue->IsStringObject())){
			                    String::Utf8Value propertyName(propertyNameValue->ToString());
			                    string headerLine(*propertyName);
			                    String::Utf8Value value(propertyValue);
			                    headerLine.append(": ");
			                    headerLine.append(*value);
			                    headerList = curl_slist_append(headerList, headerLine.c_str());
			                }
			            }
			        }
			        
			        if(headerList!=NULL){
			            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
			        }
			        
			        /* send all data to this function  */
			        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
			        
			        /* we pass our 'chunk' struct to the callback function */
			        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

			        vector<string> headerLines;

			        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeaderCallback); // our static function
			        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void*)&headerLines); // "headers" is a member variable referencing HttpHeaders
			        
			        /* some servers don't like requests that are made without a user-agent
			         field, so we provide one */
			        curl_easy_setopt(curl, CURLOPT_USERAGENT, "trex/0.01");
			        
			        /* get it! */

			        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			   
			        curl_easy_perform(curl);
			        Handle<Object> response = Object::New();

			            Handle<Object> responseHeaders = Object::New();
			            for (size_t n = 0; n < headerLines.size(); n++){
			                size_t pos = headerLines[n].find(":");
			                if(pos!= string::npos){
			                    string key(headerLines[n].substr(0,pos));
			                    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			                    string value(headerLines[n].substr(pos+2,headerLines[n].size()-pos-2));
			                    std::string whitespaces ("\r\n");
			                    unsigned found = value.find_last_not_of(whitespaces);
												  if (found!=std::string::npos) {
												    value.erase(found+1, string::npos);
													}
			                    responseHeaders->Set(String::New(key.c_str()), String::New(value.c_str()));
			                }
			            }
			            response->Set(String::New("headers"), responseHeaders);
			        
			        if(chunk.memory&&(chunk.size>0)){
			            response->Set(String::New("body"), String::New(chunk.memory));
			        }

			        long responseCode;
			        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
			        response->Set(String::New("code"), Number::New(responseCode));

			        if(headerList!=NULL){
			            curl_slist_free_all(headerList);
			        }

			        curl_easy_cleanup(curl);

			        if(chunk.memory){
			            free(chunk.memory);
			        }

			        return scope.Close(response);
			}

			static Handle<Value> Post(const Arguments& args) {
			    if (args.Length() < 2) return v8::Undefined();
			    HandleScope scope;
			    Handle<Value> arg = args[0];
			    String::Utf8Value value(arg);
			    Handle<Value> dataArg = args[1];
			    String::Utf8Value data(dataArg);
			    
			    Handle<Object> headers;
			    if(args.Length()>=3){
			        Handle<Value> headerArg = args[2];
			        if(!headerArg->IsUndefined()){
			            headers = headerArg->ToObject();
			        }
			    }
			    
			    Handle<Object> response = Object::New();
			    
			    struct MemoryStruct chunk;
			    
			    chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
			    chunk.size = 0;    /* no data at this point */

			    CURL *curl;
			    
			    curl = curl_easy_init();
			    if(curl==NULL) {
			        return Undefined();
			    }

			        /* specify URL to get */
			        curl_easy_setopt(curl, CURLOPT_URL, *value);
			        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			        
			        struct curl_slist *headerList=NULL;
			        
			        /* headers */
			        if(!headers.IsEmpty()){
			            Handle<Array> propertyNames = headers->GetPropertyNames();
			            for(unsigned int i = 0; i < propertyNames->Length(); i++){
			                Handle<Value> propertyNameValue = propertyNames->Get(i);
			                Handle<Value> propertyValue = headers->GetRealNamedProperty(propertyNameValue->ToString());
			                if(!propertyValue->IsUndefined() && !propertyValue->IsNull() && (propertyValue->IsString() || propertyValue->IsStringObject())){
			                    String::Utf8Value propertyName(propertyNameValue->ToString());
			                    string headerLine(*propertyName);
			                    String::Utf8Value value(propertyValue);
			                    headerLine.append(": ");
			                    headerLine.append(*value);
			                    headerList = curl_slist_append(headerList, headerLine.c_str());
			                }
			            }
			        }
			        
			        if(headerList!=NULL){
			            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
			        }
			        
			        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, *data);
			        
			        /* send all data to this function  */
			        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
			        
			        /* we pass our 'chunk' struct to the callback function */
			        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
			        
			 vector<string> headerLines;

			        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeaderCallback); // our static function
			        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)&headerLines); // "headers" is a member variable referencing HttpHeaders
			        
			        /* some servers don't like requests that are made without a user-agent
			         field, so we provide one */
			        curl_easy_setopt(curl, CURLOPT_USERAGENT, "trex/0.01");
			        
			        /* post it! */
			        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			        curl_easy_perform(curl);
			        
			        /* we're done with libcurl, so clean it up */
			        if(chunk.memory){
			            response->Set(String::New("body"), String::New(chunk.memory));
			        }

			        Handle<Object> responseHeaders = Object::New();
			            for (size_t n = 0; n < headerLines.size(); n++){
			                size_t pos = headerLines[n].find(":");
			                if(pos!= string::npos){
			                    string key(headerLines[n].substr(0,pos));
			                    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			                    string value(headerLines[n].substr(pos+2,headerLines[n].size()-pos-2));
			                    std::string whitespaces ("\r\n");
			                    unsigned found = value.find_last_not_of(whitespaces);
												  if (found!=std::string::npos) {
												    value.erase(found+1, string::npos);
													}
			                    responseHeaders->Set(String::New(key.c_str()), String::New(value.c_str()));
			                }
			            }
			            response->Set(String::New("headers"), responseHeaders);
			        
			        long responseCode;
			        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
			        response->Set(String::New("code"), Number::New(responseCode));

			        /* cleanup curl stuff */
			        if(headerList!=NULL){
			            curl_slist_free_all(headerList);
			        }
			        curl_easy_cleanup(curl);
			        if(chunk.memory){
			            free(chunk.memory);
			        }
			        return scope.Close(response);
			}

			// http.request(method, url, headers, data)
			static Handle<Value> Request(const Arguments& args) {
			    if (args.Length() < 2) return v8::Undefined();
			    
			    HandleScope scope;

			    string method;
			    string url;
			    string data;
			    
			    String::Utf8Value methodValue(args[0]);
			    method = string(*methodValue);

					std::transform(method.begin(), method.end(), method.begin(), ::toupper);

			    String::Utf8Value urlValue(args[1]);
			    url = string(*urlValue);
			    
			    Handle<Object> headers;
			    if(args.Length()>=3){
			        Handle<Value> headerArg = args[2];
			        if(!headerArg->IsUndefined()){
			            headers = headerArg->ToObject();
			        }
			    }
			     
			    if(args.Length()>=4){
			        if(args[3]->IsString()){
			            String::Utf8Value dataValue(args[3]);
			            data = string(*dataValue);
			        }
			    }
			    
			    Handle<Object> response = Object::New();
			    
			    struct MemoryStruct chunk;
			    
			    chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
			    chunk.size = 0;    /* no data at this point */
			    
			    CURL *curl;
			    
			    curl = curl_easy_init();
			    
			    if(curl==NULL) {
			        return Undefined();
			    }

			        /* specify URL to get */
			        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			        
			        struct curl_slist *headerList=NULL;
			        
			        /* headers */
			        if(!headers.IsEmpty()){
			            Handle<Array> propertyNames = headers->GetPropertyNames();
			            for(unsigned int i = 0; i < propertyNames->Length(); i++){
			                Handle<Value> propertyNameValue = propertyNames->Get(i);
			                Handle<Value> propertyValue = headers->GetRealNamedProperty(propertyNameValue->ToString());
			                if(!propertyValue->IsUndefined() && !propertyValue->IsNull() && (propertyValue->IsString() || propertyValue->IsStringObject())){
			                    String::Utf8Value propertyName(propertyNameValue->ToString());
			                    string headerLine(*propertyName);
			                    String::Utf8Value value(propertyValue);
			                    headerLine.append(": ");
			                    headerLine.append(*value);
			                    headerList = curl_slist_append(headerList, headerLine.c_str());
			                }
			            }
			        }
			        
			        if(data.length()>0){
			            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
			        }
			        
			        // set method
			        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
							if(method.compare(string("HEAD"))==0){
								curl_easy_setopt(curl, CURLOPT_NOBODY,1L);
							}
			        
			        if(headerList!=NULL){
			            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
			        }
			        
			        /* send all data to this function  */
			        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
			        
			        /* we pass our 'chunk' struct to the callback function */
			        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
			         vector<string> headerLines;
			        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeaderCallback); // our static function
			        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)&headerLines); // "headers" is a member variable referencing HttpHeaders
			        
			        /* some servers don't like requests that are made without a user-agent
			         field, so we provide one */
			        curl_easy_setopt(curl, CURLOPT_USERAGENT, "trex/0.01");
			       
			        /* request it! */
			        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			        curl_easy_perform(curl);
			        
			        
			        /* we're done with libcurl, so clean it up */
			        if(chunk.memory && (chunk.size>0)){
			            response->Set(String::New("body"), String::New(chunk.memory, chunk.size));
			        }else{
									response->Set(String::New("body"), String::New(""));
			        }
			        
			        Handle<Object> responseHeaders = Object::New();
			            for (size_t n = 0; n < headerLines.size(); n++){
			                size_t pos = headerLines[n].find(":");
			                if(pos!= string::npos){
			                    string key(headerLines[n].substr(0,pos));
			                    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			                    string value(headerLines[n].substr(pos+2,headerLines[n].size()-pos-2));
			                    std::string whitespaces ("\r\n");
			                    unsigned found = value.find_last_not_of(whitespaces);
												  if (found!=std::string::npos) {
												    value.erase(found+1, string::npos);
													}
			                    responseHeaders->Set(String::New(key.c_str()), String::New(value.c_str()));
			                }
			            }
			            response->Set(String::New("headers"), responseHeaders);

			                /* we're done with libcurl, so clean it up */
			        long responseCode;
			        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
			        response->Set(String::New("code"), Number::New(responseCode));
			        
			        /* cleanup curl stuff */
			        if(headerList!=NULL){
			            curl_slist_free_all(headerList);
			        }
			        
			        curl_easy_cleanup(curl);

			        if(chunk.memory){
			            free(chunk.memory);
			        }
			        
			        return scope.Close(response);
			}
		}
	}
}

#endif

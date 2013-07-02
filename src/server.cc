#include <unistd.h>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <libxml/xmlmemory.h>
#include <signal.h>
#include <curl/curl.h>
#include <v8.h>
#include <vector>
#include <ctime>
#include "mongoose/mongoose.h"
#include "trex/trex.h"

using namespace std;

#define MAX_OPTIONS 1

static int exit_flag;
static void signal_handler(int sig_num) {
  exit_flag = sig_num;
}

static time_t lastRequest = 0;
static map<long,time_t> threadHandlerTime;

static int begin_request_handler(struct mg_connection *conn) {
    long threadId = (long)pthread_self();

    lastRequest = std::time(NULL);
    
    Trex::Request* request = new Trex::Request(conn);
    Trex::Runtime* threadRuntime = Trex::Runtime::instance();
    Trex::RequestHandler* handler = new Trex::RequestHandler(threadRuntime);

    threadHandlerTime[threadId] = std::time(NULL);

    Trex::Response* response = handler->process(request);

    threadHandlerTime[threadId] = -1;

    string headers;
    map<string,string> trexHeaders = response->headers();
    map<string,string>::iterator it;
    for (map<string,string>::iterator it=trexHeaders.begin(); it!=trexHeaders.end(); ++it){
        headers.append(it->first);
        headers.append(": ");
        headers.append(it->second);
        headers.append("\r\n");
    }
    
    mg_printf(conn,
              "HTTP/1.1 %d OK\r\n"
              "%s"
              "\r\n"
              "%s",
              response->code(), headers.c_str(), response->body().c_str());

    if(threadRuntime->shouldStop()){
        Trex::Runtime::killRuntime(threadId, threadRuntime);
    }
    
    delete response;
    delete handler;
    delete request;

    return 1;
}

static size_t 
dummyWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
   return size * nmemb;
}
static size_t
headerCheckCallback(void *contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;

    vector<string>* headerArray = (vector<string>*)userp;
    headerArray->push_back(string((char*)contents, realsize));

    return realsize;
}

static char* trexOpml;
static char* trexOpmlEtag = NULL;
static time_t lastCheck = 0;
static int xml_memory_used = 0;

void checkScripts(){
    map<long,time_t>::iterator it;
    time_t now = std::time(NULL);
    for (map<long,time_t>::iterator it=threadHandlerTime.begin(); it!=threadHandlerTime.end(); ++it){
        long threadId = it->first;
        time_t handleStart = it->second;
        if((handleStart > 0) && ((now - (handleStart)) > 10)){
            Trex::Runtime* threadRuntime = Trex::Runtime::instanceForThreadId(threadId);
            if(threadRuntime!=NULL){
                threadRuntime->terminateExecution();
            }
            threadHandlerTime[threadId] = -1;
            
        }
    }
}

void checkOPML(){
    time_t now = std::time(NULL);
    if(((now - lastCheck) > 10) && ((now - lastRequest) < 30)){
        CURL *curl = curl_easy_init();
        if(curl!=NULL) {
            curl_easy_setopt(curl, CURLOPT_URL, trexOpml);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            string trexDir(getenv("TREX_DIR"));
            trexDir.append("/deps/cacert.pem");
            curl_easy_setopt (curl, CURLOPT_CAINFO, trexDir.c_str());
            
            struct curl_slist *headerList=NULL;
            if(trexOpmlEtag!=NULL){
                string headerLine("If-None-Match: ");
                headerLine.append(trexOpmlEtag);
                headerList = curl_slist_append(headerList, headerLine.c_str());
            }
            if(headerList!=NULL){
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
            }
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummyWriteCallback);
            vector<string> headerLines;
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCheckCallback); // our static function
            curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)&headerLines); // "headers" is a member variable referencing HttpHeaders        
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "trex/0.01");
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
            curl_easy_perform(curl);
            long responseCode;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
            if(responseCode==200){
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
                        if(key.compare("etag")==0){
                            bool firstTime = (trexOpmlEtag==NULL);
                            trexOpmlEtag = (char*)malloc(value.size());
                            strcpy(trexOpmlEtag, value.c_str());
                            if(!firstTime){
                                Trex::Runtime* testRuntime = new Trex::Runtime();
                                if(!testRuntime->compileError){
                                    xml_memory_used = xmlMemUsed();
                                    Trex::Runtime::restart();
                                }
                                delete testRuntime;
                            }
                            break;
                        }
                    }
                }
            }
            if(headerList!=NULL){
                curl_slist_free_all(headerList);
            }  
            curl_easy_cleanup(curl);
            lastCheck = std::time(NULL);
        }
    } 
}

// wrapper for xmlMemMalloc to update v8's knowledge of memory used
// the GC relies on this information
void* xmlMemMallocWrap(size_t size)
{
    void* res = xmlMemMalloc(size);

    // no need to udpate memory if we didn't allocate
    if (!res)
    {
        return res;
    }

    const int diff = xmlMemUsed() - xml_memory_used;
    xml_memory_used += diff;
    v8::V8::AdjustAmountOfExternalAllocatedMemory(diff);

    return res;
}

// wrapper for xmlMemFree to update v8's knowledge of memory used
// the GC relies on this information
void xmlMemFreeWrap(void* p)
{
    xmlMemFree(p);

    // if v8 is no longer running, don't try to adjust memory
    // this happens when the v8 vm is shutdown and the program is exiting
    // our cleanup routines for libxml will be called (freeing memory)
    // but v8 is already offline and does not need to be informed
    // trying to adjust after shutdown will result in a fatal error
    if (v8::V8::IsDead())
    {
        return;
    }

    const int diff = xmlMemUsed() - xml_memory_used;

    xml_memory_used += diff;

    v8::V8::AdjustAmountOfExternalAllocatedMemory(diff);
}

// wrapper for xmlMemRealloc to update v8's knowledge of memory used
void* xmlMemReallocWrap(void* ptr, size_t size)
{
    void* res = xmlMemRealloc(ptr, size);

    // if realloc fails, no need to update v8 memory state
    if (!res)
    {
        return res;
    }

    const int diff = xmlMemUsed() - xml_memory_used;
    xml_memory_used += diff;
    v8::V8::AdjustAmountOfExternalAllocatedMemory(diff);

    return res;
}

// wrapper for xmlMemoryStrdupWrap to update v8's knowledge of memory used
char* xmlMemoryStrdupWrap(const char* str)
{
    char* res = xmlMemoryStrdup(str);

    // if strdup fails, no need to update v8 memory state
    if (!res)
    {
        return res;
    }

    const int diff = xmlMemUsed() - xml_memory_used;
    xml_memory_used += diff;
    v8::V8::AdjustAmountOfExternalAllocatedMemory(diff);

    return res;
}

int main(int argc, char **argv) {
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    if(argc>=2){
        trexOpml = argv[1];
        setenv("TREX_OPML", argv[1], strlen(argv[1]));
        cout << "TREX_OPML: " << trexOpml << endl << flush;
    }else{
        cerr << "Trex OPML must be specified" << endl << flush;
        cerr << "Usage: trex <OPMLURL>" << endl << flush;
        exit(1);
    }

    LIBXML_TEST_VERSION;
    xmlInitParser();
    xml_memory_used = xmlMemUsed();
    xmlMemSetup(xmlMemFreeWrap, xmlMemMallocWrap, xmlMemReallocWrap, xmlMemoryStrdupWrap);

    Trex::Db::instance();

    char* trexDir;
    trexDir = getenv("TREX_DIR");
    if(trexDir==NULL){
      trexDir = ".";
      setenv("TREX_DIR",".",1);
    }
    cout << "TREX_DIR: " << trexDir << endl << flush;

    char* trexEnv;
    trexEnv = getenv("TREX_ENV");
    if(trexEnv==NULL){
      trexEnv = "development";
    }
    cout << "TREX_ENV: " << trexEnv << endl << flush;

    curl_global_init(CURL_GLOBAL_ALL);
    
    v8::V8::SetFlagsFromString("--harmony", 9);

    struct mg_context *ctx;
    struct mg_callbacks callbacks;
    
    char* port;
    port = getenv("TREX_PORT");
    if(port==NULL){
      port = "8080";
    }

    cout << "TREX_PORT: " << port << endl << flush;

    char* num_threads;
    num_threads = getenv("TREX_THREADS");
    if(num_threads==NULL){
      num_threads = "4";
    }
    
    cout << "TREX_THREADS: " << num_threads << endl << flush;

    const char *options[] = {"listening_ports", port, "num_threads", num_threads, NULL};
    
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.begin_request = begin_request_handler;

    ctx = mg_start(&callbacks, NULL, options);

    while (exit_flag == 0) {
        sleep(1);  
        checkScripts();
        checkOPML(); 
    }
    
    mg_stop(ctx);
    
    delete Trex::Db::instance();
    
    Trex::Runtime::cleanup();

    curl_global_cleanup();
    
    return 0;
}

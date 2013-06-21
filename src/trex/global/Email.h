#ifndef __TrexEmail__
#define __TrexEmail__

#include <iostream>
#include <v8.h>
#include <curl/curl.h>
#include <vector>

using namespace v8;

namespace Trex{
	namespace Global {
		namespace Email {
			static void split(const string& str, const string& delimiters , vector<string>& tokens)
			{
			    // Skip delimiters at beginning.
			    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
			    // Find first "non-delimiter".
			    string::size_type pos     = str.find_first_of(delimiters, lastPos);

			    while (string::npos != pos || string::npos != lastPos)
			    {
			        // Found a token, add it to the vector.
			    		string s = str.substr(lastPos, pos - lastPos);
			    		s.append("\n");
			        tokens.push_back(s);
			        // Skip delimiters.  Note the "not_of"
			        lastPos = str.find_first_not_of(delimiters, pos);
			        // Find next "non-delimiter"
			        pos = str.find_first_of(delimiters, lastPos);
			    }
			}
 
			struct upload_status {
			  int lines_read;
			  vector<string> lines;
			};
 
			static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
			{
			  struct upload_status *upload_ctx = (struct upload_status *)userp;
			 
			  if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
			    return 0;
			  }
			 
			  if (upload_ctx->lines_read < upload_ctx->lines.size()) {
			  	string line = upload_ctx->lines[upload_ctx->lines_read];
			  	cout << line << flush;
			  	const char* data = line.c_str();
			    size_t len = strlen(data);
			    memcpy(ptr, data, len);
			    upload_ctx->lines_read ++;
			    return len;
			  }
			  return 0;
			}

			// email.send(host,port,user,pass,to,from,subject,body)
			static Handle<Value> Send(const Arguments& args) {
				if (args.Length() < 8) return Undefined();

			  HandleScope scope;

			  String::Utf8Value host(args[0]);
			  String::Utf8Value port(args[1]);
			  String::Utf8Value user(args[2]);
			  String::Utf8Value pass(args[3]);
			  String::Utf8Value to(args[4]);
			  String::Utf8Value from(args[5]);
			  String::Utf8Value subject(args[6]);
			  String::Utf8Value body(args[7]);

				CURL *curl;
			  CURLcode res;
			  struct curl_slist *recipients = NULL;
			  struct upload_status upload_ctx;
			 
			  upload_ctx.lines_read = 0;
			 
			  curl = curl_easy_init();
			  if (curl) {
			  	ostringstream url;
			  	url << "smtp://" << *host << ":" << *port;
			  	string _to("To: <");
			  	_to.append(*to);
			  	_to.append(">\n");
			  	upload_ctx.lines.push_back(_to);
			  	string _from("From: <");
			  	_from.append(*from);
			  	_from.append(">\n");
			  	upload_ctx.lines.push_back(_from);
			  	string _subject("Subject: ");
			  	_subject.append(*subject);
			  	_subject.append("\n");
			  	upload_ctx.lines.push_back(_subject);
			  	upload_ctx.lines.push_back("\n");
			  	split(string(*body),string("\n"), upload_ctx.lines);
			  	upload_ctx.lines.push_back("\r\n");
			  	upload_ctx.lines.push_back("\r\n");
			  	
			    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
			    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
			    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			    curl_easy_setopt(curl, CURLOPT_USERNAME, *user);
			    curl_easy_setopt(curl, CURLOPT_PASSWORD, *pass);
			    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, *from);
			    recipients = curl_slist_append(recipients, *to);
			    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
			    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
			    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
			    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			    res = curl_easy_perform(curl);
			    if(res != CURLE_OK){
			    	return Undefined();
			    }
			    curl_slist_free_all(recipients);
			    curl_easy_cleanup(curl);
			  }

			  return True();
			}
		}
	}
}

#endif

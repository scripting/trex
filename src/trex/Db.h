#ifndef __TrexDb__
#define __TrexDb__

#include <leveldb/db.h>
#include <leveldb/cache.h>
#include <iostream>
#include <string>

using namespace std;

namespace Trex {

	class Db{
	private:
	    
	    leveldb::DB* db;
	    leveldb::Options options;
	public:
	    static Db* instance();
	    virtual ~Db();
	    bool Put(string key, string value);
	    string Get(string key);
	    bool Delete(string key);
	protected:
	    
	    explicit Db();
	};

}

#endif

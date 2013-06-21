#include "Db.h"
#include <assert.h>
#include <iostream>
#include <cstdlib>

using namespace std;

namespace Trex {

    Db::Db(){
        options.create_if_missing=true;
        options.block_cache = leveldb::NewLRUCache(100 * 1048576);  // 100MB cache
        char* dbDir;
        dbDir = getenv("TREX_DB");
        if(dbDir==NULL){
            dbDir="./trexdb";
        }
        leveldb::Status status = leveldb::DB::Open(options, dbDir, &db);
        cout << "TREX_DB: " << dbDir << endl << flush;
        if (!status.ok()) cerr << status.ToString() << endl;
    }

    Db::~Db(){
        delete options.block_cache;
        delete db;
    }

    bool Db::Put(string key, string value){
        leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
        return s.ok();
    }

    string Db::Get(string key){
        std::string value;
        leveldb::Status s = db->Get(leveldb::ReadOptions(), key, &value);
        if(s.ok()){
            return value;
        }
        return value;
    }

    bool Db::Delete(string key){
        leveldb::Status s = db->Delete(leveldb::WriteOptions(), key);
        return s.ok();
    }
    static Db* _instance;
    Db* Db::instance(){
        if(_instance==NULL){
            _instance = new Db();
        }
        return _instance;
    }

}
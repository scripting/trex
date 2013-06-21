var db = {};
db.get = function(key){
    var value = leveldb.get(key);
    if(value!=null){
        return JSON.parse(value);
    }
    return undefined;
};
db.put = function(key,value){
    return leveldb.put(key,JSON.stringify(value));
};
db.delete = function(key){
    return leveldb.delete(key);
};
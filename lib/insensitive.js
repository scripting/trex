function InsensitiveObject(){
	var forwarder = new ForwardingHandler(new Object());
	forwarder.canoncialToOriginalKeys = new Object();
	forwarder.enumerate = function() {
		var result = [];
		for (var name in this.target) { result.push(this.canoncialToOriginalKeys[name]); };
		return result;
		};
	forwarder.get = function(rcvr, name, val){
		return this.target[name.toLowerCase()];
		};
	forwarder.has = function(name) { return name.toLowerCase() in this.target; },
	forwarder.hasOwn = function(name) { return ({}).hasOwnProperty.call(this.target, name.toLowerCase()); },
	forwarder.set = function(rcvr, name, val){
		this.target[name.toLowerCase()] = val;
		this.canoncialToOriginalKeys[name.toLowerCase()] = name;
		return true;
		};
	return Proxy.create(forwarder, ForwardingHandler.prototype);
	}

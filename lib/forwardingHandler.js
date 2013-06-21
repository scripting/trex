function ForwardingHandler(target) {
	this.target = target;
	}
ForwardingHandler.prototype = {
	// Object.getOwnPropertyDescriptor(proxy, name) -> pd | undefined
		getOwnPropertyDescriptor: function(name) {
			var desc = Object.getOwnPropertyDescriptor(this.target, name);
			return desc;
			},
	// Object.getPropertyDescriptor(proxy, name) -> pd | undefined
		getPropertyDescriptor: function(name) {
			var desc = Object.getOwnPropertyDescriptor(this.target, name);
			var parent = Object.getPrototypeOf(this.target);
			while (desc === undefined && parent !== null) {
				desc = Object.getOwnPropertyDescriptor(parent, name);
				parent = Object.getPrototypeOf(parent);
				}
			return desc;
			},
	// Object.getOwnPropertyNames(proxy) -> [ string ]
		getOwnPropertyNames: function() {
			return Object.getOwnPropertyNames(this.target);
			},
	// Object.getPropertyNames(proxy) -> [ string ]
		getPropertyNames: function() {
			var props = Object.getOwnPropertyNames(this.target);
			var parent = Object.getPrototypeOf(this.target);
			while (parent !== null) {
				props = props.concat(Object.getOwnPropertyNames(parent));
				parent = Object.getPrototypeOf(parent);
				}
			return props;
			},
	// Object.defineProperty(proxy, name, pd) -> boolean
		defineProperty: function(name, desc) {
			Object.defineProperty(this.target, name, desc);
			return true;
			},
	// delete proxy[name] -> boolean
		'delete': function(name) { return delete this.target[name]; },
	// Object.{freeze|seal|preventExtensions}(proxy) -> proxy
		fix: function() {
			// As long as target is not frozen, the proxy won't allow itself to be fixed
			if (!Object.isFrozen(this.target))
				return undefined;
			var props = {};
			var handler = this;
			Object.getOwnPropertyNames(this.target).forEach(function (name) {
				var desc = Object.getOwnPropertyDescriptor(handler.target, name);
				// turn descriptor into a trapping accessor property
				props[name] = {
					get: function( ) { return handler.get(this, name); },
					set: function(v) { return handler.set(this, name, v); },
					enumerable: desc.enumerable,
					configurable: desc.configurable
					};
				});
			return props;
			},
	// name in proxy -> boolean
		has: function(name) { return name in this.target; },
	// ({}).hasOwnProperty.call(proxy, name) -> boolean
		hasOwn: function(name) { return ({}).hasOwnProperty.call(this.target, name); },
	// proxy[name] -> any
		get: function(receiver, name) { return this.target[name]; },
	// proxy[name] = val -> val
		set: function(receiver, name, val) {
			this.target[name] = val;
			// bad behavior when set fails in non-strict mode
			return true;
			},
	// for (var name in Object.create(proxy)) { ... }
		enumerate: function() {
			var result = [];
			for (var name in this.target) { result.push(name); };
			return result;
			},
	// for (var name in proxy) { ... }
		// Note: non-standard trap
		iterate: function() {
			var props = this.enumerate();
			var i = 0;
			return {
				next: function() {
					if (i === props.length) throw StopIteration;
					return props[i++];
				}
				};
			},
	// Object.keys(proxy) -> [ string ]
		keys: function() { return Object.keys(this.target); }
	};
// monkey-patch Proxy.Handler if it's not defined yet
	if (typeof Proxy === "object" && !Proxy.Handler) {
		Proxy.Handler = ForwardingHandler;
		}

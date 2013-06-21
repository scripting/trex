// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

var isArray = Array.isArray;
var domain;

function EventEmitter() {
    this.domain = null;
    if (exports.usingDomains) {
        // if there is an active domain, then attach to it.
        domain = domain || require('domain');
        if (domain.active && !(this instanceof domain.Domain)) {
            this.domain = domain.active;
        }
    }
    this._events = this._events || null;
    this._maxListeners = this._maxListeners || defaultMaxListeners;
}

// By default EventEmitters will print a warning if more than
// 10 listeners are added to it. This is a useful default which
// helps finding memory leaks.
//
// Obviously not all Emitters should be limited to 10. This function allows
// that to be increased. Set to zero for unlimited.
var defaultMaxListeners = 10;
EventEmitter.prototype.setMaxListeners = function(n) {
    this._maxListeners = n;
};

// non-global reference, for speed.
var PROCESS;

EventEmitter.prototype.emit = function(type) {
    // If there is no 'error' event listener then throw.
    if (type === 'error') {
        if (!this._events || !this._events.error ||
            (isArray(this._events.error) && !this._events.error.length))
        {
            if (this.domain) {
                var er = arguments[1];
                er.domainEmitter = this;
                er.domain = this.domain;
                er.domainThrown = false;
                this.domain.emit('error', er);
                return false;
            }
            
            if (arguments[1] instanceof Error) {
                throw arguments[1]; // Unhandled 'error' event
            } else {
                throw new Error("Uncaught, unspecified 'error' event.");
            }
            return false;
        }
    }
    
    if (!this._events) return false;
    var handler = this._events[type];
    if (!handler) return false;
    
    if (typeof handler == 'function') {
        if (this.domain) {
            PROCESS = PROCESS || process;
            if (this !== PROCESS) {
                this.domain.enter();
            }
        }
        switch (arguments.length) {
                // fast cases
            case 1:
                handler.call(this);
                break;
            case 2:
                handler.call(this, arguments[1]);
                break;
            case 3:
                handler.call(this, arguments[1], arguments[2]);
                break;
                // slower
            default:
                var l = arguments.length;
                var args = new Array(l - 1);
                for (var i = 1; i < l; i++) args[i - 1] = arguments[i];
                handler.apply(this, args);
        }
        if (this.domain && this !== PROCESS) {
            this.domain.exit();
        }
        return true;
        
    } else if (isArray(handler)) {
        if (this.domain) {
            PROCESS = PROCESS || process;
            if (this !== PROCESS) {
                this.domain.enter();
            }
        }
        var l = arguments.length;
        var args = new Array(l - 1);
        for (var i = 1; i < l; i++) args[i - 1] = arguments[i];
        
        var listeners = handler.slice();
        for (var i = 0, l = listeners.length; i < l; i++) {
            listeners[i].apply(this, args);
        }
        if (this.domain && this !== PROCESS) {
            this.domain.exit();
        }
        return true;
        
    } else {
        return false;
    }
};

EventEmitter.prototype.addListener = function(type, listener) {
    if ('function' !== typeof listener) {
        throw new Error('addListener only takes instances of Function');
    }
    
    if (!this._events) this._events = {};
    
    // To avoid recursion in the case that type == "newListener"! Before
    // adding it to the listeners, first emit "newListener".
    if (this._events.newListener) {
        this.emit('newListener', type, typeof listener.listener === 'function' ?
                  listener.listener : listener);
    }
    
    if (!this._events[type]) {
        // Optimize the case of one listener. Don't need the extra array object.
        this._events[type] = listener;
    } else if (isArray(this._events[type])) {
        
        // If we've already got an array, just append.
        this._events[type].push(listener);
        
    } else {
        // Adding the second element, need to change to array.
        this._events[type] = [this._events[type], listener];
        
    }
    
    // Check for listener leak
    if (isArray(this._events[type]) && !this._events[type].warned) {
        var m;
        m = this._maxListeners;
        
        if (m && m > 0 && this._events[type].length > m) {
            this._events[type].warned = true;
            console.error('(node) warning: possible EventEmitter memory ' +
                          'leak detected. %d listeners added. ' +
                          'Use emitter.setMaxListeners() to increase limit.',
                          this._events[type].length);
            console.trace();
        }
    }
    
    return this;
};

EventEmitter.prototype.on = EventEmitter.prototype.addListener;

EventEmitter.prototype.once = function(type, listener) {
    if ('function' !== typeof listener) {
        throw new Error('.once only takes instances of Function');
    }
    
    var self = this;
    function g() {
        self.removeListener(type, g);
        listener.apply(this, arguments);
    };
    
    g.listener = listener;
    self.on(type, g);
    
    return this;
};

// emits a 'removeListener' event iff the listener was removed
EventEmitter.prototype.removeListener = function(type, listener) {
    if ('function' !== typeof listener) {
        throw new Error('removeListener only takes instances of Function');
    }
    
    // does not use listeners(), so no side effect of creating _events[type]
    if (!this._events || !this._events[type]) return this;
    
    var list = this._events[type];
    
    if (isArray(list)) {
        var position = -1;
        for (var i = 0, length = list.length; i < length; i++) {
            if (list[i] === listener ||
                (list[i].listener && list[i].listener === listener))
            {
                position = i;
                break;
            }
        }
        
        if (position < 0) return this;
        list.splice(position, 1);
        if (list.length == 0)
            this._events[type] = null;
        
        if (this._events.removeListener) {
            this.emit('removeListener', type, listener);
        }
    } else if (list === listener ||
               (list.listener && list.listener === listener))
    {
        this._events[type] = null;
        
        if (this._events.removeListener) {
            this.emit('removeListener', type, listener);
        }
    }
    
    return this;
};

EventEmitter.prototype.removeAllListeners = function(type) {
    if (!this._events) return this;
    
    // fast path
    if (!this._events.removeListener) {
        if (arguments.length === 0) {
            this._events = {};
        } else if (type && this._events && this._events[type]) {
            this._events[type] = null;
        }
        return this;
    }
    
    // slow(ish) path, emit 'removeListener' events for all removals
    if (arguments.length === 0) {
        for (var key in this._events) {
            if (key === 'removeListener') continue;
            this.removeAllListeners(key);
        }
        this.removeAllListeners('removeListener');
        this._events = {};
        return this;
    }
    
    var listeners = this._events[type];
    if (isArray(listeners)) {
        while (listeners.length) {
            // LIFO order
            this.removeListener(type, listeners[listeners.length - 1]);
        }
    } else if (listeners) {
        this.removeListener(type, listeners);
    }
    this._events[type] = null;
    
    return this;
};

EventEmitter.prototype.listeners = function(type) {
    if (!this._events || !this._events[type]) return [];
    if (!isArray(this._events[type])) {
        return [this._events[type]];
    }
    return this._events[type].slice(0);
};

var events = {};
events.EventEmitter = EventEmitter;

var util = {};

util.inherits = function(ctor, superCtor) {
        ctor.super_ = superCtor;
        ctor.prototype = Object.create(superCtor.prototype, {
                                       constructor: {
                                       value: ctor,
                                       enumerable: false,
                                       writable: true,
                                       configurable: true
                                       }
                                       });
};

var SaxParser = function(callbacks) {
    var parser = new libxml.SaxParser();
    
    // attach callbacks
    for (var callback in callbacks) {
        parser.on(callback, callbacks[callback]);
    }
    
    return parser;
};

// store existing functions because util.inherits overrides the prototype
var parseString = libxml.SaxParser.prototype.parseString;

util.inherits(libxml.SaxParser, events.EventEmitter);

libxml.SaxParser.prototype.parseString = parseString;

var SaxPushParser = function(callbacks) {
    var parser = new libxml.SaxPushParser();
    
    // attach callbacks
    for (var callback in callbacks) {
        parser.on(callback, callbacks[callback]);
    }
    
    return parser;
};

var push = libxml.SaxPushParser.prototype.push;

util.inherits(libxml.SaxPushParser, events.EventEmitter);

libxml.SaxPushParser.prototype.push = push;

/// create a new element on the given document
/// @param doc the Document to create the element for
/// @param name the element name
/// @param {String} [contenn] element content
/// @constructor
var Element = function(doc, name, content) {
    if (!doc) {
        throw new Error('document argument required');
    } else if (! (doc instanceof libxml.Document)) {
        throw new Error('document argument must be an ' +
                        'instance of Document');
    } else if (!name) {
        throw new Error('name argument required');
    }
    
    var elem = new libxml.Element(doc, name, content);
    return elem;
};

Element.prototype = libxml.Element.prototype;

Element.prototype.attr = function() {
    if (arguments.length === 1) {
        var arg = arguments[0];
        if (typeof arg === 'object') {
            // object setter
            // iterate keys/value to set attributes
            for (var k in arg) {
                this._attr(k, arg[k]);
            };
            return this;
        } else if (typeof arg === 'string') {
            // getter
            return this._attr(arg);
        }
    } else if (arguments.length === 2) {
        // 2 arg setter
        var name = arguments[0];
        var value = arguments[1];
        this._attr(name, value);
        return this;
    }
};

/// helper method to attach a new node to this element
/// @param name the element name
/// @param {String} [content] element content
Element.prototype.node = function(name, content) {
    var elem = Element(this.doc(), name, content);
    this.addChild(elem);
    return elem;
};

/// helper method to attach a cdata to this element
/// @param name the element name
/// @param {String} [content] element content
Element.prototype.cdata = function(content) {
    this.addCData(content);
    return this;
};

Element.prototype.get = function() {
    var res = this.find.apply(this, arguments);
    if (res instanceof Array) {
        return res[0];
    } else {
        return res;
    }
};

Element.prototype.defineNamespace = function(prefix, href) {
    // if no prefix specified
    if (!href) {
        href = prefix;
        prefix = null;
    }
    return new libxml.Namespace(this, prefix, href);
};

/// Create a new document
/// @param {string} version xml version, default 1.0
/// @param {string} encoding the encoding, default utf8
/// @constructor
var Document = function(version, encoding) {
    version = version || '1.0';
    var doc = new libxml.Document(version);
    doc.encoding(encoding || 'utf8');
    return doc;
};

Document.prototype = libxml.Document.prototype;

/// get or set the root element
/// if called without any arguments, this will return the document root
/// @param {Element} [elem] if specified, this will become the new document root
Document.prototype.root = function(elem) {
    return this._root(elem);
};

/// add a child node to the document
/// this will set the document root
Document.prototype.node = function(name, content) {
    return this.root(Element(this, name, content));
};

/// xpath search
/// @return array of matching elements
Document.prototype.find = function(xpath, ns_uri) {
    return this.root().find(xpath, ns_uri);
};

/// xpath search
/// @return first element matching
Document.prototype.get = function(xpath, ns_uri) {
    return this.root().get(xpath, ns_uri);
};

/// @return a given child
Document.prototype.child = function(id) {
    if (id === undefined || typeof id !== 'number') {
        throw new Error('id argument required for #child');
    }
    return this.root().child(id);
};

/// @return an Array of child nodes of the document root
Document.prototype.childNodes = function() {
    return this.root().childNodes();
};

/// @return a string representation of the document
Document.prototype.toString = function(formatted) {
    return this._toString(formatted !== undefined ? formatted : true);
};

/// @return the document version
Document.prototype.version = function() {
    return this._version();
};

/// @return the document encoding
Document.prototype.encoding = function(encoding) {
    return this._encoding(encoding);
};

/// @return whether the XmlDocument is valid
Document.prototype.validate = function(xsd) {
    return this._validate(xsd);
};

Document.prototype.setDtd = function(name, ext, sys) {
    if (!name) {
        throw new Error('Must pass in a DTD name');
    } else if (typeof name !== 'string') {
        throw new Error('Must pass in a valid DTD name');
    }
    
    var params = [name];
    if (typeof ext !== 'undefined') {
        params.push(ext);
    }
    if (ext && typeof sys !== 'undefined') {
        params.push(sys);
    }
    
    return this._setDtd.apply(this, params);
};

/// @return array of namespaces in document
Document.prototype.namespaces = function() {
    return this.root().namespaces();
};

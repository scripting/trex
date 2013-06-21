// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_LIBXMLJS_H_
#define SRC_LIBXMLJS_H_

#include <v8.h>
#include "node_object_wrap.h"
#include <vector>

namespace node{

#define NODE_PSYMBOL(s) \
v8::Persistent<v8::String>::New(v8::String::NewSymbol(s))

    template <typename target_t>
    void SetMethod(target_t obj, const char* name,
                   v8::InvocationCallback callback)
    {
        obj->Set(v8::String::NewSymbol(name),
                 v8::FunctionTemplate::New(callback)->GetFunction());
    }
    
template <typename target_t>
void SetPrototypeMethod(target_t target,
                        const char* name, v8::InvocationCallback callback)
{
    v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(callback);
    target->PrototypeTemplate()->Set(v8::String::NewSymbol(name), templ);
}

#define NODE_SET_METHOD node::SetMethod
#define NODE_SET_PROTOTYPE_METHOD node::SetPrototypeMethod
}

#define LIBXMLJS_ARGUMENT_TYPE_CHECK(arg, type, err)                          \
  if (!arg->type()) {                                                         \
    v8::Local<v8::Value> exception = v8::Exception::TypeError(                \
      v8::String::New(err));                                                  \
    return v8::ThrowException(exception);                                     \
  }

namespace libxmljs {

// convenience function to create a v8 error wrapped by ThrowException
v8::Handle<v8::Value> ThrowError(const char* msg);

#ifdef LIBXML_DEBUG_ENABLED
static const bool debugging = true;
#else
static const bool debugging = false;
#endif

// Ensure that libxml is properly initialised and destructed at shutdown
class LibXMLJS {
public:
    std::vector<void*> docs;
    LibXMLJS(v8::Handle<v8::Object> target);
    virtual ~LibXMLJS();
    void cleanup();
    v8::Persistent<v8::FunctionTemplate> document_constructor_template;
    v8::Persistent<v8::FunctionTemplate> attribute_constructor_template;
    v8::Persistent<v8::FunctionTemplate> element_constructor_template;
    v8::Persistent<v8::FunctionTemplate> node_constructor_template;
    v8::Persistent<v8::FunctionTemplate> namespace_constructor_template;
    v8::Persistent<v8::FunctionTemplate> sax_parser_template;
    v8::Persistent<v8::FunctionTemplate> sax_push_parser_template;
};

}  // namespace libxmljs

#endif  // SRC_LIBXMLJS_H_

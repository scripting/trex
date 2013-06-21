// Copyright 2009, Squish Tech, LLC.

#include <v8.h>

#include <libxml/xmlmemory.h>

#include "libxmljs.h"
#include "xml_document.h"
#include "xml_node.h"
#include "xml_sax_parser.h"
#include "xml_element.h"
#include "xml_attribute.h"

namespace libxmljs {

v8::Handle<v8::Value> ThrowError(const char* msg)
{
    return v8::ThrowException(v8::Exception::Error(v8::String::New(msg)));
}

LibXMLJS::LibXMLJS(v8::Handle<v8::Object> target)
{
    v8::HandleScope scope(v8::Isolate::GetCurrent());
    
    XmlDocument::Initialize(document_constructor_template, target);
    XmlSaxParser::Initialize(sax_parser_template, sax_push_parser_template, target);
    
    target->Set(v8::String::NewSymbol("libxml_version"),
                v8::String::New(LIBXML_DOTTED_VERSION));
    
    target->Set(v8::String::NewSymbol("libxml_parser_version"),
                v8::String::New(xmlParserVersion));
    
    target->Set(v8::String::NewSymbol("libxml_debug_enabled"),
                v8::Boolean::New(debugging));
    
    target->Set(v8::String::NewSymbol("libxml"), target);
    
    XmlNode::Initialize(node_constructor_template, target);
    XmlNamespace::Initialize(namespace_constructor_template, target);
    XmlElement::Initialize(element_constructor_template, node_constructor_template, target);
    XmlAttribute::Initialize(attribute_constructor_template, node_constructor_template, target);
}

LibXMLJS::~LibXMLJS()
{
    document_constructor_template.Dispose();
    attribute_constructor_template.Dispose();
    element_constructor_template.Dispose();
    node_constructor_template.Dispose();
    namespace_constructor_template.Dispose();
    sax_parser_template.Dispose();
    sax_push_parser_template.Dispose();
}

void LibXMLJS::cleanup(){

}

}  // namespace libxmljs

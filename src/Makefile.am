AM_CFLAGS = -std=c99 -O2 -W -Wall -pedantic -pthread

bin_PROGRAMS = trex
trex_SOURCES = server.cc mongoose/mongoose.c trex/Db.cpp trex/Request.cpp trex/RequestHandler.cpp trex/Response.cpp trex/Runtime.cpp libxmljs/libxmljs.cc libxmljs/html_document.cc libxmljs/xml_attribute.cc libxmljs/xml_document.cc libxmljs/xml_element.cc libxmljs/xml_namespace.cc libxmljs/xml_node.cc libxmljs/xml_sax_parser.cc libxmljs/xml_syntax_error.cc libxmljs/xml_xpath_context.cc
trex_LDADD = -lpthread -lcurl -lxml2 -lleveldb -lv8 -lssl -lcrypto -lz -lm -ldl
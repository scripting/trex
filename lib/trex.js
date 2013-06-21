/** INCLUDES **/
include("forwardingHandler.js");
include("insensitive.js");
include("libxml.js");
include("db.js");
include("server.js");
include("xml.js");
include("op.js");

var bootOPML = op.getNetOutline(TREX_OPML);
op.xmlToText(bootOPML);
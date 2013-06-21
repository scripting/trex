var xml = {};
xml.parse = function(xmltext){
	return libxml.fromXml(xmltext);
}
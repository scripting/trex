var op = {};
op.getNetOutline = function(urlOutline, flKeepXml, flAskForOpml, minsecs){
	var flNeedXml = false;
	if(flKeepXml===undefined){
		flKeepXml = true;
	}
	if(flAskForOpml===undefined){
		flAskForOpml = true;
	}
	if(minsecs===undefined){
		minsecs = 10;
	}
	var dbOutlineKey = "system.temp.op.netOutlines.["+urlOutline+"]";
	var adrtable = db.get(dbOutlineKey);
	if(adrtable===undefined){
		adrtable = {
			xmltext: undefined,
			etag: undefined,
			stats: {
				ctFullReads: 0,
				ct304s: 0,
				ctAccess: 0,
				whenLastRead: ((new Date()).getTime()-11000)
			}
		};
	}
	if(flKeepXml && (adrtable.xmltext===undefined)){
		flNeedXml = true;
	}

	var now = (new Date()).getTime();

	if(flNeedXml || ( (now-adrtable.stats.whenLastRead) > (minsecs * 1000) ) ){
		var flEtagOkay = true;
		if(flNeedXml || (minsecs==0)){
			flEtagOkay = false;
		}
		
		var requestHeaders = {};
		if(flEtagOkay && adrtable.etag){
			requestHeaders["If-None-Match"] = adrtable.etag;
		}

		var httpResponse = http.get(urlOutline, requestHeaders);

		if (httpResponse.headers["etag"]) {
			adrtable.etag = httpResponse.headers["etag"];
		}
		
		switch(httpResponse.code){
			case 200:
				if(flKeepXml){
					adrtable.xmltext=httpResponse.body;
				}
				break;
			case 304:
				// Do nothing
				break;
			case 404:
				throw "Can't access outline at \""+urlOutline+"\" because the remote server didn't find it.";
				break;
			default:
				if( (httpResponse.code < 500) || (adrtable.xmltext===undefined)){
					throw "Can't access outline at \""+urlOutline+"\" because an error occurred.";
				}
		}
	}

	var xmlDoc = xml.parse(adrtable.xmltext);

	adrtable.stats.whenLastRead = (new Date()).getTime();
	adrtable.stats.ctAccess++;

	db.put(dbOutlineKey, adrtable);
	return xmlDoc;
};
op.xmlToText = function(xmlDoc){
	var s = "";
	var outlineElements = xmlDoc.find("//outline");
	for(var i in outlineElements){
		var outlineElement = outlineElements[i];
		s += outlineElement.attr("text").value() + "\n";
	}
	return s;
};
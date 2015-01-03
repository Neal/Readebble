var xhr = null;
var http = function(method, url, data, headers, cb, fb) {
	if (xhr) { xhr.abort(); xhr = null; }
	if (method == 'GET' && data) {
		url += '?' + serialize(data);
		data = null;
	}
	console.log(method + ' ' + url + ' ' + data);
	xhr = new XMLHttpRequest();
	xhr.open(method, url, true);
	for (var h in headers) xhr.setRequestHeader(h, headers[h]);
	xhr.onload = function() { if (typeof cb === 'function') cb(xhr); };
	xhr.onerror = function() { if (typeof fb === 'function') fb('Connection error'); };
	xhr.ontimeout = function() { if (typeof fb === 'function') fb('Connection timed out!'); };
	xhr.timeout = 20000;
	xhr.send(data);
};

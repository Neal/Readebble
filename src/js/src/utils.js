function serialize(obj) {
	var s = [];
	for (var p in obj) {
		if (obj.hasOwnProperty(p)) {
			s.push(encodeURIComponent(p) + '=' + encodeURIComponent(obj[p]));
		}
	}
	return s.join('&');
}

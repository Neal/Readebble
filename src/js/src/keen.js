var Keen = {
	addEvent: function(name, data) {
		if (!data || !data instanceof Object) data = {};
		data.app = { name: AppInfo.shortName, version: AppInfo.versionLabel, uuid: AppInfo.uuid };
		data.user = { accountToken: Pebble.getAccountToken() };
		if (AppInfo.debug) {
			return console.log('[keen]', JSON.stringify(data));
		}
		var xhr = new XMLHttpRequest();
		xhr.open('POST', 'https://api.keen.io/3.0/projects/' + AppInfo.config.keen.projectId + '/events/' + name, true);
		xhr.setRequestHeader('Content-Type', 'application/json');
		xhr.setRequestHeader('Authorization', AppInfo.config.keen.writeKey);
		xhr.send(JSON.stringify(data));
	}
};

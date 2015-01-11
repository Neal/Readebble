var Readebble = {};

Readebble.subscriptions  = [];
Readebble.headlines = [];
Readebble.pocket = {};
Readebble.currentSubscription = {};
Readebble.currentHeadline = {};
Readebble.bufferSize = 440;

Readebble.init = function() {
	Readebble.subscriptions = JSON.parse(localStorage.getItem('subscriptions')) || [];
	Readebble.pocket = JSON.parse(localStorage.getItem('pocket')) || {};
	if (!Readebble.subscriptions instanceof Array) Readebble.subscriptions = [];
	if (!Readebble.pocket instanceof Object) Readebble.pocket = {};
	Readebble.sendSubscriptions();
	setTimeout(function() { Keen.addEvent('init', { subscriptions: { count: Readebble.subscriptions.length }, hasPocketToken: (Readebble.pocket.access_token !== 'undefined') }); }, 100);
};

Readebble.sendError = function(type, err) {
	appMessageQueue.clear();
	appMessageQueue.send({type:type, method:METHOD.ERROR, title:err});
};

Readebble.sendSubscriptions = function() {
	if (!this.subscriptions.length) {
		return Readebble.sendError(TYPE.SUBSCRIPTION, 'No subscriptions found. Use the app settings in the Pebble mobile app to add subscriptions.');
	}
	appMessageQueue.send({type:TYPE.SUBSCRIPTION, method:METHOD.SIZE, index:Readebble.subscriptions.length});
	for (var i = 0; i < Readebble.subscriptions.length; i++) {
		var title = Readebble.subscriptions[i].title.substring(0,30) || '';
		appMessageQueue.send({type:TYPE.SUBSCRIPTION, method:METHOD.DATA, index:i, title:title});
	}
};

Readebble.sendHeadlines = function() {
	if (!Readebble.headlines.length) {
		return Readebble.sendError(TYPE.HEADLINE, 'No headlines found in this feed.');
	}
	appMessageQueue.send({type:TYPE.HEADLINE, method:METHOD.SIZE, index:Readebble.headlines.length});
	for (var i = 0; i < Readebble.headlines.length; i++) {
		var title = Readebble.headlines[i].title.substring(0,160) || '';
		appMessageQueue.send({type:TYPE.HEADLINE, method:METHOD.DATA, index:i, title:title});
	}
};

Readebble.sendStory = function(story) {
	var maxStoryBuffer = Readebble.bufferSize - 32;
	for (var i = 0; i <= Math.floor(story.length/maxStoryBuffer); i++) {
		appMessageQueue.send({type:TYPE.STORY, method:METHOD.DATA, title:story.substring(i * maxStoryBuffer, i * maxStoryBuffer + maxStoryBuffer)});
	}
	appMessageQueue.send({type:TYPE.STORY, method:METHOD.READY});
};

Readebble.fetchHeadlines = function() {
	http('GET', 'https://ajax.googleapis.com/ajax/services/feed/load', {v:'1.0', num:'30', q:Readebble.currentSubscription.url}, null, function (e) {
		var res = JSON.parse(e.responseText);
		debugLog(res);
		if (res.responseStatus !== 200) {
			return Readebble.sendError(TYPE.HEADLINE, 'Error: ' + res.responseDetails);
		}
		Readebble.headlines = res.responseData.feed.entries;
		Readebble.sendHeadlines();
	}, function (e) {
		Readebble.sendError(TYPE.HEADLINE, e);
	});
};

Readebble.fetchStory = function() {
	var sendStory = function(story) {
		story = story.replace(/(<([^>]+)>)/ig, '').replace(/(\r\n|\n|\r)/gm, '').replace(/ +(?= )/g,'').substring(0,2000);
		return Readebble.sendStory(story);
	};
	if (Readebble.currentSubscription.use_description) {
		return sendStory(Readebble.currentHeadline.content);
	}
	http('GET', 'http://clipped.me/algorithm/clippedapi.php', {url:Readebble.currentHeadline.link}, null, function (e) {
		try {
			var res = JSON.parse(e.responseText);
			debugLog(res);
			if (!res.summary) {
				return sendStory(Readebble.currentHeadline.content);
			}
			var summary = res.summary.join(' ') || '';
			sendStory(summary);
		} catch (ee) {
			sendStory(Readebble.currentHeadline.content);
		}
	}, function (e) {
		Readebble.sendError(TYPE.STORY, e);
	});
	Keen.addEvent('story');
};

Readebble.addToPocket = function() {
	if (!Readebble.pocket.access_token) return Pebble.showSimpleNotificationOnPebble('Readebble', 'No Pocket account found. Please log in to your Pocket account via the Pebble mobile app.');
	var url = 'https://ineal.me/pebble/readebble/pocket/add';
	var data = { access_token: Readebble.pocket.access_token, url: Readebble.currentHeadline.link };
	http('POST', url, serialize(data), null, function (e) {
		var res = JSON.parse(e.responseText);
		debugLog(res);
		if (res.item && res.status == 1) {
			var title = res.item.title || 'article';
			Pebble.showSimpleNotificationOnPebble('Readebble', 'Successfully added ' + title + ' to Pocket!');
		}
	});
	Keen.addEvent('pocket');
};

Readebble.handleAppMessage = function(e) {
	console.log('AppMessage received: ' + JSON.stringify(e.payload));
	if (!e.payload.method) return;
	switch (e.payload.method) {
		case METHOD.REQUESTSUBSCRIPTIONS:
			Readebble.sendSubscriptions();
			break;
		case METHOD.REQUESTHEADLINES:
			Readebble.currentSubscription = Readebble.subscriptions[e.payload.index];
			Readebble.fetchHeadlines();
			break;
		case METHOD.REQUESTSTORY:
			Readebble.currentHeadline = Readebble.headlines[e.payload.index];
			Readebble.fetchStory();
			break;
		case METHOD.ADDTOPOCKET:
			Readebble.currentHeadline = Readebble.headlines[e.payload.index];
			Readebble.addToPocket();
			break;
		case METHOD.BUFFERSIZE:
			Readebble.bufferSize = e.payload.index;
			break;
	}
};

Readebble.showConfiguration = function() {
	var data = { subscriptions: Readebble.subscriptions };
	Keen.addEvent('configuration', { subscriptions: { count: Readebble.subscriptions.length } });
	Pebble.openURL('https://ineal.me/pebble/readebble/configuration/?data=' + encodeURIComponent(JSON.stringify(data)));
};

Readebble.handleConfiguration = function(e) {
	console.log('configuration received: ' + JSON.stringify(e));
	if (!e.response) return;
	var data = JSON.parse(decodeURIComponent(e.response));
	if (data.subscriptions) {
		Readebble.subscriptions = data.subscriptions;
		localStorage.setItem('subscriptions', JSON.stringify(Readebble.subscriptions));
	}
	if (data.pocket) {
		Readebble.pocket = data.pocket;
		localStorage.setItem('pocket', JSON.stringify(Readebble.pocket));
		if (Readebble.pocket.username) {
			Pebble.showSimpleNotificationOnPebble('Readebble', 'Hi ' + Readebble.pocket.username + '! You are now logged in to Pocket!\n\nHold the select button to add a story to your Pocket.');
		}
	}
	Readebble.sendSubscriptions();
};

Pebble.addEventListener('ready', Readebble.init);
Pebble.addEventListener('appmessage', Readebble.handleAppMessage);
Pebble.addEventListener('showConfiguration', Readebble.showConfiguration);
Pebble.addEventListener('webviewclosed', Readebble.handleConfiguration);

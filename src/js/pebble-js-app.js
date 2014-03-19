var subscriptions = [], headlines = [], pocket = {};
try { subscriptions = JSON.parse(localStorage.getItem('subscriptions')); } catch (e) {}
try { pocket = JSON.parse(localStorage.getItem('pocket')); } catch (e) {}

var appMessageQueue = {
	queue: [],
	numTries: 0,
	maxTries: 5,
	maxBuffer: 480,
	add: function(obj) {
		this.queue.push(obj);
	},
	clear: function() {
		this.queue = [];
	},
	isEmpty: function() {
		return this.queue.length === 0;
	},
	nextMessage: function() {
		return this.isEmpty() ? {} : this.queue[0];
	},
	send: function() {
		if (this.queue.length > 0) {
			var ack = function() {
				appMessageQueue.numTries = 0;
				appMessageQueue.queue.shift();
				appMessageQueue.send();
			};
			var nack = function() {
				appMessageQueue.numTries++;
				appMessageQueue.send();
			};
			if (this.numTries >= this.maxTries) {
				console.log('Failed sending AppMessage: ' + JSON.stringify(this.nextMessage()));
				ack();
			}
			console.log('Sending AppMessage: ' + JSON.stringify(this.nextMessage()));
			Pebble.sendAppMessage(this.nextMessage(), ack, nack);
		}
	}
};

function addToPocket(headline) {
	var url = headline.link || '';
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'http://ineal.me/pebble/readebble/pocket/add?access_token=' + decodeURIComponent(pocket.access_token) + '&url=' + decodeURIComponent(url), true);
	xhr.onload = function() {
		if (xhr.readyState == 4 && xhr.status == 200) {
			res = JSON.parse(xhr.responseText);
			if (res.item && res.status == 1) {
				var title = res.item.title || 'article';
				Pebble.showSimpleNotificationOnPebble('Readebble', 'Successfully added ' + title + ' to Pocket!');
			}
		}
	};
	xhr.send(null);
}

function summarize(headline) {
	var url = headline.link || '';
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'http://clipped.me/algorithm/clippedapi.php?url=' + decodeURIComponent(url), true);
	xhr.onload = function() {
		if (xhr.readyState == 4 && xhr.status == 200) {
			try {
				res = JSON.parse(xhr.responseText);
				var title = res.title || '';
				var summary = res.summary.join(' ') || '';
				for (var i = 0; i <= Math.floor(summary.length/appMessageQueue.maxBuffer); i++) {
					appMessageQueue.add({summary: summary.substring(i * appMessageQueue.maxBuffer, i * appMessageQueue.maxBuffer + appMessageQueue.maxBuffer)});
				}
				appMessageQueue.add({summary: true, title: title.substring(0,64)});
			} catch(e) {
				appMessageQueue.add({summary: true, title: 'Failed to summarize!'});
			}
		} else {
			appMessageQueue.add({summary: true, title: 'Request returned HTTP error ' + xhr.statusText});
		}
		appMessageQueue.send();
	};
	xhr.send(null);
}

function fetchHeadlines(subscription) {
	appMessageQueue.clear();
	var xhr = new XMLHttpRequest();
	xhr.open('GET', 'https://ajax.googleapis.com/ajax/services/feed/load?v=1.0&num=30&q=' + encodeURIComponent(subscription.url), true);
	xhr.onload = function(e) {
		appMessageQueue.add({headline: true, index: true});
		if (xhr.readyState == 4 && xhr.status == 200) {
			res = JSON.parse(xhr.responseText);
			headlines = res.responseData.feed.entries;
			headlines.forEach(function (element, index, array) {
				var title = element.title.substring(0,160) || '';
				appMessageQueue.add({headline: true, index: index, title: title});
			});
		}
		appMessageQueue.send();
	};
	xhr.send(null);
}

function sendSubscriptions() {
	appMessageQueue.clear();
	if (subscriptions.length === 0) {
		appMessageQueue.add({subscription: true, index: true});
	}
	for (var i = 0; i < subscriptions.length; i++) {
		appMessageQueue.add({subscription: true, index: i, title: subscriptions[i].title});
	}
	appMessageQueue.send();
}

Pebble.addEventListener('ready', function(e) {
	if (!(subscriptions instanceof Array)) subscriptions = [];
	sendSubscriptions();
});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received from Pebble: ' + JSON.stringify(e.payload));
	if (e.payload.pocket) {
		addToPocket(headlines[e.payload.index]);
	} else if (e.payload.summary) {
		summarize(headlines[e.payload.index]);
	} else if (e.payload.headline) {
		fetchHeadlines(subscriptions[e.payload.index]);
	} else if (e.payload.subscription) {
		sendSubscriptions();
	} else {
		appMessageQueue.clear();
	}
});

Pebble.addEventListener('showConfiguration', function() {
	var data = {subscriptions: subscriptions};
	var uri = 'http://neal.github.io/Readebble/index.html?data=' + encodeURIComponent(JSON.stringify(data));
	Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
	if (e.response) {
		console.log(e.response);
		var data = JSON.parse(decodeURIComponent(e.response));
		if (data.subscriptions) {
			subscriptions = data.subscriptions;
			localStorage.setItem('subscriptions', JSON.stringify(subscriptions));
		}
		if (data.pocket) {
			pocket = data.pocket;
			localStorage.setItem('pocket', JSON.stringify(pocket));
			if (pocket.username) {
				Pebble.showSimpleNotificationOnPebble('Readebble', 'Hi ' + pocket.username + '! You are now logged in to Pocket!\n\nHold the select button to add a story to your Pocket.');
			}
		}
		sendSubscriptions();
	}
});

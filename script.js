$(document).bind('pageinit', function() {

	var subscriptions = [];
	var panels = 0;

	var data = /data=([^&|^\/]*)/.exec(location.search);
	if (data && data[1]) {
		var d = JSON.parse(decodeURIComponent(data[1]));
		subscriptions = d.subscriptions;
	}

	$.each(subscriptions, function(index, subscription) {
		var newPanel = $('.panel-template').first().clone();
		addNewPanel(subscription.title, newPanel);
		$.each(subscription, function(index, value) {
			newPanel.find('#'+index).val(value);
		});
	});

	function addNewPanel(title, newPanel) {
		newPanel.find('.accordion-toggle').attr('href', '#panel' + (++panels));
		newPanel.find('.panel-collapse').attr('id', 'panel' + panels);
		newPanel.find('.panel-title').text(title);
		$('#accordion').append(newPanel.fadeIn());
		newPanel.find('#itemName').change(function() {
			newPanel.find('.panel-title').text($(this).val());
		});
		newPanel.find('.btn-delete').click(function() {
			newPanel.remove();
		});
		newPanel.find('.panel-heading').on('swipe', function() {
			if(newPanel.find('.btn-delete').is(':hidden')) { newPanel.find('.btn-delete').show(); } else { newPanel.find('.btn-delete').hide(); }
		});
	}

	$('.btn-add-subscription').on('click', function() {
		addNewPanel('New Subscription', $('.panel-template').first().clone());
	});

	$('.btn-save').on('click', function() {
		var subscriptions = $.map($('.panel'), function(e) {
			if ($(e).find('.panel-collapse').attr('id').indexOf('template') != -1)
				return;
			var subscription = {};
			$(e).find('input').map(function() {
				subscription[this.id] = $(this).val();
			});
			return subscription;
		});
		var ret = {
			subscriptions: subscriptions
		};
		document.location = 'pebblejs://close#' + encodeURIComponent(JSON.stringify(ret));
	});

	$('.panel-container').sortable({
		handle: '.handle',
		axis: 'y'
	});
});

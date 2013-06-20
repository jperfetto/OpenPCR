LOCALIZE_CLASS_REGEXP = new RegExp('iinya_localize_([^ ]+)');
function localize()
{
	var tags = [];
	var all = document.getElementsByTagName('*');
	for (var i=0; i<all.length; i++) {
		if ('SPAN'!=all[i].name)
			tags.push(all[i]);
	}
	var buttons = document.getElementsByTagName('INPUT');
	
	for (var i=0, l=tags.length; i<l; i++)
	{
		var element = tags[i];
		if (null!=element.className && element.className.match(LOCALIZE_CLASS_REGEXP))
		{
			element.innerHTML = chrome.i18n.getMessage(RegExp.$1);
		}
	}
	for (var i=0, l=buttons.length; i<l; i++)
	{
		var element = buttons[i];
		if ('button'!=element.type) continue;
		if (null!=element.className && element.className.match(LOCALIZE_CLASS_REGEXP))
		{
			element.value = chrome.i18n.getMessage(RegExp.$1);
		}
	}
};
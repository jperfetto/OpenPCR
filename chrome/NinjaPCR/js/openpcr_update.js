		
// UPDATE APPLICATION. Uses config/updater-descriptor.xml
// XML file includes a "version" tag, which I think is no longer in use by Air 2, but is needed for the Updater we use. Otherwise the "Currently Installed Version" on the update screen will read blank.
/**
			 * Click event handler for the Check for Update button. Checks for an update
			 * version of the application posted on the update server. If there is 
			 * and update version, the updater proceeds with the update, based on 
			 * the settings in the update configuration file.
			 */
			function checkForUpdate()
			{
				appUpdater.checkNow();
			}
			/**
			 * Displays the current application version in the UI. 
			 */
			function setApplicationNameAndVersion() 
			{
				var xmlString = air.NativeApplication.nativeApplication.applicationDescriptor;
				
				var appXml = new DOMParser();
				var xmlObject = appXml.parseFromString(xmlString, "text/xml");
				var root = xmlObject.getElementsByTagName('application')[0];

				
				var lblAppVersion = "";
				var lblAppName = "";

				lblAppVersion.value = root.getElementsByTagName("versionNumber")[0].firstChild.data;
				lblAppName.value = root.getElementsByTagName("filename")[0].firstChild.data;
			}
			/**
			 * Displays an error message when the updater dispatches an error event.
			 */
			function onError(event)
			{
            			alert(event.toString());
			}
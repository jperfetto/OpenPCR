/* Javascript for OpenPCR
 *
 * http://openpcr.org
 * Copyright (c) 2011 OpenPCR
 */
 
 /* Checks if the OpenPCR app is booted up for the first time. If so, a sample set of "Experiments" is created for the user */
 
 // check if the "Experiments" folder exists in the user's storage
 // if the Experiments folder exists, don't do anything
// if the "Experiments" folder doesn't exist, copy the "Experiments" folder from the app directory into the user's application storage dir

// files from the app folder
Log.v('TODO TORI source = air.File.applicationDirectory.resolvePath("Default Experiments");');

// into the user's storage folder
Log.v('TODO TORI destination = air.File.applicationStorageDirectory.resolvePath("Experiments");');
// copy
Log.v('TODO TORI source.copyTo( destination, false );');
// should throw an error to IO, "Error: Error #3012: Cannot delete file or directory" if folder exists
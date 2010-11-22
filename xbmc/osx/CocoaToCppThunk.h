#pragma once
/*
 *  CocoaToCppThunk.h
 *  XBMC
 *
 *  Created by Elan Feingold on 2/27/2008.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#import "AppleRemoteKeys.h"

#ifdef __cplusplus
extern "C" 
{
#endif

void Cocoa_OnAppleRemoteKey(void* application, AppleRemoteEventIdentifier event, bool pressedDown, unsigned int count);
void Cocoa_DownloadFile(const char* remoteFile, const char* localFile);
void Cocoa_CPPUpdateProgressDialog();
void Cocoa_AutodetectRemotePlexSources(const char* hostName, const char* hostLabel, const char* url);
void Cocoa_RemoveRemotePlexSources(const char* hostName);
  
#ifdef __cplusplus
}
#endif

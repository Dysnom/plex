//
//  XBMCMain.m
//  XBMC
//
//  Created by Elan Feingold on 2/27/2008.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//
#include <vector>

#import "XBMCMain.h"
#import "AppleRemote.h"
typedef char BYTE;
#include "Log.h"
#include "CocoaToCppThunk.h"

#define PLEX_SERVICE_PORT 32401
#define PLEX_SERVICE_TYPE @"_plexmediasvr._tcp"

using namespace std;

@implementation XBMCMain

static XBMCMain *_o_sharedMainInstance = nil;

+ (XBMCMain *)sharedInstance
{
  return _o_sharedMainInstance ? _o_sharedMainInstance : [[self alloc] init];
}

- (id)init
{
  if( _o_sharedMainInstance)
      [self dealloc];
  else
      _o_sharedMainInstance = [super init];

  o_remote = [[AppleRemote alloc] init];
  [o_remote setClickCountEnabledButtons: kRemoteButtonPlay];
  [o_remote setDelegate: _o_sharedMainInstance];
  
  o_plexMediaServers = [[NSMutableArray alloc] initWithCapacity:10];
  o_plexMediaServerHosts = [[NSMutableDictionary alloc] initWithCapacity:10];
  o_plexMediaServerBrowser = [[NSNetServiceBrowser alloc] init];
  [o_plexMediaServerBrowser setDelegate:self];

  // Let the system stablize a bit before we start searching for media servers
  [self performSelector:@selector(searchForPlexMediaServers) withObject:nil afterDelay:0.1];
	
  // Start the client's Bonjour service
  o_plexNetService = [[NSNetService alloc] initWithDomain:@"" type:@"_plexclient._tcp" name:@"" port:32401];
  [o_plexNetService setTXTRecordData:
   [NSNetService dataFromTXTRecordDictionary:
    [NSDictionary dictionaryWithObject:[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"]
                                forKey:@"Version"]]];
  [o_plexNetService publish];
  
  // broadcast launch notification
	CFNotificationCenterPostNotification( CFNotificationCenterGetDistributedCenter (),
										 CFSTR("PlexGUIInit"), 
										 CFSTR("PlexTVServer"), 
										 /*userInfo*/ NULL, 
										 TRUE );

  // Start listening in exclusive mode.
  //[o_remote startListening: self];
  
  return _o_sharedMainInstance;
}

- (void)dealloc {
  [o_plexNetService release];
  [o_remote release], o_remote = nil;
  [o_plexMediaServerBrowser release], o_plexMediaServers = nil;
  [o_plexMediaServers release], o_plexMediaServers = nil;
  [o_plexMediaServerHosts release], o_plexMediaServerHosts = nil;
  [super dealloc];
}

- (void)searchForPlexMediaServers
{
  [o_plexMediaServerBrowser searchForServicesOfType:PLEX_SERVICE_TYPE inDomain:@""];
}

- (void)stopSearchingForPlexMediaServers
{
  [o_plexMediaServerBrowser stop];
}

- (NSArray *)plexMediaServers
{
  return o_plexMediaServers;
}

/* Apple Remote callback */
- (void) appleRemoteButton: (AppleRemoteEventIdentifier)buttonIdentifier
               pressedDown: (BOOL) pressedDown
                clickCount: (unsigned int) count
{
    // Pass event to thunk.
    Cocoa_OnAppleRemoteKey(pApplication, buttonIdentifier, pressedDown, count);
}

- (void)setApplication: (void*) application;
{
  pApplication = application;
}

/* NSNetServiceBrowser Delegate Overrides */
-(void)netServiceBrowser:(NSNetServiceBrowser *)aBrowser didFindService:(NSNetService *)service moreComing:(BOOL)more 
{
  CLog::Log(LOGNOTICE, "Bonjour: Did find service - %s", [[service name] UTF8String]);
  [o_plexMediaServers addObject:service];
  service.delegate = self;
  [service resolveWithTimeout:30];
}

-(void)netServiceBrowser:(NSNetServiceBrowser *)aBrowser didRemoveService:(NSNetService *)service moreComing:(BOOL)more 
{
  CLog::Log(LOGNOTICE, "Bonjour: Did remove service - %s", [[service name] UTF8String]);
  [o_plexMediaServers removeObject:service];
  if ([o_plexMediaServerHosts objectForKey:[service name]])
  {
    Cocoa_RemoveRemotePlexSources([[o_plexMediaServerHosts objectForKey:[service name]] UTF8String]);
    [o_plexMediaServerHosts removeObjectForKey:[service name]];
  }
}

/* NSNetService Delegate Overrides */
-(void)netServiceDidResolveAddress:(NSNetService *)service 
{
  CLog::Log(LOGNOTICE, "Bonjour: Did resolve service - %s (type: %s)", [[service name] UTF8String], [[service type] UTF8String]);
  [o_plexMediaServerHosts setObject:[service hostName] forKey:[service name]];
  Cocoa_AutodetectRemotePlexSources([[service hostName] UTF8String], [[service name] UTF8String]);
  [service startMonitoring];
}

-(void)netService:(NSNetService *)service didNotResolve:(NSDictionary *)errorDict 
{ 
  CLog::Log(LOGWARNING, "Bonjour: Did NOT resolve service - %s", [[service name] UTF8String]);
}

- (void)netService:(NSNetService *)service didUpdateTXTRecordData:(NSData *)data
{
  CLog::Log(LOGNOTICE, "Bonjour: Updated TXT record for - %s", [[service name] UTF8String]);
  Cocoa_AutodetectRemotePlexSources([[service hostName] UTF8String], [[service name] UTF8String]);
}

- (void)refreshAllRemotePlexSources
{
  for (NSNetService* service in o_plexMediaServers)
  {
    Cocoa_AutodetectRemotePlexSources([[service hostName] UTF8String], [[service name] UTF8String]);
  }
}

- (void)removeAllRemotePlexSources
{
  for (NSNetService* service in o_plexMediaServers)
  {
    Cocoa_RemoveRemotePlexSources([[service hostName] UTF8String]);
  }  
}

@end

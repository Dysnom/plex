/*
 * Copyright (c) 2006-2008 Amit Singh. All Rights Reserved.
 * Copyright (c) 2008 Elan Feingold. All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     
 *  THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 */
#define BINARY_NAME   "Plex"
#define APPNAME       "Plex.app"
#define PROGNAME      "PlexHelper"
#define PROGVERS      "1.1.1"

#define ENABLE_SWITCH_EVENTS_HANDLER

#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sysexits.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <mach-o/dyld.h>

#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>

#include "XBox360.h"
#include "Reactor.h"
#include "../../lib/c++/xbmcclient.h"

#include <iostream>
#include <map>
#include <string>
#include <iterator>
#include <sstream>

#import "HIDRemote.h"

using namespace std;

static struct option long_options[] = 
{
    { "help",       no_argument,         0, 'h' },
    { "server",     required_argument,   0, 's' },
    { "universal",  no_argument,         0, 'u' },
    { "multicode",  no_argument,         0, 'm' },
    { "timeout",    required_argument,   0, 't' },
    { "verbose",    no_argument,         0, 'v' },
    { "externalConfig", no_argument,     0, 'x' },
    { "appLocation", required_argument,  0, 'a' },
    { "secureInput", required_argument,  0, 'i' },
    { 0, 0, 0, 0 },
};
static const char *options = "hsutvxaim";
 
const int REMOTE_SWITCH_COOKIE = 39;
const int IGNORE_CODE = 19;
const int FIRST_REMOTE_ID = 150;

#define REMOTE_MODE_STANDARD 0
#define REMOTE_MODE_UNIVERAL 1
#define REMOTE_MODE_MULTI    2

static int GetBSDProcessList(kinfo_proc **procList, size_t *procCount);
bool isProgramRunning(const char* strProgram, int ignorePid=0);
void startXBMC();
long getTimeInMilliseconds();
void readConfig();
void parseOptions(int argc, char** argv);
void handleSignal(int signal);

#define APPLE_REMOTE "JS0:Apple Remote"
#define SEQUENTIAL_UNIVERSAL_REMOTE "R1"
#define MULTICODE_UNIVERSAL_REMOTE "MCUR"

#define kRemoteButtonUp              1
#define kRemoteButtonDown            2
#define kRemoteButtonLeft            3
#define kRemoteButtonRight           4
#define kRemoteButtonCenter          5
#define kRemoteButtonMenu            6
#define kRemoteButtonLeft_Hold       9
#define kRemoteButtonRight_Hold     10
#define kRemoteButtonMenu_Hold      11
#define kRemoteButtonCenter_Hold    12
#define kRemoteButtonPlayPause      13
#define kRemoteButtonPlayPause_Hold 14

#define kDownMask             0x1000
#define kButtonMask           0x00FF

#define kButtonRepeats        0x01
#define kButtonSendsUpEvents  0x02

/*!
 * @brief Class used to track and check characteristics of buttons
 */
class ButtonConfig
{
 public:
  ButtonConfig()
    : sendsUpEvent(false)
    , repeats(false) {}
 
  ButtonConfig(int flags)
    : sendsUpEvent(flags & kButtonSendsUpEvents)
    , repeats(flags & kButtonRepeats) {}
 
  bool sendsUpEvent;
  bool repeats;
};

map<int, int> keyMap;
map<string, string> keyMapUniversal;
map<string, bool> keyMapUniversalRepeat;
map<int, ButtonConfig> buttonConfigMap;

volatile bool verbose = false;
volatile bool secureInput = false;
string serverAddress = "127.0.0.1";
string appLocation = "/Applications";
CAddress* pServer = 0;
int sockfd;

/*!
 * @brief Subclass of XBMC's Reactor class which differentiates between universal and Apple Remote mode
 *
 * This makes use of various globals which are managed in PlexHIDRemoteDelegate and in the
 * various C functions below.
 */
class ReactorAppleRemote : public Reactor
{
public:
	
	ReactorAppleRemote()
    : _needToRelease(false)
    , _isUniversalMode(false)
    , _isMultiRemoteMode(false)
    , _itsTimeout(500)
    , _itsRemoteID(FIRST_REMOTE_ID)
	{
	}
	
	virtual void onTimeout()
	{
		if (verbose)
			printf(" -> Timeout!\n");
		
		// Reset the Universal prefix we were working on.
		_itsRemotePrefix = "";
	}
	
	/* 
	 * @brief Used by Reactor.sendMessage() to process a button behavior
	 */
	virtual void onCmd(int data)
	{ 
		ButtonConfig   config = buttonConfigMap[data & kButtonMask];
		char           strButton[16];
		int            button = data & kButtonMask;
		
		if (_isMultiRemoteMode)
			sprintf(strButton, "%d:%d", _itsRemoteID, button);
		else
			sprintf(strButton, "%d", button);
		
		if (verbose)
			printf("Received %s button\n", strButton);
		
		if ((button == kRemoteButtonMenu) && ((data & kDownMask) == 0))
		{
			// Make sure XBMC is running.
			if (isProgramRunning(BINARY_NAME) == false && serverAddress == "127.0.0.1")
			{
				startXBMC();
				return;
			}
		}
		
		if (_isUniversalMode)
		{      
			if (data & kDownMask)
			{
				_itsRemotePrefix += strButton;
				_itsRemotePrefix += "_";
				
				if (verbose && _lastTime != 0)
					printf(" -> Key received after %ld ms.\n", getTimeInMilliseconds()-_lastTime);
				
				string val = keyMapUniversal[_itsRemotePrefix];
				if (val.length() != 0)
				{
					bool isSimpleButton;
					if (keyMapUniversalRepeat.count(_itsRemotePrefix) > 0)
						isSimpleButton = _needToRelease = true;
					else 
						isSimpleButton = _needToRelease = false;
					
					// Turn off repeat for complex buttons.
					int flags = BTN_DOWN;
					if (isSimpleButton == false)
						flags |= (BTN_NO_REPEAT | BTN_QUEUE);

					// Figure out the keymap.
					string remoteString = SEQUENTIAL_UNIVERSAL_REMOTE;
					if (_isMultiRemoteMode)
					  remoteString = MULTICODE_UNIVERSAL_REMOTE;
					
					// Send the command to XBMC.
					if (verbose)
						printf(" -> Sending command: %s, remote: %s\n", val.c_str(), remoteString.c_str());
					
					CPacketBUTTON btn(val.c_str(), remoteString.c_str(), flags);
					btn.Send(sockfd, *pServer);
					_itsRemotePrefix = "";
					
					// Reset the timer.
					resetTimer();
					_lastTime = 0;
				}
				else
				{  
					// We got a prefix. Set a timeout.
					setTimer(_itsTimeout);
					_lastTime = getTimeInMilliseconds();
				}
			}
			else
			{
				if (_needToRelease)
				{
					_needToRelease = true;
					if (verbose)
						printf(" -> Sending button release.\n");
					
					// Send the button release.  
					CPacketBUTTON btn;
					btn.Send(sockfd, *pServer);
				}
			}
		}
		else if (_isUniversalMode == false)
		{
			// Send the down part.
			if ((config.sendsUpEvent && (data & kDownMask)) || config.sendsUpEvent == false)
			{
				if (verbose)
					printf(" -> Sending button [%s] down.\n", strButton);
				
				int flags = BTN_DOWN;
				if (config.sendsUpEvent == false)
					flags |= (BTN_NO_REPEAT | BTN_QUEUE);
				
				CPacketBUTTON btn(atoi(strButton), APPLE_REMOTE, flags, config.repeats);
				btn.Send(sockfd, *pServer);
			}
			
			// Send the up part.
			if (config.sendsUpEvent && (data & kDownMask) == 0)
			{
				if (verbose)
					printf(" -> Sending button up.\n");
				
				CPacketBUTTON btn; 
				btn.Send(sockfd, *pServer);
			}
		}
	}
	
	virtual void onSetTimeout(int timeout)
	{
	  if (verbose)
	    printf("Setting timeout to %dms.\n", timeout);
	}
	
	virtual void onSetMode(int mode)
	{
	  _isUniversalMode = false;
	  _isMultiRemoteMode = false;
	  
	  if (mode == REMOTE_MODE_UNIVERAL || mode == REMOTE_MODE_MULTI)
	    _isUniversalMode = true;
	  
	  if (mode == REMOTE_MODE_MULTI)
	    _isMultiRemoteMode = true;
	  
	  if (verbose)
	    printf("Setting mode to universal:%d multi:%d.\n", _isUniversalMode, _isMultiRemoteMode);
	}
	
	virtual void onSetRemote(int remote)
	{
	  if (verbose)
	    printf("Setting remote to #%d\n", remote);
	    
	  _itsRemoteID = remote;
	}
	
	virtual void onServerCmd(char* data, char* arg)
	{
	}
	
protected:
	
	bool   _isUniversalMode;
	bool   _isMultiRemoteMode;
	int    _itsTimeout;
	int    _itsRemoteID;
	
	string _itsRemotePrefix;
	bool   _needToRelease;
	int    _lastTime;
};

ReactorAppleRemote theReactor;

@interface PlexHIDRemoteDelegate : NSObject <HIDRemoteDelegate> {
	HIDRemote *hidRemote;
}
@end

@implementation PlexHIDRemoteDelegate

- (BOOL)startRemoteControl
{
	if ((hidRemote = [HIDRemote sharedHIDRemote])) {
		/* Left/Right/Play-Pause/Menu have real hold events, which we'll still receive.
		 * This disables faking hold events for Up/Down, which we already account for in code dating to
		 * when we had our own HID management rather than using hidRemoteControl.
		 */
		[hidRemote setSimulateHoldEvents:NO];
		[hidRemote setExclusiveLockLendingEnabled:YES];
		[hidRemote setEnableSecureEventInputWorkaround:secureInput];
		[hidRemote setDelegate:self];
	} else {
		NSLog(@"Couldn't get sharedHIDRemote");
	}

	return [hidRemote startRemoteControl:kHIDRemoteModeExclusive];
}

- (void)stopRemoteControl
{
  if (hidRemote)
    [hidRemote stopRemoteControl];
}

/*!
 * @brief Update whether we are using the secure event input workaround.
 *
 * See HIDRemote's documentation for details.
 * I am unclear as to why we would ever not use this. -evands
 */
- (void)setEnableSecureEventInputWorkaround:(BOOL)inSecureInput
{
	[hidRemote setEnableSecureEventInputWorkaround:secureInput];
}

/*!
 * @brief Called when a button is pressed or released on the remote
 */
- (void)hidRemote:(HIDRemote *)hidRemote  eventWithButton:(HIDRemoteButtonCode)buttonCode
                                                isPressed:(BOOL)isPressed
		                           fromHardwareWithAttributes:(NSMutableDictionary *)attributes; // Information on the device this event comes from
{
	int  buttonId = keyMap[buttonCode];
	// Send the message to the button processor.
	theReactor.sendMessage(Reactor::CmdData, buttonId | (kDownMask * isPressed));
}


/*!
 * @brief Invoked when the user switched to a remote control with a different ID
 *
 * This is particularly useful to us because the multicode Harmony remote switches IDs regularly to avoid sequences
 */
- (void)hidRemote:(HIDRemote *)hidRemote remoteIDChangedOldID:(SInt32)old
                                                        newID:(SInt32)newID
                                    forHardwareWithAttributes:(NSMutableDictionary *)attributes
{
	theReactor.setRemote(newID);
}

- (void)cleanupRemote
{
	if ([hidRemote isStarted])
		[hidRemote stopRemoteControl];

	[hidRemote setDelegate:nil];
	[hidRemote release];
	hidRemote = nil;
}

- (void)dealloc
{
	[self cleanupRemote];
	[super dealloc];
}

/* We want an exclusive lock whenever possible */
- (void)hidRemote:(HIDRemote *)aHidRemote exclusiveLockReleasedByApplicationWithInfo:(NSDictionary *)applicationInfo
{
	[aHidRemote startRemoteControl:kHIDRemoteModeExclusive];
}

/* We want an exclusive lock whenever possible */
- (BOOL)hidRemote:(HIDRemote *)aHidRemote shouldRetryExclusiveLockWithInfo:(NSDictionary *)applicationInfo
{
	return YES;
}

/* We'll give up an exclusive lock if asked nicely, though */
- (BOOL)hidRemote:(HIDRemote *)aHidRemote lendExclusiveLockToApplicationWithInfo:(NSDictionary *)applicationInfo
{
	return YES;
}

@end

void            usage();
inline          void print_errmsg_if_io_err(int expr, char *msg);
inline          void print_errmsg_if_err(int expr, char *msg);
void            setupAndRun();
PlexHIDRemoteDelegate *remoteDelegate = nil;

void usage()
{
    printf("%s (version %s)\n", PROGNAME, PROGVERS);
    printf("   Copyright (c) 2008 Plex. All Rights Reserved.\n");
    printf("   Sends control events to Plex.\n\n");
    printf("Usage: %s [OPTIONS...]\n\nOptions:\n", PROGNAME);
    printf("  -h, --help               Print this help message and exit.\n");
    printf("  -s, --server <addr>      Send events to the specified IP.\n");
    printf("  -t, --timeout <ms>       Timeout length for sequences (default: 500ms).\n");
    printf("  -v, --verbose            Prints lots of debugging information.\n");
    printf("  -a, --appLocation <path> The location of the application.\n");
}

inline void print_errmsg_if_io_err(int expr, char *msg)
{
    IOReturn err = (expr);

    if (err != kIOReturnSuccess) {
        fprintf(stderr, "*** %s - %s(%x, %d).\n", msg, mach_error_string(err),
                err, err & 0xffffff);
        fflush(stderr);
        exit(EX_OSERR);
    }
}

inline void print_errmsg_if_err(int expr, char *msg)
{
    if (expr) {
        fprintf(stderr, "*** %s.\n", msg);
        fflush(stderr);
        exit(EX_OSERR);
    }
}

bool isProgramRunning(const char* strProgram, int ignorePid)
{
	kinfo_proc* mylist = (kinfo_proc *)malloc(sizeof(kinfo_proc));
	size_t mycount = 0;
	bool ret = false;
	
	GetBSDProcessList(&mylist, &mycount);
  for(size_t k = 0; k < mycount && ret == false; k++) 
	{
		kinfo_proc *proc = NULL;
	  proc = &mylist[k];

		if (strcmp(proc->kp_proc.p_comm, strProgram) == 0)
		{
		  if (ignorePid == 0 || ignorePid != proc->kp_proc.p_pid)
			  ret = true;
		}
	}
	free(mylist);
	
	return ret;
}

void startXBMC()
{
	printf("Trying to start Plex.\n");
	
  string app = appLocation + "/" APPNAME;
  string strCmd = "open ";
	strCmd += app;
	strCmd += "&";

	// Start it in the background.
	printf("Got path: [%s]\n", app.c_str());
	system(strCmd.c_str());
}

void doRun()
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	CFRunLoopRun();	
	[pool release];
	
	// If we enabled secure input, disable it after the run loop exits
	if (secureInput == true)
    {
		DisableSecureEventInput();
		printf("Disabling secure input.\n");
    }
}

void setupAndRun()
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	remoteDelegate = [[PlexHIDRemoteDelegate alloc] init];
	NSLog(@"PlexHIDRemoteDelegate is %@", remoteDelegate);
	if (remoteDelegate)
	{
		[remoteDelegate startRemoteControl];
		[pool release];

		// This won't return until the application is complete
		doRun();
		
		// Now shut down the devices we opened previously and dispose of their queues
		[remoteDelegate release];
    }
    else
    {
      printf("WARNING: No Remote hardware found.\n");
    }
}

#ifdef ENABLE_SWITCH_EVENTS_HANDLER
/* This code won't currently compile; it's legacy. We may not actually need it... but I'm not sure.
 * It existed to work around a bug in which we could lose remote exclusivity.
 */
pascal OSStatus switchEventsHandler(EventHandlerCallRef nextHandler,
                                    EventRef switchEvent,
                                    void* userData)
{
	id pool = [[NSAutoreleasePool alloc] init];
	  
	// Simply stop and start the remote.
	[remoteDelegate stopRemoteControl];
	[remoteDelegate startRemoteControl];
	
	[pool drain];
	
	return 0;
}

void* RunEventLoop(void* )
{
  sigset_t signalSet;
  sigemptyset (&signalSet);
  sigaddset (&signalSet, SIGHUP);
  pthread_sigmask(SIG_BLOCK, &signalSet, NULL);
  RunApplicationEventLoop();
  
  return 0;
}          
#endif

int main(int argc, char **argv)
{
  // Handle SIGHUP.  
  signal(SIGHUP, handleSignal);
  signal(SIGUSR1, handleSignal);
  
  // We should close all file descriptions.
  for (int i=3; i<256; i++)
    close(i);
  
  // If we're already running, exit right away.
  if (isProgramRunning(PROGNAME, getpid()))
  {
    printf(PROGNAME " is already running.\n");
    exit(-1);
  }

	// Add the key mappings.
	keyMap[kHIDRemoteButtonCodePlus] = kRemoteButtonUp;
	keyMap[kHIDRemoteButtonCodeMinus] = kRemoteButtonDown;
	keyMap[kHIDRemoteButtonCodeMenu] = kRemoteButtonMenu;
	keyMap[kHIDRemoteButtonCodeCenter] = kRemoteButtonCenter;
	keyMap[kHIDRemoteButtonCodeRight] = kRemoteButtonRight;
	keyMap[kHIDRemoteButtonCodeLeft] = kRemoteButtonLeft;
	keyMap[kHIDRemoteButtonCodeRightHold] = kRemoteButtonRight_Hold;
	keyMap[kHIDRemoteButtonCodeLeftHold] = kRemoteButtonLeft_Hold;
	keyMap[kHIDRemoteButtonCodeMenuHold] = kRemoteButtonMenu_Hold;
	keyMap[kHIDRemoteButtonCodeCenterHold] = kRemoteButtonCenter_Hold;
	
	// Aluminum remote keys.
	keyMap[kHIDRemoteButtonCodePlay] = kRemoteButtonPlayPause;
	keyMap[kHIDRemoteButtonCodePlayHold] = kRemoteButtonPlayPause_Hold;
	
	// Mappings for Universal Mode (sequential mode)
	keyMapUniversal["5_"] = "Select";
	keyMapUniversal["3_"] = "Left";
	keyMapUniversal["9_"] = "Left";
	keyMapUniversal["4_"] = "Right";
	keyMapUniversal["10_"] = "Right";
	keyMapUniversal["1_"] = "Up";
	keyMapUniversal["2_"] = "Down";
	keyMapUniversal["6_"] = "";
	
	// Left/right held, and up/down normal repeat.
	keyMapUniversalRepeat["1_"] = true;
	keyMapUniversalRepeat["2_"] = true;
	keyMapUniversalRepeat["9_"] = true;
	keyMapUniversalRepeat["10_"] = true;
	
	keyMapUniversal["6_3_"] = "Reverse";
	keyMapUniversal["6_4_"] = "Forward";
	keyMapUniversal["6_2_"] = "Back";
	keyMapUniversal["6_6_"] = "Menu";
	keyMapUniversal["6_1_"] = "";
	keyMapUniversal["6_5_"] = "";
	
	keyMapUniversal["6_5_3_"] = "Title";
	keyMapUniversal["6_5_4_"] = "Info";
	keyMapUniversal["6_5_1_"] = "PagePlus";
	keyMapUniversal["6_5_2_"] = "PageMinus";
	keyMapUniversal["6_5_5_"] = "Display";
	
	keyMapUniversal["6_1_5_"] = "Stop";
	keyMapUniversal["6_1_3_"] = "Power";
	keyMapUniversal["6_1_4_"] = "Zero";
	keyMapUniversal["6_1_1_"] = "Play";
	keyMapUniversal["6_1_2_"] = "Pause";
	
	// Multi Code mode key mappings (these include remote id)
	keyMapUniversal["150:1_"] = "Up";
	keyMapUniversal["150:2_"] = "Down";
	keyMapUniversal["150:3_"] = "Left";
	keyMapUniversal["150:4_"] = "Right";
	keyMapUniversal["150:5_"] = "Ok";
	keyMapUniversal["150:6_"] = "Menu";
	keyMapUniversal["150:9_"] = "Left";    // For repeat
	keyMapUniversal["150:10_"] = "Right";  // For repeat
	
	keyMapUniversalRepeat["150:1_"] = true;
	keyMapUniversalRepeat["150:2_"] = true;
	keyMapUniversalRepeat["150:9_"] = true;
	keyMapUniversalRepeat["150:10_"] = true;  
    
	
	keyMapUniversal["151:1_"] = "One";
	keyMapUniversal["151:2_"] = "Two";
	keyMapUniversal["151:3_"] = "Three";
	keyMapUniversal["151:4_"] = "Four";
	keyMapUniversal["151:5_"] = "Stop";
	keyMapUniversal["151:6_"] = "Play";
	keyMapUniversal["151:9_"] = "Three";  // For repeat
	keyMapUniversal["151:10_"] = "Four";  // For repeat
	
	keyMapUniversalRepeat["151:1_"] = true;
	keyMapUniversalRepeat["151:2_"] = true;
	keyMapUniversalRepeat["151:9_"] = true;
	keyMapUniversalRepeat["151:10_"] = true;  
	
	
	keyMapUniversal["152:1_"] = "VolumePlus";
	keyMapUniversal["152:2_"] = "VolumeMinus";
	keyMapUniversal["152:3_"] = "Five";
	keyMapUniversal["152:4_"] = "Six";
	keyMapUniversal["152:5_"] = "Mute";
	keyMapUniversal["152:6_"] = "Pause";
	keyMapUniversal["152:9_"] = "Five";  // For repeat
	keyMapUniversal["152:10_"] = "Six";  // For repeat
	
	keyMapUniversalRepeat["152:1_"] = true;
	keyMapUniversalRepeat["152:2_"] = true;
	keyMapUniversalRepeat["152:9_"] = true;
	keyMapUniversalRepeat["152:10_"] = true;  
	
	keyMapUniversal["153:1_"] = "Info";
	keyMapUniversal["153:2_"] = "Prev";
	keyMapUniversal["153:3_"] = "Seven";
	keyMapUniversal["153:4_"] = "Eight";
	// Due to remote programming error, no 153:5 available
	keyMapUniversal["153:6_"] = "Enter";  // (#)
	keyMapUniversal["153:9_"] = "Seven";  // For repeat
	keyMapUniversal["153:10_"] = "Eight"; // For repeat
	
	keyMapUniversalRepeat["153:2_"] = true;
	keyMapUniversalRepeat["153:9_"] = true;
	keyMapUniversalRepeat["153:10_"] = true;  
	
	keyMapUniversal["154:1_"] = "Rewind";
	keyMapUniversal["154:2_"] = "Forward";
	keyMapUniversal["154:3_"] = "Nine";
	keyMapUniversal["154:4_"] = "Zero";
	keyMapUniversal["154:5_"] = "Clear";  // (*)
	keyMapUniversal["154:6_"] = "Sleep";
	keyMapUniversal["154:9_"] = "Nine";   // For repeat
	keyMapUniversal["154:10_"] = "Zero";  // For repeat
	
	keyMapUniversalRepeat["154:9_"] = true;
	keyMapUniversalRepeat["154:10_"] = true;  
	
	keyMapUniversal["155:1_"] = "Exit";
	keyMapUniversal["155:2_"] = "Record";
	keyMapUniversal["155:3_"] = "F1";
	keyMapUniversal["155:4_"] = "F2";
	keyMapUniversal["155:5_"] = "F3";
	keyMapUniversal["155:6_"] = "F4";
    
	keyMapUniversal["157:1_"] = "Aspect";
	keyMapUniversal["157:2_"] = "Queue";
	keyMapUniversal["157:3_"] = "F13";
	keyMapUniversal["157:4_"] = "F14";
	keyMapUniversal["157:5_"] = "Guide";
	keyMapUniversal["157:6_"] = "Power";
	
	keyMapUniversal["158:1_"] = "ChannelPlus";
	keyMapUniversal["158:2_"] = "ChannelMinus";
	keyMapUniversal["158:3_"] = "F9";
	keyMapUniversal["158:4_"] = "F10";
	keyMapUniversal["158:5_"] = "F11";
	keyMapUniversal["158:6_"] = "F12";
	
	keyMapUniversalRepeat["158:1_"] = true;
	keyMapUniversalRepeat["158:2_"] = true;  
	
	keyMapUniversal["159:1_"] = "LargeUp";
	keyMapUniversal["159:2_"] = "LargeDown";
	keyMapUniversal["159:3_"] = "Red";
	keyMapUniversal["159:4_"] = "Green";
	keyMapUniversal["159:5_"] = "Yellow";
	keyMapUniversal["159:6_"] = "Blue";
	
	keyMapUniversalRepeat["159:1_"] = true;
	keyMapUniversalRepeat["159:2_"] = true;
	
	keyMapUniversal["160:1_"] = "Replay";
	keyMapUniversal["160:2_"] = "Skip";
	keyMapUniversal["160:3_"] = "F5";
	keyMapUniversal["160:4_"] = "F6";
	keyMapUniversal["160:5_"] = "F7";
	keyMapUniversal["160:6_"] = "F8";
	
  // Keeps track of which keys send up events and repeat.
  buttonConfigMap[kRemoteButtonUp] = ButtonConfig(kButtonRepeats | kButtonSendsUpEvents);
  buttonConfigMap[kRemoteButtonDown] = ButtonConfig(kButtonRepeats | kButtonSendsUpEvents);
  buttonConfigMap[kRemoteButtonLeft] = ButtonConfig();
  buttonConfigMap[kRemoteButtonRight] = ButtonConfig();
  buttonConfigMap[kRemoteButtonLeft_Hold] = ButtonConfig(kButtonSendsUpEvents);
  buttonConfigMap[kRemoteButtonRight_Hold] = ButtonConfig(kButtonSendsUpEvents);
  buttonConfigMap[kRemoteButtonCenter] = ButtonConfig();
  buttonConfigMap[kRemoteButtonMenu_Hold] = ButtonConfig();
  buttonConfigMap[kRemoteButtonCenter_Hold] = ButtonConfig();
  buttonConfigMap[kRemoteButtonMenu] = ButtonConfig();
  buttonConfigMap[kRemoteButtonPlayPause] = ButtonConfig();
  buttonConfigMap[kRemoteButtonPlayPause_Hold] = ButtonConfig();

  // Start the reactor.
  theReactor.start();
  
  // Parse command line options.
  parseOptions(argc, argv);

  // Socket.
  pServer = new CAddress(serverAddress.c_str());

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
  {
    printf("Error creating socket.\n");
    return -1;
  }
  
  // Bind the socket to a specific local port.
  int port = 30000;
  CAddress client(port);
  while (client.Bind(sockfd) == false)
    client.SetPort(++port);
  
  // Start the XBox 360 thread.
  XBox360 xbox360;
  xbox360.start();

#ifdef ENABLE_SWITCH_EVENTS_HANDLER
  // Register for events and spawn the Carbon event loop, but only if Candelair isn't installed.
  if ([HIDRemote isCandelairInstalled] == NO) 
  {
    printf("Candelair is not installed, so we are going to set up Carbon event handlers.\n");
    const EventTypeSpec applicationEvents[] = {kEventClassApplication, kEventAppFrontSwitched,
                                               kEventClassApplication, kEventAppLaunched,
                                               kEventClassApplication, kEventAppTerminated};
    InstallApplicationEventHandler(NewEventHandlerUPP(switchEventsHandler), GetEventTypeCount(applicationEvents), applicationEvents, 0, NULL);
    
    pthread_t thread;
    pthread_create(&thread, 0, RunEventLoop, 0);
  }
#endif
	
  setupAndRun();

  // Wait for the XBox 360 thread to exit.
  xbox360.join();

  return 0;
}

void parseOptions(int argc, char** argv)
{
  optind = 0;

  bool readExternal = false;
  int  mode = REMOTE_MODE_STANDARD;
  int  timeout = 500;
  
  int c, option_index = 0;
  while ((c = getopt_long(argc, argv, options, long_options, &option_index)) != -1) 
	{
    switch (c) 
  	{
    case 'h':
      usage();
      exit(0);
      break;
    case 's':
      serverAddress = optarg;
      break;
    case 'u':
      mode = REMOTE_MODE_UNIVERAL;
      break;
    case 'm':
      mode = REMOTE_MODE_MULTI;
      break;			
    case 't':
      timeout = atoi(optarg);
      break;
    case 'v':
      verbose = true;
      break;
    case 'x':
      readExternal = true;
      break;
    case 'a':
      appLocation = optarg;
      break;
	case 'i':
	  secureInput = ((atoi(optarg) == 1) ? true : false);
	  [remoteDelegate setEnableSecureEventInputWorkaround:secureInput];
	  break;			
    default:
      usage();
      exit(1);
      break;
    }
  }
  
  // Set new configuration.
  theReactor.setMode(mode);
  theReactor.setTimeout(timeout);
  
  if (readExternal == true)
    readConfig(); 
}

void readConfig()
{
  // Compute filename.
  string strFile = getenv("HOME");
  strFile += "/Library/Application Support/" BINARY_NAME "/" PROGNAME ".conf";

  // Open file.
  ifstream ifs(strFile.c_str());
  if (!ifs)
    return;

  // Read file.
  stringstream oss;
  oss << ifs.rdbuf();

  if (!ifs && !ifs.eof())
    return;

  // Tokenize.
  string strData(oss.str());
  istringstream is(strData);
  vector<string> args = vector<string>(istream_iterator<string>(is), istream_iterator<string>());
  
  // Convert to char**.
  int argc = args.size() + 1;
  char** argv = new char*[argc + 1];
  int i = 0;
  argv[i++] = (char* )PROGNAME;
  
  for (vector<string>::iterator it = args.begin(); it != args.end(); )
    argv[i++] = (char* )(*it++).c_str();
  argv[i] = 0;
      
  // Parse the arguments.
  parseOptions(argc, argv);
   
  delete[] argv;
}

void handleSignal(int signal)
{
  // Re-read config.
  if (signal == SIGHUP || signal == SIGUSR1)
  {
    // Read configuration.
    printf("SIGHUP: Reading configuration.\n");
    readConfig();
  }
}

static long long offsetMS = 0;

long getTimeInMilliseconds()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  
  long long ms = tv.tv_sec * 1000 + tv.tv_usec/1000;
  
  if (offsetMS == 0)
    offsetMS = ms;
    
  return (long)(ms - offsetMS);
}

typedef struct kinfo_proc kinfo_proc;

static int GetBSDProcessList(kinfo_proc **procList, size_t *procCount)
    // Returns a list of all BSD processes on the system.  This routine
    // allocates the list and puts it in *procList and a count of the
    // number of entries in *procCount.  You are responsible for freeing
    // this list (use "free" from System framework).
    // On success, the function returns 0.
    // On error, the function returns a BSD errno value.
{
    int                err;
    kinfo_proc *        result;
    bool                done;
    static const int    name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
    // Declaring name as const requires us to cast it when passing it to
    // sysctl because the prototype doesn't include the const modifier.
    size_t              length;

    assert( procList != NULL);
    assert(procCount != NULL);

    *procCount = 0;

    // We start by calling sysctl with result == NULL and length == 0.
    // That will succeed, and set length to the appropriate length.
    // We then allocate a buffer of that size and call sysctl again
    // with that buffer.  If that succeeds, we're done.  If that fails
    // with ENOMEM, we have to throw away our buffer and loop.  Note
    // that the loop causes use to call sysctl with NULL again; this
    // is necessary because the ENOMEM failure case sets length to
    // the amount of data returned, not the amount of data that
    // could have been returned.

    result = NULL;
    done = false;
    do {
        assert(result == NULL);

        // Call sysctl with a NULL buffer.

        length = 0;
        err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                      NULL, &length,
                      NULL, 0);
        if (err == -1) {
            err = errno;
        }

        // Allocate an appropriately sized buffer based on the results
        // from the previous call.

        if (err == 0) {
            result = (kinfo_proc* )malloc(length);
            if (result == NULL) {
                err = ENOMEM;
            }
        }

        // Call sysctl again with the new buffer.  If we get an ENOMEM
        // error, toss away our buffer and start again.

        if (err == 0) {
            err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                          result, &length,
                          NULL, 0);
            if (err == -1) {
                err = errno;
            }
            if (err == 0) {
                done = true;
            } else if (err == ENOMEM) {
                assert(result != NULL);
                free(result);
                result = NULL;
                err = 0;
            }
        }
    } while (err == 0 && ! done);

    // Clean up and establish post conditions.

    if (err != 0 && result != NULL) {
        free(result);
        result = NULL;
    }
    *procList = result;
    if (err == 0) {
        *procCount = length / sizeof(kinfo_proc);
    }

    assert( (err == 0) == (*procList != NULL) );

    return err;
}

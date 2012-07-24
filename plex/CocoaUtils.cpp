//
//  CocoaUtils.cpp
//  Plex
//
//  Created by Max Feingold on 10/21/2011.
//  Copyright 2011 Plex Inc. All rights reserved.
//
#include <string.h>

#include "CocoaUtils.h"
#include "version.h"

#ifdef _WIN32

using namespace std;

const char* Cocoa_GetAppVersion()
{
  return APPLICATION_VERSION;
}

string Cocoa_GetLanguage()
{
  string strRet = "en-US";

  static char ret[64];
  LANGID langID = GetUserDefaultUILanguage();

  // Stackoverflow reports LOCALE_SISO639LANGNAME as fitting in 8 characters.
  int ccBuf = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO639LANGNAME, ret, 9);
  if (ccBuf != 0)
  {
    ccBuf--;
    ret[ccBuf++] = '-';
    int ccBuf2 = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME, ret+ccBuf, 32);
    if (ccBuf2 != 0)
      strRet = ret;
  }

  return strRet;
}

#endif

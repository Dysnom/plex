/*
 *  Created on: Nov 8, 2008
 *      Author: Elan Feingold
 */
#include "PlexMediaServerHelper.h"
#include "PlatformDefs.h"
#include "log.h"
#include "system.h"
#include "Settings.h"

PlexMediaServerHelper* PlexMediaServerHelper::_theInstance = 0;

/////////////////////////////////////////////////////////////////////////////
bool PlexMediaServerHelper::DoConfigure(int& mode, bool& alwaysRunning, bool& errorStarting)
{
  bool changed = false;
  
  mode = g_guiSettings.GetInt("plexmediaserver.mode");
  alwaysRunning = g_guiSettings.GetBool("plexmediaserver.alwayson");
  
  return changed;
}

/////////////////////////////////////////////////////////////////////////////
string PlexMediaServerHelper::GetConfigString()
{
  return "";
}

/////////////////////////////////////////////////////////////////////////////
void PlexMediaServerHelper::InstallLatestVersion(const string& dstDir)
{
  string src = dstDir + "/Plex Media Server.app/Contents/Resources/Plex Plug-in Installer.app";
  string dst = dstDir + "/Plex Plug-in Installer.app";
  
  printf("From [%s] to [%s]\n", src.c_str(), dst.c_str());
  
  // Move over the latest plug-in installer.
  string rsync = "/usr/bin/rsync --delete -a \"" + src + "/\" \"" + dst + "\"";
  system(rsync.c_str());
}

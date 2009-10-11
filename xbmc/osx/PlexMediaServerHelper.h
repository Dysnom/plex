#pragma once

/*
 *  Created on: Nov 8, 2008
 *      Author: Elan Feingold
 */
#include "PlexHelperApp.h"

class PlexMediaServerHelper : public PlexHelperApp
{
 public:
   
  /// Singleton method.
  static PlexMediaServerHelper& Get()
  {
    if (_theInstance == 0)
    {
      _theInstance = new PlexMediaServerHelper();
      _theInstance->Initialize();
    }
    
    return *_theInstance;
  }
  
 protected:
   
  PlexMediaServerHelper() {}
  
  virtual bool   DoConfigure(int& mode, bool& alwaysRunning, bool& errorStarting);
  virtual string GetConfigString();
  
  virtual string GetHelperBinaryName() const { return "Plex Media Server.app"; }
  virtual string GetPlistName() { return "com.plexapp.mediaserver.plist"; }
  virtual void   InstallLatestVersion(const string& dstDir);
  
 private:
   
  static PlexMediaServerHelper* _theInstance;
};

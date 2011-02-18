/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "Application.h"
#include "KeyboardLayoutConfiguration.h"
#ifdef HAS_XBOX_HARDWARE
#include "xbox/XKEEPROM.h"
#include "utils/LCD.h"
#include "xbox/IoSupport.h"
#include "xbox/XKHDD.h"
#include "xbox/xbeheader.h"
#endif
#include "Util.h"
#include "TextureManager.h"
#include "cores/PlayerCoreFactory.h"
#include "cores/dvdplayer/DVDFileInfo.h"
#include "PlayListPlayer.h"
#include "MusicDatabase.h"
#include "VideoDatabase.h"
#include "Autorun.h"
#include "ActionManager.h"
#ifdef HAS_LCD
#include "utils/LCDFactory.h"
#else
#include "GUILabelControl.h"  // needed for CInfoLabel
#include "guiImage.h"
#endif
#ifdef HAS_KAI
#include "utils/KaiClient.h"
#endif
#ifdef HAS_XBOX_HARDWARE
#include "utils/MemoryUnitManager.h"
#include "utils/FanController.h"
#include "utils/LED.h"
#endif
#include "XBVideoConfig.h"
#include "LangCodeExpander.h"
#include "utils/GUIInfoManager.h"
#include "PlayListFactory.h"
#include "GUIFontManager.h"
#include "GUIColorManager.h"
#include "SkinInfo.h"
#ifdef HAS_PYTHON
#include "lib/libPython/XBPython.h"
#endif
#include "ButtonTranslator.h"
#include "GUIAudioManager.h"
#include "lib/libscrobbler/scrobbler.h"
#include "GUIPassword.h"
#include "ApplicationMessenger.h"
#include "SectionLoader.h"
#include "cores/DllLoader/DllLoaderContainer.h"
#include "GUIUserMessages.h"
#include "FileSystem/DirectoryCache.h"
#include "FileSystem/StackDirectory.h"
#include "FileSystem/DllLibCurl.h"
#include "FileSystem/CMythSession.h"
#include "utils/TuxBoxUtil.h"
#include "utils/SystemInfo.h"
#include "ApplicationRenderer.h"
#include "GUILargeTextureManager.h"
#include "LastFmManager.h"
#include "SmartPlaylist.h"
#include "FileSystem/RarManager.h"
#include "PlayList.h"
#include "RssReader.h"
#include "Surface.h"

#if defined(FILESYSTEM) && !defined(_LINUX)
#include "FileSystem/FileDAAP.h"
#endif
#ifdef HAS_UPNP
#include "UPnP.h"
#include "FileSystem/UPnPDirectory.h"
#endif
#if defined(_LINUX) && defined(HAS_FILESYSTEM_SMB)
#include "FileSystem/SMBDirectory.h"
#endif
#include "PartyModeManager.h"
#ifdef HAS_VIDEO_PLAYBACK
#include "cores/VideoRenderers/RenderManager.h"
#endif
#ifdef HAS_KARAOKE
#include "CdgParser.h"
#endif
#include "AudioContext.h"
#include "GUIFontTTF.h"
#include "utils/Network.h"
#ifndef _LINUX
#include "utils/Win32Exception.h"
#endif
#ifdef HAS_WEB_SERVER
#include "lib/libGoAhead/XBMChttp.h"
#include "lib/libGoAhead/WebServer.h"
#endif
#ifdef HAS_FTP_SERVER
#include "lib/libfilezilla/xbfilezilla.h"
#endif
#ifdef HAS_TIME_SERVER
#include "utils/Sntp.h"
#endif
#ifdef HAS_XFONT
#include <xfont.h>  // for textout functions
#endif
#ifdef HAS_EVENT_SERVER
#include "utils/EventServer.h"
#endif
#ifdef HAS_SDL_JOYSTICK
#include "common/SDLJoystick.h"
#endif

// Windows includes
#include "GUIWindowManager.h"
#include "GUIWindowHome.h"
#include "GUIStandardWindow.h"
#include "GUIWindowSettings.h"
#include "GUIWindowFileManager.h"
#include "GUIWindowSettingsCategory.h"
#include "GUIWindowMusicPlaylist.h"
#include "GUIWindowMusicSongs.h"
#include "GUIWindowMusicNav.h"
#include "GUIWindowMusicPlaylistEditor.h"
#include "GUIWindowVideoPlaylist.h"
#include "GUIWindowMusicInfo.h"
#include "GUIWindowVideoInfo.h"
#include "GUIWindowVideoFiles.h"
#include "GUIWindowVideoNav.h"
#include "GUIWindowSettingsProfile.h"
#include "GUIWindowTestPattern.h"
#include "GUIWindowSettingsScreenCalibration.h"
#include "GUIWindowPrograms.h"
#include "GUIWindowPictures.h"
#include "GUIWindowScripts.h"
#include "GUIWindowWeather.h"
#include "GUIWindowGameSaves.h"
#include "GUIWindowLoginScreen.h"
#include "GUIWindowVisualisation.h"
//#include "GUIWindowSystemInfo.h"
#include "GUIWindowScreensaver.h"
#include "GUIWindowSlideShow.h"
#include "GUIWindowNowPlaying.h"
#include "GUIWindowPlexSearch.h"
#ifdef HAS_KAI
#include "GUIWindowBuddies.h"
#endif
#include "GUIWindowStartup.h"
#include "GUIWindowFullScreen.h"
#include "GUIWindowOSD.h"
#include "GUIWindowMusicOverlay.h"
#include "GUIWindowVideoOverlay.h"

// Dialog includes
#include "GUIDialogMusicOSD.h"
#include "GUIDialogVisualisationSettings.h"
#include "GUIDialogVisualisationPresetList.h"
#ifdef HAS_KAI
#include "GUIDialogInvite.h"
#include "GUIDialogHost.h"
#endif
#ifdef HAS_TRAINER
#include "GUIDialogTrainerSettings.h"
#endif
#include "GUIWindowScriptsInfo.h"
#include "GUIDialogNetworkSetup.h"
#include "GUIDialogMediaSource.h"
#include "GUIDialogVideoSettings.h"
#include "GUIDialogAudioSubtitleSettings.h"
#include "GUIDialogVideoBookmarks.h"
#include "GUIDialogProfileSettings.h"
#include "GUIDialogLockSettings.h"
#include "GUIDialogContentSettings.h"
#include "GUIDialogBusy.h"

#include "GUIDialogKeyboard.h"
#include "GUIDialogYesNo.h"
#include "GUIDialogOK.h"
#include "GUIDialogProgress.h"
#include "GUIDialogSelect.h"
#include "GUIDialogFileStacking.h"
#include "GUIDialogNumeric.h"
#include "GUIDialogGamepad.h"
#include "GUIDialogSubMenu.h"
#include "GUIDialogFavourites.h"
//#include "GUIDialogButtonMenu.h"
#include "GUIDialogContextMenu.h"
#include "GUIDialogPlayerControls.h"
#include "GUIDialogSongInfo.h"
#include "GUIDialogSmartPlaylistEditor.h"
#include "GUIDialogSmartPlaylistRule.h"
#include "GUIDialogPictureInfo.h"
#include "GUIDialogPluginSettings.h"
#ifdef HAS_LINUX_NETWORK
#include "GUIDialogAccessPoints.h"
#endif
#include "GUIDialogFullScreenInfo.h"
#include "GUIDialogRating.h"
#include "GUIDialogTimer.h"

#ifdef HAS_PERFORMACE_SAMPLE
#include "utils/PerformanceSample.h"
#else
#define MEASURE_FUNCTION
#endif

#ifdef HAS_SDL_AUDIO
#include <SDL/SDL_mixer.h>
#endif
#if defined(HAS_SDL) && defined(_WIN32)
#include <SDL/SDL_syswm.h>
#endif
#ifdef _WIN32
#include <shlobj.h>
#include <win32/MockXboxSymbols.h>
#endif
#ifdef HAS_XRANDR
#include "XRandR.h"
#endif
#ifdef __APPLE__
#include "CocoaUtils.h"
#include "PlexRemoteHelper.h"
#include "PlexSourceScanner.h"
#include "PlexMediaServerHelper.h"
#include "PlexMediaServerPlayer.h"
#include "PlexMediaServerQueue.h"
#include "QTPlayer.h"
#include "GUIDialogUtils.h"
#include "CoreAudioAUHAL.h"
#endif
#ifdef HAS_HAL
#include "linux/LinuxFileSystem.h"
#endif
#ifdef HAS_EVENT_SERVER
#include "utils/EventServer.h"
#endif

#include "lib/libcdio/logging.h"

using namespace std;
using namespace XFILE;
using namespace DIRECTORY;
using namespace MEDIA_DETECT;
using namespace PLAYLIST;
using namespace VIDEO;
using namespace MUSIC_INFO;
#ifdef HAS_EVENT_SERVER
using namespace EVENTSERVER;
#endif

// uncomment this if you want to use release libs in the debug build.
// Atm this saves you 7 mb of memory
#define USE_RELEASE_LIBS

#define MAX_FFWD_SPEED 5
#define CRASH_DETECTION_FILE _P("U:/CleanlyExited")

CStdString g_LoadErrorStr;

#ifdef HAS_XBOX_D3D
static void WaitCallback(DWORD flags)
{
#ifndef PROFILE
  /* if cpu is far ahead of gpu, sleep instead of yield */
  if( flags & D3DWAIT_PRESENT )
    while(D3DDevice::GetPushDistance(D3DDISTANCE_FENCES_TOWAIT) > 0)
      Sleep(1);
  else if( flags & (D3DWAIT_OBJECTLOCK | D3DWAIT_BLOCKONFENCE | D3DWAIT_BLOCKUNTILIDLE) )
    while(D3DDevice::GetPushDistance(D3DDISTANCE_FENCES_TOWAIT) > 1)
      Sleep(1);
#endif
}
#endif

CBackgroundPlayer::CBackgroundPlayer(const CFileItem &item, int iPlayList) : m_iPlayList(iPlayList)
{
  m_item = new CFileItem;
  *m_item = item;
}

CBackgroundPlayer::~CBackgroundPlayer()
{
}

void CBackgroundPlayer::Process()
{
  g_application.PlayMediaSync(*m_item, m_iPlayList);
}

//extern IDirectSoundRenderer* m_pAudioDecoder;
CApplication::CApplication(void)
    : m_ctrDpad(220, 220), m_bQuiet(false)
    , m_itemCurrentFile(new CFileItem)
{
  m_iPlaySpeed = 1;
#ifdef HAS_XBOX_HARDWARE
  m_bSpinDown = false;
  m_bNetworkSpinDown = false;
  m_dwSpinDownTime = timeGetTime();
#endif
#ifdef HAS_WEB_SERVER
  m_pWebServer = NULL;
  m_pXbmcHttp = NULL;
  m_prevMedia="";
#endif
  m_pFileZilla = NULL;
  m_pPlayer = NULL;
#ifdef HAS_XBOX_HARDWARE
  XSetProcessQuantumLength(5); //default=20msec
  XSetFileCacheSize (256*1024); //default=64kb
#endif
  m_bInactive = false;
  m_bDisplaySleeping = false;
  m_bSystemSleeping = false;
  m_bScreenSave = false;
  m_iScreenSaveLock = 0;
  m_dwShutdownTick = m_dwSaverTick = timeGetTime();
#ifdef __APPLE__
  m_dwOSXscreensaverTicks = timeGetTime();
#endif
  m_dwSkinTime = 0;
  m_bInitializing = true;
  m_eForcedNextPlayer = EPC_NONE;
  m_strPlayListFile = "";
  m_nextPlaylistItem = -1;
  m_playCountUpdated = false;
  m_bPlaybackStarting = false;
  m_bPlaybackInFullScreen = false;
  m_bBackgroundMusicEnabled = false;
  
  //true while we in IsPaused mode! Workaround for OnPaused, which must be add. after v2.0
  m_bIsPaused = false;

  /* for now allways keep this around */
#ifdef HAS_KARAOKE
  m_pCdgParser = new CCdgParser();
#endif
  m_currentStack = new CFileItemList;

#ifdef HAS_SDL
  m_frameCount = 0;
  m_frameMutex = SDL_CreateMutex();
  m_frameCond = SDL_CreateCond();
#endif

  m_bPresentFrame = false;
  m_bPlatformDirectories = false;

  m_logPath = NULL;
}

CApplication::~CApplication(void)
{
  delete m_currentStack;

  if (m_logPath)
    delete[] m_logPath;

  if (m_frameMutex)
    SDL_DestroyMutex(m_frameMutex);

  if (m_frameCond)
    SDL_DestroyCond(m_frameCond);
}

// text out routine for below
#ifdef HAS_XFONT
static void __cdecl FEH_TextOut(XFONT* pFont, int iLine, const wchar_t* fmt, ...)
{
  wchar_t buf[100];
  va_list args;
  va_start(args, fmt);
  _vsnwprintf(buf, 100, fmt, args);
  va_end(args);

  if (!(iLine & 0x8000))
    CLog::Log(LOGFATAL, "%S", buf);

  bool Center = (iLine & 0x10000) > 0;
  pFont->SetTextAlignment(Center ? XFONT_TOP | XFONT_CENTER : XFONT_TOP | XFONT_LEFT);

  iLine &= 0x7fff;

  for (int i = 0; i < 2; i++)
  {
    D3DRECT rc = { 0, 50 + 25 * iLine, 720, 50 + 25 * (iLine + 1) };
    D3DDevice::Clear(1, &rc, D3DCLEAR_TARGET, 0, 0, 0);
    pFont->TextOut(g_application.m_pBackBuffer, buf, -1, Center ? 360 : 80, 50 + 25*iLine);
    D3DDevice::Present(0, 0, 0, 0);
  }
}
#else
static void __cdecl FEH_TextOut(void* pFont, int iLine, const wchar_t* fmt, ...) {}
#endif

HWND g_hWnd = NULL;

void CApplication::InitBasicD3D()
{
#ifndef HAS_SDL
  bool bPal = g_videoConfig.HasPAL();
  CLog::Log(LOGINFO, "Init display in default mode: %s", bPal ? "PAL" : "NTSC");
  // init D3D with defaults (NTSC or PAL standard res)
  m_d3dpp.BackBufferWidth = 720;
  m_d3dpp.BackBufferHeight = bPal ? 576 : 480;
  m_d3dpp.BackBufferFormat = D3DFMT_LIN_X8R8G8B8;
  m_d3dpp.BackBufferCount = 1;
  m_d3dpp.EnableAutoDepthStencil = FALSE;
  m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
#ifdef HAS_XBOX_D3D
  m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
#else
  m_d3dpp.FullScreen_PresentationInterval = 0;
  m_d3dpp.Windowed = TRUE;
  m_d3dpp.hDeviceWindow = g_hWnd;
#endif

  if (!(m_pD3D = Direct3DCreate8(D3D_SDK_VERSION)))
  {
    CLog::Log(LOGFATAL, "FATAL ERROR: Unable to create Direct3D!");
    Sleep(INFINITE); // die
  }
#endif

  // Check if we have the required modes available
#ifndef HAS_SDL
  g_videoConfig.GetModes(m_pD3D);
#else
  g_videoConfig.GetModes();
#endif
  if (!g_graphicsContext.IsValidResolution(g_guiSettings.m_LookAndFeelResolution))
  {
    // Oh uh - doesn't look good for starting in their wanted screenmode
    CLog::Log(LOGERROR, "The screen resolution requested is not valid, resetting to a valid mode");
    g_guiSettings.m_LookAndFeelResolution = g_videoConfig.GetSafeMode();
    CLog::Log(LOGERROR, "Resetting to mode %s", g_settings.m_ResInfo[g_guiSettings.m_LookAndFeelResolution].strMode);
    CLog::Log(LOGERROR, "Done reset");
  }

  // Transfer the resolution information to our graphics context
#ifndef HAS_SDL
  g_graphicsContext.SetD3DParameters(&m_d3dpp);
#endif
  g_graphicsContext.SetVideoResolution(g_guiSettings.m_LookAndFeelResolution, TRUE);

  // Create the device
#ifdef HAS_XBOX_D3D
  // Xbox MUST use HAL / Hardware Vertex Processing!
  if (m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_d3dpp, &m_pd3dDevice) != S_OK)
  {
    CLog::Log(LOGFATAL, "FATAL ERROR: Unable to create D3D Device!");
    Sleep(INFINITE); // die
  }
  m_pd3dDevice->GetBackBuffer(0, 0, &m_pBackBuffer);
#elif !defined(HAS_SDL)
  if (m_pD3D->CreateDevice(0, D3DDEVTYPE_REF, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp, &m_pd3dDevice) != S_OK)
  {
    CLog::Log(LOGFATAL, "FATAL ERROR: Unable to create D3D Device!");
    Sleep(INFINITE); // die
  }
#endif

  if (m_splash)
  {
#ifndef HAS_SDL_OPENGL
    m_splash->Stop();
#else
    m_splash->Hide();
#endif
  }

#ifndef HAS_SDL

  m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
  m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
#endif
}

// This function does not return!
void CApplication::FatalErrorHandler(bool InitD3D, bool MapDrives, bool InitNetwork)
{
#ifdef __APPLE__
  Cocoa_DisplayError(g_LoadErrorStr);
  exit(-1);
#endif
  
  // XBMC couldn't start for some reason...
  // g_LoadErrorStr should contain the reason
  CLog::Log(LOGWARNING, "Emergency recovery console starting...");

#ifndef _XBOX
  fprintf(stderr, "Fatal error encountered, aborting\n");
  fprintf(stderr, "Error log at %sxbmc.log\n", g_stSettings.m_logFolder.c_str());
  abort();
#endif

  bool HaveGamepad = true; // should always have the gamepad when we get here
  
#ifndef __APPLE__
  if (InitD3D)
    InitBasicD3D();
#endif

  if (m_splash)
  {
#ifndef HAS_SDL_OPENGL
    m_splash->Stop();
#else
    m_splash->Hide();
#endif
  }

#ifndef HAS_SDL
  m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
  m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
#endif

  // D3D is up, load default font
#ifdef HAS_XFONT
  XFONT* pFont;
  if (XFONT_OpenDefaultFont(&pFont) != S_OK)
  {
    CLog::Log(LOGFATAL, "FATAL ERROR: Unable to open default font!");
    Sleep(INFINITE); // die
  }

  // defaults for text
  pFont->SetBkMode(XFONT_OPAQUE);
  pFont->SetBkColor(D3DCOLOR_XRGB(0, 0, 0));
  pFont->SetTextColor(D3DCOLOR_XRGB(0xff, 0x20, 0x20));
#else
  void *pFont = NULL;
#endif
  int iLine = 0;
  FEH_TextOut(pFont, iLine++, L"XBMC Fatal Error:");
  char buf[500];
  strncpy(buf, g_LoadErrorStr.c_str(), 500);
  buf[499] = 0;
  char* pNewline = strtok(buf, "\n");
  while (pNewline)
  {
    FEH_TextOut(pFont, iLine++, L"%S", pNewline);
    pNewline = strtok(NULL, "\n");
  }
  ++iLine;

  bool Pal = g_graphicsContext.GetVideoResolution() == PAL_4x3;

  if (HaveGamepad)
    FEH_TextOut(pFont, (Pal ? 16 : 12) | 0x18000, L"Press any button to reboot");


#ifndef HAS_XBOX_NETWORK
  bool NetworkUp = m_network.IsAvailable();
#endif

  if( NetworkUp )
  {
#ifdef HAS_LINUX_NETWORK
    FEH_TextOut(pFont, iLine++, L"IP Address: %S", m_network.GetFirstConnectedInterface()->GetCurrentIPAddress().c_str());
#else
    FEH_TextOut(pFont, iLine++, L"IP Address: %S", m_network.m_networkinfo.ip);
#endif
    ++iLine;
  }

  if (NetworkUp)
  {
#ifdef HAS_FTP_SERVER
    if (!m_pFileZilla)
    {
      // Start FTP with default settings
      FEH_TextOut(pFont, iLine++, L"Starting FTP server...");
      StartFtpEmergencyRecoveryMode();
    }

    FEH_TextOut(pFont, iLine++, L"FTP server running on port %d, login: xbox/xbox", m_pFileZilla->mSettings.GetServerPort());
#endif
    ++iLine;
  }

  if (HaveGamepad)
  {
    for (;;)
    {
      Sleep(50);
      if (AnyButtonDown())
      {
        g_application.Stop();
        Sleep(200);
#ifdef _XBOX
#ifndef _DEBUG  // don't actually shut off if debug build, it hangs VS for a long time
        XKUtils::XBOXPowerCycle();
#endif
#elif !defined(HAS_SDL)
        SendMessage(g_hWnd, WM_CLOSE, 0, 0);
#endif
      }
    }
  }
  else
  {
#ifdef _XBOX
    Sleep(INFINITE);
#elif !defined(HAS_SDL)
    SendMessage(g_hWnd, WM_CLOSE, 0, 0);
#endif
  }
}

#ifndef _LINUX
LONG WINAPI CApplication::UnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
  PCSTR pExceptionString = "Unknown exception code";

#define STRINGIFY_EXCEPTION(code) case code: pExceptionString = #code; break

  switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
    STRINGIFY_EXCEPTION(EXCEPTION_ACCESS_VIOLATION);
    STRINGIFY_EXCEPTION(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
    STRINGIFY_EXCEPTION(EXCEPTION_BREAKPOINT);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_DENORMAL_OPERAND);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_DIVIDE_BY_ZERO);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_INEXACT_RESULT);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_INVALID_OPERATION);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_OVERFLOW);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_STACK_CHECK);
    STRINGIFY_EXCEPTION(EXCEPTION_ILLEGAL_INSTRUCTION);
    STRINGIFY_EXCEPTION(EXCEPTION_INT_DIVIDE_BY_ZERO);
    STRINGIFY_EXCEPTION(EXCEPTION_INT_OVERFLOW);
    STRINGIFY_EXCEPTION(EXCEPTION_INVALID_DISPOSITION);
    STRINGIFY_EXCEPTION(EXCEPTION_NONCONTINUABLE_EXCEPTION);
    STRINGIFY_EXCEPTION(EXCEPTION_SINGLE_STEP);
  }
#undef STRINGIFY_EXCEPTION

  g_LoadErrorStr.Format("%s (0x%08x)\n at 0x%08x",
                        pExceptionString, ExceptionInfo->ExceptionRecord->ExceptionCode,
                        ExceptionInfo->ExceptionRecord->ExceptionAddress);

  CLog::Log(LOGFATAL, "%s", g_LoadErrorStr.c_str());

  return ExceptionInfo->ExceptionRecord->ExceptionCode;
}
#endif

#ifdef _XBOX
#include "xbox/Undocumented.h"
extern "C" HANDLE __stdcall KeGetCurrentThread(VOID);
#endif
extern "C" void __stdcall init_emu_environ();

//
// Utility function used to copy files from the application bundle
// over to the user data directory in Application Support/Plex.
//
static void CopyUserDataIfNeeded(const CStdString& libraryPath, const CStdString& strPath, LPCTSTR file)
{
  CStdString dstPath = libraryPath;
  dstPath.append(PATH_SEPARATOR_STRING);
  dstPath.append(strPath);
  dstPath.append(PATH_SEPARATOR_STRING);
  dstPath.append(file);
  if (access(dstPath.c_str(), 0) == -1)
  {
    CStdString srcPath = _P("q:\\");
    srcPath.append(strPath);
    srcPath.append(PATH_SEPARATOR_STRING);
    srcPath.append(file);
    
    if (GetFileAttributes(srcPath.c_str()) & FILE_ATTRIBUTE_DIRECTORY)
      CopyDirectory(srcPath.c_str(), dstPath.c_str(), TRUE);
    else
      CopyFile(srcPath.c_str(), dstPath.c_str(), TRUE);
  }
}

HRESULT CApplication::Create(HWND hWnd)
{
#ifdef _LINUX
  tzset();   // Initialize timezone information variables
#endif

  g_hWnd = hWnd;

#ifndef HAS_SDL
  HRESULT hr = S_OK;
#endif
  // map Q to home drive of xbe to load the config file
  static CStdString strExecutablePath;
  CUtil::GetHomePath(strExecutablePath);

  // Check logpath...needs to be done before the first log to be meaningful.
#ifdef _LINUX
  if (m_bPlatformDirectories)
  {
#ifdef __APPLE__
    CStdString logDir = getenv("HOME");
    logDir.append("/Library/Logs/");
    g_stSettings.m_logFolder = logDir;
#else
    g_stSettings.m_logFolder = "/var/tmp/";
#endif
  }
  else
  {
    m_logPath = new char[MAX_PATH];
    snprintf(m_logPath, MAX_PATH, "%s/", strExecutablePath.c_str());
    g_stSettings.m_logFolder = m_logPath;
  }
#endif

  // Grab a handle to our thread to be used later in identifying the render thread.
  m_threadID = GetCurrentThreadId();

  init_emu_environ();

#ifndef _LINUX
  //floating point precision to 24 bits (faster performance)
  _controlfp(_PC_24, _MCW_PC);


  /* install win32 exception translator, win32 exceptions
   * can now be caught using c++ try catch */
  win32_exception::install_handler();
#endif

#ifdef HAS_XBOX_HARDWARE
  char szDevicePath[MAX_PATH];

  CIoSupport::GetPartition(strExecutablePath.c_str()[0], szDevicePath);
  strcat(szDevicePath, &strExecutablePath.c_str()[2]);

  CIoSupport::RemapDriveLetter('Q', szDevicePath);
#endif

  CProfile *profile;

  // only the InitDirectories* for the current platform should return
  // non-null (if at all i.e. to set a profile)
  // putting this before the first log entries saves another ifdef for g_stSettings.m_logFolder
  profile = InitDirectoriesLinux();
  if (!profile)
    profile = InitDirectoriesOSX();
  if (!profile)
    profile = InitDirectoriesWin32();
  if (profile)
  {
    profile->setName("Master user");
    profile->setLockMode(LOCK_MODE_EVERYONE);
    profile->setLockCode("");
    profile->setDate("");
    g_settings.m_vecProfiles.push_back(*profile);
    delete profile;
  }
#ifndef _WIN32PC
  InitDirectories();
#endif

  CLog::Log(LOGNOTICE, "-----------------------------------------------------------------------");
#if defined(_LINUX) && !defined(__APPLE__)
  CLog::Log(LOGNOTICE, "Starting XBMC, Platform: GNU/Linux.  Built on %s (SVN:%s)", __DATE__, SVN_REV);
#elif defined(__APPLE__)
  CLog::Log(LOGNOTICE, "Starting Plex, Platform: Mac OS X.  Built on %s", __DATE__);
  CLog::Log(LOGNOTICE, "Plex version %s.", Cocoa_GetAppVersion());
#elif defined(_WIN32) && !defined(HAS_XBOX_HARDWARE)
  CLog::Log(LOGNOTICE, "Starting XBMC, Platform: Windows.  Built on %s (compiler %i)", __DATE__, _MSC_VER);
#else
  CLog::Log(LOGNOTICE, "Starting XBMC, Platform: Xbox.  Built on %s", __DATE__);
#endif
  CLog::Log(LOGNOTICE, "Q is mapped to: %s", strExecutablePath.c_str());
  char szXBEFileName[1024];
  CIoSupport::GetXbePath(szXBEFileName);
  CStdString strLogFile;
  strLogFile.Format("%sPlex.log", _P(g_stSettings.m_logFolder).c_str());

  CLog::Log(LOGNOTICE, "The executable running is: %s", szXBEFileName);
  CLog::Log(LOGNOTICE, "Log File is located: %s", strLogFile.c_str());
  CLog::Log(LOGNOTICE, "-----------------------------------------------------------------------");

  // if we are running from DVD our UserData location will be TDATA
  if (CUtil::IsDVD(strExecutablePath))
  {
    // TODO: Should we copy over any UserData folder from the DVD?
    if (!CFile::Exists("T:\\guisettings.xml")) // first run - cache userdata folder
    {
      CFileItemList items;
      CUtil::GetRecursiveListing("q:\\userdata",items,"");
      for (int i=0;i<items.Size();++i)
          CFile::Cache(items[i]->m_strPath,"T:\\"+CUtil::GetFileName(items[i]->m_strPath));
    }
    g_settings.m_vecProfiles[0].setDirectory("T:\\");
    g_stSettings.m_logFolder = "T:\\";
  }
  else
  {
#ifdef HAS_XBOX_HARDWARE
    CStdString strMnt = g_settings.GetUserDataFolder();
    if (g_settings.GetUserDataFolder().Left(2).Equals("Q:"))
    {
      CUtil::GetHomePath(strMnt);
      strMnt += g_settings.GetUserDataFolder().substr(2);
    }

    CIoSupport::GetPartition(strMnt.c_str()[0], szDevicePath);
    strcat(szDevicePath, &strMnt.c_str()[2]);
    CIoSupport::RemapDriveLetter('T',szDevicePath);
#endif
  }

#ifdef HAS_XRANDR
  g_xrandr.LoadCustomModeLinesToAllOutputs();
#endif

#ifndef HAS_SDL
  CLog::Log(LOGNOTICE, "Setup DirectX");
  // Create the Direct3D object
  if ( NULL == ( m_pD3D = Direct3DCreate8(D3D_SDK_VERSION) ) )
  {
    CLog::Log(LOGFATAL, "XBAppEx: Unable to create Direct3D!" );
    return E_FAIL;
  }
#else
  CLog::Log(LOGNOTICE, "Setup SDL");

#ifndef __APPLE__
  /* Clean up on exit, exit on window close and interrupt */
  atexit(SDL_Quit);
#endif

  Uint32 sdlFlags = SDL_INIT_VIDEO;

#ifdef HAS_SDL_AUDIO
  sdlFlags |= SDL_INIT_AUDIO;
#endif

#ifndef __APPLE__
#ifdef HAS_SDL_JOYSTICK
  sdlFlags |= SDL_INIT_JOYSTICK;
#endif
#endif


#ifdef _LINUX
  // for nvidia cards - vsync currently ALWAYS enabled.
  // the reason is that after screen has been setup changing this env var will make no difference.
  setenv("__GL_SYNC_TO_VBLANK","1",true);
#endif

  if (SDL_Init(sdlFlags) != 0)
  {
        CLog::Log(LOGFATAL, "XBAppEx: Unable to initialize SDL: %s", SDL_GetError());
        return E_FAIL;
  }

  // for python scripts that check the OS
#ifdef __APPLE__
  setenv("OS","OS X",true);
#elif defined(_LINUX)
  setenv("OS","Linux",true);
#endif
#endif

  //list available videomodes
#ifndef HAS_SDL
  g_videoConfig.GetModes(m_pD3D);
  //init the present parameters with values that are supported
  RESOLUTION initialResolution = g_videoConfig.GetInitialMode(m_pD3D, &m_d3dpp);
  g_graphicsContext.SetD3DParameters(&m_d3dpp);
#else
  g_videoConfig.GetModes();
  //init the present parameters with values that are supported
  RESOLUTION initialResolution = g_videoConfig.GetInitialMode();
#endif
  
  // Reset FPS to the display FPS. 
  g_infoManager.ResetFPS(g_graphicsContext.GetFPS());

#ifndef __APPLE__
  // Transfer the resolution information to our graphics context
  g_graphicsContext.SetVideoResolution(initialResolution, TRUE);
#endif

  // Initialize core peripheral port support. Note: If these parameters
  // are 0 and NULL, respectively, then the default number and types of
  // controllers will be initialized.
#ifdef HAS_XBOX_HARDWARE
  XInitDevices( m_dwNumInputDeviceTypes, m_InputDeviceTypes );

  // Create the gamepad devices
  if ( FAILED(hr = XBInput_CreateGamepads(&m_Gamepad)) )
  {
    CLog::Log(LOGERROR, "XBAppEx: Call to CreateGamepads() failed!" );
    return hr;
  }

  if ( FAILED(hr = XBInput_CreateIR_Remotes()) )
  {
    CLog::Log(LOGERROR, "XBAppEx: Call to CreateIRRemotes() failed!" );
    return hr;
  }
#endif

#if defined(HAS_SDL) && defined(_WIN32)
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version)
  int te = SDL_GetWMInfo( &wmInfo );
  g_hWnd = wmInfo.window;
#endif

  // Create the Mouse and Keyboard devices
  g_Mouse.Initialize(&hWnd);
  g_Keyboard.Initialize(hWnd);
#ifdef HAS_LIRC
  g_RemoteControl.Initialize();
#endif
#ifdef HAS_SDL_JOYSTICK
  g_Joystick.Initialize(hWnd);
#endif

  CLog::Log(LOGINFO, "Drives are mapped");

  CLog::Log(LOGNOTICE, "load settings...");
  g_LoadErrorStr = "Unable to load settings";
  g_settings.m_iLastUsedProfileIndex = g_settings.m_iLastLoadedProfileIndex;
  if (g_settings.bUseLoginScreen && g_settings.m_iLastLoadedProfileIndex != 0)
    g_settings.m_iLastLoadedProfileIndex = 0;

  m_bAllSettingsLoaded = g_settings.Load(m_bXboxMediacenterLoaded, m_bSettingsLoaded);
  if (!m_bAllSettingsLoaded)
    FatalErrorHandler(true, true, true);

  // Check for X+Y - if pressed, set debug log mode and mplayer debuging on
  CheckForDebugButtonCombo();

#ifdef __APPLE__
  // Configure and possible manually start the helpers.
  PlexRemoteHelper::Get().Configure();
  PlexMediaServerHelper::Get().Configure();
  
#ifndef _DEBUG
  // See if we crashed the last time.
  if (CFile::Exists(CRASH_DETECTION_FILE))
  {
    // Oops, make sure we restart the media server just in case.
    if (g_guiSettings.GetBool("plexmediaserver.alwayson"))
      PlexMediaServerHelper::Get().Restart();
  }
  else
  {
    // Create the file so we know if we crashed on exit.
    ofstream out(CRASH_DETECTION_FILE);
  }
#endif
  
  // Note that the screensaver should turn off.
  Cocoa_UpdateSystemActivity();
  Cocoa_TurnOffScreenSaver();

  // Make sure audio device is set to something real.
  PlexAudioDevices::Initialize();
  PlexAudioDevicePtr device = PlexAudioDevices::FindByName(g_guiSettings.GetString("audiooutput.audiodevice"));
  if (!device)
  {
    device = PlexAudioDevices::FindDefault();
    if (device)
      g_guiSettings.SetString("audiooutput.audiodevice", device->getName().c_str());
  }
  
  // Reset the device.
  if (device)
    device->reset();
  
  // Set the correct audio device.
  if (device && g_guiSettings.GetBool("audiooutput.systemoutputfollows"))
  {
    // Save the default so we can restore it.
    m_defaultSystemDevice = PlexAudioDevices::FindDefault();
    printf("Saving old default of %s\n", m_defaultSystemDevice->getName().c_str());
    
    // Set the one in the preferences.
    device->setDefault();
  }
  
  // Start background music playing
  m_bBackgroundMusicEnabled = g_guiSettings.GetBool("backgroundmusic.bgmusicenabled");
  Cocoa_UpdateGlobalVolume(g_application.GetVolume());
  Cocoa_SetBackgroundMusicEnabled(m_bBackgroundMusicEnabled);
  Cocoa_SetBackgroundMusicThemesEnabled(g_guiSettings.GetBool("backgroundmusic.thememusicenabled"));
  Cocoa_SetBackgroundMusicThemeDownloadsEnabled(g_guiSettings.GetBool("backgroundmusic.themedownloadsenabled"));
  Cocoa_SetBackgroundMusicVolume((float)(g_guiSettings.GetInt("backgroundmusic.bgmusicvolume")/100.0f));
  Cocoa_StartBackgroundMusic();
  
  Cocoa_SetKeyboardBacklightControlEnabled(g_advancedSettings.m_bEnableKeyboardBacklightControl);
#endif

  CStdString strHomePath = "Q:";
  CLog::Log(LOGINFO, "Checking skinpath existence, and existence of keymap.xml:%s...", (strHomePath + "\\skin").c_str());
  CStdString keymapPath;

  keymapPath = g_settings.GetUserDataItem("Keymap.xml");

#ifdef _XBOX
  if (access(strHomePath + "\\skin", 0) || access(keymapPath.c_str(), 0))
  {
    g_LoadErrorStr = "Unable to find skin or Keymap.xml.  Make sure you have UserData/Keymap.xml and Skin/ folder";
    FatalErrorHandler(true, false, true);
  }
#endif

  if (!g_graphicsContext.IsValidResolution(g_guiSettings.m_LookAndFeelResolution))
  {
    // Oh uh - doesn't look good for starting in their wanted screenmode
    CLog::Log(LOGERROR, "The screen resolution requested is not valid, resetting to a valid mode");
    g_guiSettings.m_LookAndFeelResolution = initialResolution;
  }
  // Transfer the new resolution information to our graphics context
#if !defined(HAS_XBOX_D3D) && !defined(HAS_SDL)
  m_d3dpp.Windowed = TRUE;
  m_d3dpp.hDeviceWindow = g_hWnd;
#else
#define D3DCREATE_MULTITHREADED 0
#endif

#ifndef HAS_SDL
  g_graphicsContext.SetD3DParameters(&m_d3dpp);
#endif
  g_graphicsContext.SetVideoResolution(g_guiSettings.m_LookAndFeelResolution, TRUE);

  // set GUI res and force the clear of the screen
  g_graphicsContext.SetVideoResolution(g_guiSettings.m_LookAndFeelResolution, TRUE, true);

  // initialize our charset converter
  g_charsetConverter.reset();

  // Load the langinfo to have user charset <-> utf-8 conversion
  CStdString strLanguage = g_settings.GetLanguage();
  
  strLanguage[0] = toupper(strLanguage[0]);

  CStdString strLangInfoPath;
  strLangInfoPath.Format("Q:\\language\\%s\\langinfo.xml", strLanguage.c_str());
  strLangInfoPath = _P(strLangInfoPath);

  CLog::Log(LOGINFO, "load language info file: %s", strLangInfoPath.c_str());
  g_langInfo.Load(strLangInfoPath);

  CStdString strKeyboardLayoutConfigurationPath;
  strKeyboardLayoutConfigurationPath.Format("Q:\\language\\%s\\keyboardmap.xml", strLanguage.c_str());
  strKeyboardLayoutConfigurationPath = _P(strKeyboardLayoutConfigurationPath);
  CLog::Log(LOGINFO, "load keyboard layout configuration info file: %s", strKeyboardLayoutConfigurationPath.c_str());
  g_keyboardLayoutConfiguration.Load(strKeyboardLayoutConfigurationPath);

  m_splash = new CSplash(_P("Q:\\media\\splash.png"));
#ifndef HAS_SDL_OPENGL
  m_splash->Start();
#else
  m_splash->Show();
#endif

  CStdString strLanguagePath;
  strLanguagePath.Format("Q:\\language\\%s\\strings.xml", strLanguage.c_str());
  strLanguagePath = _P(strLanguagePath);

  CLog::Log(LOGINFO, "load language file:%s", strLanguagePath.c_str());
  if (!g_localizeStrings.Load(_P(strLanguagePath)))
    FatalErrorHandler(false, false, true);

  CLog::Log(LOGINFO, "load keymapping");
  if (!g_buttonTranslator.Load())
    FatalErrorHandler(false, false, true);

  // check the skin file for testing purposes
  CStdString strSkinBase = _P("Q:\\skin\\");
  CStdString strSkinPath = strSkinBase + g_guiSettings.GetString("lookandfeel.skin");
  CLog::Log(LOGINFO, "Checking skin version of: %s", g_guiSettings.GetString("lookandfeel.skin").c_str());
  if (!g_SkinInfo.Check(strSkinPath))
  {
    // reset to the default skin (MediaStream)
    CLog::Log(LOGINFO, "The above skin isn't suitable - checking the version of the default: %s", "MediaStream");
    strSkinPath = strSkinBase + "MediaStream";
    if (!g_SkinInfo.Check(strSkinPath))
    {
      g_LoadErrorStr.Format("No suitable skin version found.\nWe require at least version %5.4f \n", g_SkinInfo.GetMinVersion());
      FatalErrorHandler(false, false, true);
    }
  }
  int iResolution = g_graphicsContext.GetVideoResolution();
  CLog::Log(LOGINFO, " GUI format %ix%i %s",
            g_settings.m_ResInfo[iResolution].iWidth,
            g_settings.m_ResInfo[iResolution].iHeight,
            g_settings.m_ResInfo[iResolution].strMode);
  m_gWindowManager.Initialize();

#ifdef HAS_PYTHON
  g_actionManager.SetScriptActionCallback(&g_pythonParser);
#endif

  // show recovery console on fatal error instead of freezing
  CLog::Log(LOGINFO, "install unhandled exception filter");
#ifndef _LINUX
  SetUnhandledExceptionFilter(UnhandledExceptionFilter);
#endif

#ifdef HAS_XBOX_D3D
  D3DDevice::SetWaitCallback(WaitCallback);
#endif

  g_Mouse.SetEnabled(g_guiSettings.GetBool("appleremote.enablemouse"));

  if (!m_bQuiet)
    m_bQuiet = !g_guiSettings.GetBool("system.debuglogging");
  
  return CXBApplicationEx::Create(hWnd);
}

void CApplication::InitDirectories()
{
  if (m_bPlatformDirectories)
    return;

  CStdString strMnt = g_settings.GetUserDataFolder();
  if (g_settings.GetUserDataFolder().Left(2).Equals("Q:"))
  {
    CUtil::GetHomePath(strMnt);
    strMnt += g_settings.GetUserDataFolder().substr(2);
  }
  CIoSupport::RemapDriveLetter('T',(char*) strMnt.c_str());
}

CProfile* CApplication::InitDirectoriesLinux()
{
/*
   The following is the directory mapping for Platform Specific Mode:

   Q: => [read-only] system directory (/usr/share/xbmc)
   U: => [read-write] user's directory that will override Q: system-wide
         installations like skins, screensavers, etc.
         ($HOME/.xbmc)
         NOTE: XBMC will look in both Q:\skin and U:\skin for skins. Same
         applies to screensavers, sounds, etc.
   T: => [read-write] userdata of master profile. It will by default be
         mapped to U:\userdata ($HOME/.xbmc/userdata)
   P: => [read-write] current profile's userdata directory.
         Generally T:\ for the master profile or T:\profiles\<profile_name>
         for other profiles.

   NOTE: All these root directories are lowercase. Some of the sub-directories
         might be mixed case.
*/

#if defined(_LINUX) && !defined(__APPLE__)
  CProfile* profile = NULL;
  CStdString strExecutablePath;

  CUtil::GetHomePath(strExecutablePath);

  // Z: common for both
  CIoSupport::RemapDriveLetter('Z',"/tmp/xbmc");
  CreateDirectory(_P("Z:\\"), NULL);

  CStdString userHome;
  if (getenv("HOME"))
  {
    userHome = getenv("HOME");
  }
  else
  {
    userHome = "/root";
  }

  if (m_bPlatformDirectories)
  {
    CStdString str = INSTALL_PATH;
    CIoSupport::RemapDriveLetter('Q', (char*) str.c_str());

    // make the $HOME/.xbmc directory
    CStdString xbmcHome = userHome + "/.xbmc";
    CreateDirectory(xbmcHome, NULL);
    CIoSupport::RemapDriveLetter('U', xbmcHome.c_str());

    // make the $HOME/.xbmc/userdata directory
    CStdString xbmcUserdata = xbmcHome + "/userdata";
    CreateDirectory(xbmcUserdata.c_str(), NULL);
    CIoSupport::RemapDriveLetter('T', xbmcUserdata.c_str());

    CStdString xbmcDir;
    xbmcDir = _P("u:\\skin");
    CreateDirectory(xbmcDir.c_str(), NULL);

    xbmcDir = _P("u:\\visualisations");
    CreateDirectory(xbmcDir.c_str(), NULL);

    xbmcDir = _P("u:\\screensavers");
    CreateDirectory(xbmcDir.c_str(), NULL);

    xbmcDir = _P("u:\\sounds");
    CreateDirectory(xbmcDir.c_str(), NULL);

    xbmcDir = _P("u:\\system");
    CreateDirectory(xbmcDir.c_str(), NULL);

    xbmcDir = _P("u:\\scripts");
    CreateDirectory(xbmcDir.c_str(), NULL);
    xbmcDir = _P("u:\\scripts\\My Scripts"); // FIXME: both scripts should be in 1 directory
    CreateDirectory(xbmcDir.c_str(), NULL);

    xbmcDir = _P("u:\\scripts\\Common Scripts"); // FIXME:
    symlink( INSTALL_PATH "/scripts",  xbmcDir.c_str() );

    // copy required files
    CopyUserDataIfNeeded(_P("t:\\"), "Keymap.xml");  // Eventual FIXME.
    CopyUserDataIfNeeded(_P("t:\\"), "RssFeeds.xml");
    CopyUserDataIfNeeded(_P("t:\\"), "Lircmap.xml");
  }
  else
  {
    CIoSupport::RemapDriveLetter('Q', (char*) strExecutablePath.c_str());
    CIoSupport::RemapDriveLetter('T', _P("Q:\\userdata"));
    CIoSupport::RemapDriveLetter('U', "/dev/null");
  }

  g_settings.m_vecProfiles.clear();
  g_settings.LoadProfiles(_P( PROFILES_FILE ));

  if (g_settings.m_vecProfiles.size()==0)
  {
    profile = new CProfile;
    profile->setDirectory(_P("t:\\"));
  }
  return profile;
#else
  return NULL;
#endif
}

CProfile* CApplication::InitDirectoriesOSX()
{
#ifdef __APPLE__
  Cocoa_Initialize(this);

  // We're going to manually manage the screensaver.
  setenv("SDL_VIDEO_ALLOW_SCREENSAVER", "1", true);
  setenv("SDL_ENABLEAPPEVENTS", "1", 1);

  CStdString strExecutablePath;
  CUtil::GetHomePath(strExecutablePath);
  setenv("PLEX_HOME", strExecutablePath.c_str(), 0);
  
  // Set the working directory to be the Plex resource directory. This
  // allows two-step dynamic library loading to work, as long as the load
  // paths are configured correctly.
  //
  CStdString strWorkingDir = strExecutablePath;
  chdir(strWorkingDir.c_str());
  
  // Z: common for both
  CIoSupport::RemapDriveLetter('Z',"/tmp/plex");
  CreateDirectory(_P("Z:\\"), NULL);

  CStdString home = getenv("HOME");
  CIoSupport::RemapDriveLetter('Q', (char*) strExecutablePath.c_str());

  CProfile* profile = NULL;

  if (m_bPlatformDirectories)
  {
    // Make sure the required directories exist.
    CStdString str = home;

    // Put the user data folder somewhere standard for the platform.
    CStdString homeDir = getenv("HOME");
    homeDir.append("/Library/Application Support/Plex");
    CIoSupport::RemapDriveLetter('T', homeDir.c_str());
    CIoSupport::RemapDriveLetter('U', homeDir.c_str());
    
    str.append("/Library/Application Support");
    CreateDirectory(str.c_str(), NULL);
    str.append("/Plex");
    CreateDirectory(str.c_str(), NULL);
    CStdString str2 = str;
    str2.append("/Mounts");
    CreateDirectory(str2.c_str(), NULL);
    str2 = str;
    str2.append("/skin");
    CreateDirectory(str2.c_str(), NULL);
    CStdString str3 = str;
    str3.append("/userdata");
    CreateDirectory(str3.c_str(), NULL);
    CStdString str4 = str;
    str4.append("/scripts");
    CreateDirectory(str4.c_str(), NULL);

    CreateDirectory(_P("U:\\userdata\\Database"), NULL);
    CreateDirectory(_P("U:\\plugins"), NULL);
    CreateDirectory(_P("U:\\plugins\\music"), NULL);
    CreateDirectory(_P("U:\\plugins\\video"), NULL);
    CreateDirectory(_P("U:\\plugins\\pictures"), NULL);
    CreateDirectory(_P("U:\\Background Music"), NULL);
    CreateDirectory(_P("U:\\Background Music\\Main"), NULL);
    CreateDirectory(_P("U:\\Background Music\\Themes"), NULL);
    
    // Install things as needed from our "virgin" copies.
    CopyUserDataIfNeeded(str, "userdata", "RssFeeds.xml");
    CopyUserDataIfNeeded(str, "userdata", "Database/ViewModes.db");

    // Create a reasonable sources.xml if one doesn't exist already.
    CStdString sourcesFile = str3;
    sourcesFile.append("/Sources.xml");

    CStdString sampleSourcesFile = strExecutablePath;
    sampleSourcesFile.append("/userdata/Sources.xml");

    if (::access(sourcesFile.c_str(), R_OK)       != 0 &&
        ::access(sampleSourcesFile.c_str(), R_OK) == 0)
    {
      // Read the sample.
      CLog::Log(LOGINFO, "Creating sample sources.xml file.");
      string strSources = PlexHelperApp::ReadFile(sampleSourcesFile.c_str());

      // Replace with real home directory.
      CStdString strHome = getenv("HOME");
      for (int start = 0; (start=strSources.find("${HOME}")) != string::npos; )
        strSources.replace(start, 7, strHome.c_str(), strHome.length());

      // Write the sample.
      PlexHelperApp::WriteFile(sourcesFile.c_str(), strSources);
    }
  }

  g_settings.m_vecProfiles.clear();
  g_settings.LoadProfiles(_P(PROFILES_FILE));

  if (g_settings.m_vecProfiles.size() == 0)
  {
    profile = new CProfile();
    profile->setDirectory(_P("t:\\userdata"));
  }

  return profile;
#else
  return NULL;
#endif
}

CProfile* CApplication::InitDirectoriesWin32()
{
#ifdef _WIN32PC

  CProfile* profile = NULL;
  CStdString strExecutablePath;
  CStdString strWin32UserFolder,strPath;

  CUtil::GetHomePath(strExecutablePath);
  SetEnvironmentVariable("XBMC_HOME", strExecutablePath.c_str());

  CIoSupport::RemapDriveLetter('Q', (char*) strExecutablePath.c_str());

  if (m_bPlatformDirectories)
  {
    TCHAR szPath[MAX_PATH];

    if(SUCCEEDED(SHGetFolderPath(NULL,CSIDL_APPDATA|CSIDL_FLAG_CREATE,NULL,0,szPath)))
      strWin32UserFolder = szPath;
    else
      strWin32UserFolder = strExecutablePath;

    // create user/app data/XBMC
    CUtil::AddFileToFolder(strWin32UserFolder,"XBMC",strPath);
    CreateDirectory(strPath.c_str(), NULL);
    // create user/app data/XBMC/cache
    CUtil::AddFileToFolder(strPath,"cache",strPath);
    CreateDirectory(strPath.c_str(), NULL);
    CIoSupport::RemapDriveLetter('Z',strPath.c_str());
    // create user/app data/XBMC/UserData
    CUtil::AddFileToFolder(strWin32UserFolder,"XBMC\\UserData",strPath);
    CreateDirectory(strPath.c_str(), NULL);
    CIoSupport::RemapDriveLetter('T', strPath.c_str());
    // See if the keymap file exists, and if not, copy it from our "virgin" one.
    CopyUserDataIfNeeded(strPath, "Keymap.xml");
    CopyUserDataIfNeeded(strPath, "RssFeeds.xml");
    CopyUserDataIfNeeded(strPath, "favourites.xml");
  }
  else
  {
    CUtil::AddFileToFolder(strExecutablePath,"cache",strPath);
    CIoSupport::RemapDriveLetter('Z',strPath.c_str());
    CreateDirectory(_P("Z:\\"), NULL);
    CUtil::AddFileToFolder(strExecutablePath,"UserData",strPath);
    CIoSupport::RemapDriveLetter('T',strPath.c_str());
  }

  g_settings.m_vecProfiles.clear();
  g_settings.LoadProfiles(_P(PROFILES_FILE));

  if (m_bPlatformDirectories)
  {

    if (g_settings.m_vecProfiles.size()==0)
    {
      profile = new CProfile;
      CUtil::AddFileToFolder(strWin32UserFolder,"XBMC\\UserData",strPath);
      profile->setDirectory(strPath.c_str());
    }
  }
  else
  {
    if (g_settings.m_vecProfiles.size()==0)
    {
      profile = new CProfile;
      profile->setDirectory(_P("q:\\UserData"));
    }
  }

    return profile;
#else
  return NULL;
#endif

}

HRESULT CApplication::Initialize()
{
  // turn off cdio logging
  cdio_loglevel_default = CDIO_LOG_ERROR;

  CLog::Log(LOGINFO, "creating subdirectories");

  //CLog::Log(LOGINFO, "userdata folder: %s", g_stSettings.m_userDataFolder.c_str());
  CLog::Log(LOGINFO, "userdata folder: %s", g_settings.GetProfileUserDataFolder().c_str());
  CLog::Log(LOGINFO, "  recording folder:%s", g_guiSettings.GetString("mymusic.recordingpath",false).c_str());
  CLog::Log(LOGINFO, "  screenshots folder:%s", g_guiSettings.GetString("pictures.screenshotpath",false).c_str());

  // UserData folder layout:
  // UserData/
  //   Database/
  //     CDDb/
  //   Thumbnails/
  //     Music/
  //       temp/
  //     0 .. F/
  //     XLinkKai/

  CreateDirectory(g_settings.GetUserDataFolder().c_str(), NULL);
  CreateDirectory(g_settings.GetProfileUserDataFolder().c_str(), NULL);
  g_settings.CreateProfileFolders();

  CreateDirectory(g_settings.GetProfilesThumbFolder().c_str(),NULL);

  CreateDirectory(_P("Z:\\temp"), NULL); // temp directory for python and dllGetTempPathA
  CreateDirectory(_P("Q:\\scripts"), NULL);
  CreateDirectory(_P("Q:\\plugins"), NULL);
  CreateDirectory(_P("Q:\\plugins\\music"), NULL);
  CreateDirectory(_P("Q:\\plugins\\video"), NULL);
  CreateDirectory(_P("Q:\\plugins\\pictures"), NULL);
  CreateDirectory(_P("Q:\\language"), NULL);
  CreateDirectory(_P("Q:\\visualisations"), NULL);
  CreateDirectory(_P("Q:\\sounds"), NULL);
  CreateDirectory(_P(g_settings.GetUserDataFolder()+"\\visualisations"),NULL);

  // initialize network
  if (!m_bXboxMediacenterLoaded)
  {
    CLog::Log(LOGINFO, "using default network settings");
    g_guiSettings.SetString("network.ipaddress", "192.168.0.100");
    g_guiSettings.SetString("network.subnet", "255.255.255.0");
    g_guiSettings.SetString("network.gateway", "192.168.0.1");
    g_guiSettings.SetString("network.dns", "192.168.0.1");
    g_guiSettings.SetBool("servers.ftpserver", true);
    g_guiSettings.SetBool("servers.webserver", false);
    g_guiSettings.SetBool("locale.timeserver", false);
  }

#ifdef __APPLE__
  // Set proxy server from system configuration
  g_guiSettings.SetString("network.httpproxyserver", Cocoa_Proxy_Host("http"));
  g_guiSettings.SetString("network.httpproxyport", Cocoa_Proxy_Port("http"));
  g_guiSettings.SetBool("network.usehttpproxy", Cocoa_Proxy_Enabled("http"));
  if (g_guiSettings.GetBool("network.usehttpproxy"))
    CLog::Log(LOGDEBUG, "Proxy is enabled - %s:%s", g_guiSettings.GetString("network.httpproxyserver").c_str(), g_guiSettings.GetString("network.httpproxyport").c_str());
  else
    CLog::Log(LOGDEBUG, "Proxy is disabled");
#endif
  
  StartServices();

  m_gWindowManager.Add(new CGUIWindowHome);                     // window id = 0

  CLog::Log(LOGNOTICE, "load default skin:[%s]", g_guiSettings.GetString("lookandfeel.skin").c_str());
  LoadSkin(g_guiSettings.GetString("lookandfeel.skin"));

  m_gWindowManager.Add(new CGUIWindowPrograms);                 // window id = 1
  m_gWindowManager.Add(new CGUIWindowPictures);                 // window id = 2
#if 0  
  m_gWindowManager.Add(new CGUIWindowFileManager);      // window id = 3
#endif
  m_gWindowManager.Add(new CGUIWindowVideoFiles);          // window id = 6
  m_gWindowManager.Add(new CGUIWindowSettings);                 // window id = 4
  //m_gWindowManager.Add(new CGUIWindowSystemInfo);               // window id = 7
  m_gWindowManager.Add(new CGUIWindowTestPattern);      // window id = 8
  m_gWindowManager.Add(new CGUIWindowSettingsScreenCalibration); // window id = 11
  m_gWindowManager.Add(new CGUIWindowSettingsCategory);         // window id = 12 slideshow:window id 2007
#if 0
  m_gWindowManager.Add(new CGUIWindowScripts);                  // window id = 20
#endif
  m_gWindowManager.Add(new CGUIWindowVideoNav);                 // window id = 36
  m_gWindowManager.Add(new CGUIWindowVideoPlaylist);            // window id = 28
#if 0
  m_gWindowManager.Add(new CGUIWindowLoginScreen);            // window id = 29
  m_gWindowManager.Add(new CGUIWindowSettingsProfile);          // window id = 34
  m_gWindowManager.Add(new CGUIWindowGameSaves);               // window id = 35
#endif
  m_gWindowManager.Add(new CGUIWindowNowPlaying);         // window id = 50
  m_gWindowManager.Add(new CGUIWindowPlexSearch);         // window id = 51
  m_gWindowManager.Add(new CGUIDialogYesNo);              // window id = 100
  m_gWindowManager.Add(new CGUIDialogProgress);           // window id = 101
#ifdef HAS_KAI
  m_gWindowManager.Add(new CGUIDialogInvite);             // window id = 102
#endif
  m_gWindowManager.Add(new CGUIDialogKeyboard);           // window id = 103
  m_gWindowManager.Add(&m_guiDialogVolumeBar);          // window id = 104
  m_gWindowManager.Add(&m_guiDialogSeekBar);            // window id = 115
  m_gWindowManager.Add(new CGUIDialogSubMenu);            // window id = 105
  m_gWindowManager.Add(new CGUIDialogContextMenu);        // window id = 106
  m_gWindowManager.Add(&m_guiDialogKaiToast);           // window id = 107
#ifdef HAS_KAI
  m_gWindowManager.Add(new CGUIDialogHost);               // window id = 108
#endif
  m_gWindowManager.Add(new CGUIDialogNumeric);            // window id = 109
  m_gWindowManager.Add(new CGUIDialogGamepad);            // window id = 110
  //m_gWindowManager.Add(new CGUIDialogButtonMenu);         // window id = 111
  m_gWindowManager.Add(new CGUIDialogPlayerControls);     // window id = 113
  m_gWindowManager.Add(new CGUIDialogMusicOSD);           // window id = 120
  m_gWindowManager.Add(new CGUIDialogVisualisationSettings);     // window id = 121
  m_gWindowManager.Add(new CGUIDialogVisualisationPresetList);   // window id = 122
  m_gWindowManager.Add(new CGUIDialogVideoSettings);             // window id = 123
  m_gWindowManager.Add(new CGUIDialogAudioSubtitleSettings);     // window id = 124
  m_gWindowManager.Add(new CGUIDialogVideoBookmarks);      // window id = 125
  // Don't add the filebrowser dialog - it's created and added when it's needed
#ifdef HAS_TRAINER
  m_gWindowManager.Add(new CGUIDialogTrainerSettings);  // window id = 127
#endif
  m_gWindowManager.Add(new CGUIDialogNetworkSetup);  // window id = 128
  m_gWindowManager.Add(new CGUIDialogMediaSource);   // window id = 129
  m_gWindowManager.Add(new CGUIDialogProfileSettings); // window id = 130
  m_gWindowManager.Add(new CGUIDialogFavourites);     // window id = 134
  m_gWindowManager.Add(new CGUIDialogSongInfo);       // window id = 135
  m_gWindowManager.Add(new CGUIDialogSmartPlaylistEditor);       // window id = 136
  m_gWindowManager.Add(new CGUIDialogSmartPlaylistRule);       // window id = 137
  m_gWindowManager.Add(new CGUIDialogBusy);      // window id = 138
  m_gWindowManager.Add(new CGUIDialogPictureInfo);      // window id = 139
  m_gWindowManager.Add(new CGUIDialogPluginSettings);      // window id = 140
#ifdef HAS_LINUX_NETWORK
  m_gWindowManager.Add(new CGUIDialogAccessPoints);      // window id = 141
#endif

  m_gWindowManager.Add(new CGUIDialogLockSettings); // window id = 131
  m_gWindowManager.Add(new CGUIDialogContentSettings);        // window id = 132
  m_gWindowManager.Add(new CGUIDialogRating);                 // window id = 200
  m_gWindowManager.Add(new CGUIDialogTimer);                 // window id = 201

  m_gWindowManager.Add(new CGUIWindowMusicPlayList);          // window id = 500
  m_gWindowManager.Add(new CGUIWindowMusicSongs);             // window id = 501
  m_gWindowManager.Add(new CGUIWindowMusicNav);               // window id = 502
  m_gWindowManager.Add(new CGUIWindowMusicPlaylistEditor);    // window id = 503

  m_gWindowManager.Add(new CGUIDialogSelect);             // window id = 2000
  m_gWindowManager.Add(new CGUIWindowMusicInfo);                // window id = 2001
  m_gWindowManager.Add(new CGUIDialogOK);                 // window id = 2002
  m_gWindowManager.Add(new CGUIWindowVideoInfo);                // window id = 2003
  m_gWindowManager.Add(new CGUIWindowScriptsInfo);              // window id = 2004
  m_gWindowManager.Add(new CGUIWindowFullScreen);         // window id = 2005
  m_gWindowManager.Add(new CGUIWindowVisualisation);      // window id = 2006
  m_gWindowManager.Add(new CGUIWindowSlideShow);          // window id = 2007
  m_gWindowManager.Add(new CGUIDialogFileStacking);       // window id = 2008

  m_gWindowManager.Add(new CGUIWindowOSD);                // window id = 2901
  m_gWindowManager.Add(new CGUIWindowMusicOverlay);       // window id = 2903
  m_gWindowManager.Add(new CGUIWindowVideoOverlay);       // window id = 2904
  m_gWindowManager.Add(new CGUIWindowScreensaver);        // window id = 2900 Screensaver
  m_gWindowManager.Add(new CGUIWindowWeather);            // window id = 2600 WEATHER
#ifdef HAS_KAI
  m_gWindowManager.Add(new CGUIWindowBuddies);            // window id = 2700 BUDDIES
#endif
  m_gWindowManager.Add(new CGUIWindowStartup);            // startup window (id 2999)

  /* window id's 3000 - 3100 are reserved for python */
#ifdef HAS_KAI
  g_DownloadManager.Initialize();
#endif

  m_ctrDpad.SetDelays(100, 500); //g_stSettings.m_iMoveDelayController, g_stSettings.m_iRepeatDelayController);

  SAFE_DELETE(m_splash);

  if (g_guiSettings.GetBool("masterlock.startuplock") &&
      g_settings.m_vecProfiles[0].getLockMode() != LOCK_MODE_EVERYONE &&
     !g_settings.m_vecProfiles[0].getLockCode().IsEmpty())
  {
     g_passwordManager.CheckStartUpLock();
  }

  // check if we should use the login screen
  if (g_settings.bUseLoginScreen)
  {
    m_gWindowManager.ActivateWindow(WINDOW_LOGIN_SCREEN);
  }
  else
  {
    RESOLUTION res = INVALID;
    CStdString startupPath = g_SkinInfo.GetSkinPath("startup.xml", &res);
    int startWindow = g_guiSettings.GetInt("lookandfeel.startupwindow");
    // test for a startup window, and activate that instead of home
    if (CFile::Exists(startupPath) && (!g_SkinInfo.OnlyAnimateToHome() || startWindow == WINDOW_HOME))
    {
      m_gWindowManager.ActivateWindow(WINDOW_STARTUP);
    }
    else
    {
      // We need to Popup the WindowHome to initiate the GUIWindowManger for MasterCode popup dialog!
      // Then we can start the StartUpWindow! To prevent BlackScreen if the target Window is Protected with MasterCode!
      m_gWindowManager.ActivateWindow(WINDOW_HOME);
      if (startWindow != WINDOW_HOME)
        m_gWindowManager.ActivateWindow(startWindow);
    }
  }

#ifdef HAS_XBOX_NETWORK
  /* setup network based on our settings */
  /* network will start it's init procedure */
  m_network.Initialize(g_guiSettings.GetInt("network.assignment"),
    g_guiSettings.GetString("network.ipaddress").c_str(),
    g_guiSettings.GetString("network.subnet").c_str(),
    g_guiSettings.GetString("network.gateway").c_str(),
    g_guiSettings.GetString("network.dns").c_str());
#endif

#ifdef HAS_PYTHON
  g_pythonParser.bStartup = true;
#endif
  g_sysinfo.Refresh();

  CLog::Log(LOGINFO, "removing tempfiles");
  CUtil::RemoveTempFiles();

  if (!m_bAllSettingsLoaded)
  {
    CLog::Log(LOGWARNING, "settings not correct, show dialog");
    CStdString test;
    CUtil::GetHomePath(test);
    CGUIDialogOK *dialog = (CGUIDialogOK *)m_gWindowManager.GetWindow(WINDOW_DIALOG_OK);
    if (dialog)
    {
      dialog->SetHeading(279);
      dialog->SetLine(0, "Error while loading settings");
      dialog->SetLine(1, test);
      dialog->SetLine(2, "");;
      dialog->DoModal();
    }
  }

  //  Show mute symbol
  if (g_stSettings.m_nVolumeLevel == VOLUME_MINIMUM)
    Mute();

  // if the user shutoff the xbox during music scan
  // restore the settings
  if (g_stSettings.m_bMyMusicIsScanning)
  {
    CLog::Log(LOGWARNING,"System rebooted during music scan! ... restoring UseTags and FindRemoteThumbs");
    RestoreMusicScanSettings();
  }

#ifdef HAS_HAL
  g_HalManager.Initialize();
#endif

  m_slowTimer.StartZero();
  
#ifdef __APPLE__
  // Register for low battery warnings (ignored on unsupported models)
  Cocoa_HW_SetBatteryWarningEnabled(true);
#endif

  CLog::Log(LOGNOTICE, "initialize done");

  m_bInitializing = false;

  // final check for debugging combo
  CheckForDebugButtonCombo();
  
  // On first run, ask if the user wants to enable auto update checks
  if (g_guiSettings.GetBool("softwareupdate.firstrun"))
  {
    g_guiSettings.SetBool("softwareupdate.firstrun", false);
    bool userAlertsDisabled = CGUIDialogYesNo::ShowAndGetInput(40022, 40023, 40024, -1, 107, 106);
    g_guiSettings.SetBool("softwareupdate.alertsenabled", userAlertsDisabled ? false : true);
  }
  
  // Check for updates & alert the user if a new version is available
  if (g_guiSettings.GetBool("softwareupdate.alertsenabled"))
  {
    Cocoa_SetUpdateAlertType(g_guiSettings.GetInt("softwareupdate.alerttype"));
    Cocoa_CheckForUpdatesInBackground();
    double interval = 3600;
    switch (g_guiSettings.GetInt("softwareupdate.checkinterval"))
    {
      case UPDATE_INTERVAL_DAILY: interval = 86400; break;
      case UPDATE_INTERVAL_WEEKLY: interval = 604800; break;
    }
    Cocoa_SetUpdateCheckInterval(interval);
  }
  
  return S_OK;
}

void CApplication::PrintXBEToLCD(const char* xbePath)
{
#ifdef HAS_LCD
#ifdef _XBOX
  CStdString strXBEName;
  if (!CUtil::GetXBEDescription(xbePath, strXBEName))
  {
    CUtil::GetDirectoryName(xbePath, strXBEName);
    CUtil::ShortenFileName(strXBEName);
    CUtil::RemoveIllegalChars(strXBEName);
  }
  // crop to LCD screen size
  if ((int)strXBEName.size() > g_advancedSettings.m_lcdColumns)
    strXBEName = strXBEName.Left(g_advancedSettings.m_lcdColumns);
  if (g_lcd)
  {
    g_infoManager.SetLaunchingXBEName(strXBEName);
    g_lcd->Render(ILCD::LCD_MODE_XBE_LAUNCH);
  }
#endif
#endif
}

void CApplication::StartIdleThread()
{
  m_idleThread.Create(false, 0x100);
}

void CApplication::StopIdleThread()
{
  m_idleThread.StopThread();
}

void CApplication::StartWebServer()
{
#ifdef HAS_WEB_SERVER
  if (g_guiSettings.GetBool("servers.webserver") && m_network.IsAvailable())
  {
    CLog::Log(LOGNOTICE, "Webserver: Starting...");
#ifdef _LINUX
    if (atoi(g_guiSettings.GetString("servers.webserverport")) < 1024 && geteuid() != 0)
    {
        CLog::Log(LOGERROR, "Cannot start Web Server as port is smaller than 1024 and user is not root");
        return;
    }
#endif
    CSectionLoader::Load("LIBHTTP");
#ifdef HAS_LINUX_NETWORK
    if (m_network.GetFirstConnectedInterface())
    {
       m_pWebServer = new CWebServer();
       m_pWebServer->Start(m_network.GetFirstConnectedInterface()->GetCurrentIPAddress().c_str(), atoi(g_guiSettings.GetString("servers.webserverport")), _P("Q:\\web"), false);
    }
#else
    m_pWebServer = new CWebServer();
    m_pWebServer->Start(m_network.m_networkinfo.ip, atoi(g_guiSettings.GetString("servers.webserverport")), _P("Q:\\web"), false);
#endif
    if (m_pWebServer && m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=1)
      getApplicationMessenger().HttpApi("broadcastlevel; StartUp;1");
  }
#endif
}

void CApplication::StopWebServer()
{
#ifdef HAS_WEB_SERVER
  if (m_pWebServer)
  {
    CLog::Log(LOGNOTICE, "Webserver: Stopping...");
    m_pWebServer->Stop();
    delete m_pWebServer;
    m_pWebServer = NULL;
    CSectionLoader::Unload("LIBHTTP");
    CLog::Log(LOGNOTICE, "Webserver: Stopped...");
  }
#endif
}

void CApplication::StartFtpServer()
{
#ifdef HAS_FTP_SERVER
  if ( g_guiSettings.GetBool("servers.ftpserver") && m_network.IsAvailable() )
  {
    CLog::Log(LOGNOTICE, "XBFileZilla: Starting...");
    if (!m_pFileZilla)
    {
      CStdString xmlpath = _P("Q:\\System\\");
      // if user didn't upgrade properly,
      // check whether P:\\FileZilla Server.xml exists (UserData/FileZilla Server.xml)
      if (CFile::Exists(g_settings.GetUserDataItem("FileZilla Server.xml")))
        xmlpath = g_settings.GetUserDataFolder();

      // check file size and presence
      CFile xml;
      if (xml.Open(xmlpath+"FileZilla Server.xml",true) && xml.GetLength() > 0)
      {
        m_pFileZilla = new CXBFileZilla(xmlpath);
        m_pFileZilla->Start(false);
      }
      else
      {
        // 'FileZilla Server.xml' does not exist or is corrupt,
        // falling back to ftp emergency recovery mode
        CLog::Log(LOGNOTICE, "XBFileZilla: 'FileZilla Server.xml' is missing or is corrupt!");
        CLog::Log(LOGNOTICE, "XBFileZilla: Starting ftp emergency recovery mode");
        StartFtpEmergencyRecoveryMode();
      }
      xml.Close();
    }
  }
#endif
}

void CApplication::StopFtpServer()
{
#ifdef HAS_FTP_SERVER
  if (m_pFileZilla)
  {
    CLog::Log(LOGINFO, "XBFileZilla: Stopping...");

    std::vector<SXFConnection> mConnections;
    std::vector<SXFConnection>::iterator it;

    m_pFileZilla->GetAllConnections(mConnections);

    for(it = mConnections.begin();it != mConnections.end();it++)
    {
      m_pFileZilla->CloseConnection(it->mId);
    }

    m_pFileZilla->Stop();
    delete m_pFileZilla;
    m_pFileZilla = NULL;

    CLog::Log(LOGINFO, "XBFileZilla: Stopped");
  }
#endif
}

void CApplication::StartTimeServer()
{
#ifdef HAS_TIME_SERVER
  if (g_guiSettings.GetBool("locale.timeserver") && m_network.IsAvailable() )
  {
    if( !m_psntpClient )
    {
      CSectionLoader::Load("SNTP");
      CLog::Log(LOGNOTICE, "start timeserver client");

      m_psntpClient = new CSNTPClient();
      m_psntpClient->Update();
    }
  }
#endif
}

void CApplication::StopTimeServer()
{
#ifdef HAS_TIME_SERVER
  if( m_psntpClient )
  {
    CLog::Log(LOGNOTICE, "stop time server client");
    SAFE_DELETE(m_psntpClient);
    CSectionLoader::Unload("SNTP");
  }
#endif
}

#ifdef HAS_KAI
void CApplication::StartKai()
{
  if (g_guiSettings.GetBool("xlinkkai.enabled"))
  {
    CGUIWindowBuddies *pKai = (CGUIWindowBuddies*)m_gWindowManager.GetWindow(WINDOW_BUDDIES);
    if (pKai)
    {
      CLog::Log(LOGNOTICE, "starting kai");
      CKaiClient::GetInstance()->SetObserver(pKai);
    }
  }
}

void CApplication::StopKai()
{
  if (CKaiClient::IsInstantiated())
  {
    CLog::Log(LOGNOTICE, "stopping kai");
    CKaiClient::GetInstance()->RemoveObserver();
    CKaiClient::RemoveInstance();
  }
}
#endif

void CApplication::StartUPnP()
{
#ifdef HAS_UPNP
    StartUPnPClient();
    StartUPnPServer();
    StartUPnPRenderer();
#endif
}

void CApplication::StopUPnP()
{
#ifdef HAS_UPNP
  if (CUPnP::IsInstantiated())
  {
    CLog::Log(LOGNOTICE, "stopping upnp");
    CUPnP::ReleaseInstance();
  }
#endif
}

void CApplication::StartEventServer()
{
#ifdef HAS_EVENT_SERVER
  if (g_guiSettings.GetBool("remoteevents.enabled"))
  {
    CLog::Log(LOGNOTICE, "ES: Starting event server");
    CEventServer::GetInstance()->StartServer();
  }
#endif
}

void CApplication::StopEventServer()
{
#ifdef HAS_EVENT_SERVER
  CLog::Log(LOGNOTICE, "ES: Stopping event server");
  CEventServer::GetInstance()->StopServer();
#endif
}

void CApplication::RefreshEventServer()
{
#ifdef HAS_EVENT_SERVER
  if (g_guiSettings.GetBool("remoteevents.enabled"))
  {
    CEventServer::GetInstance()->RefreshSettings();
  }
#endif
}

void CApplication::StartUPnPRenderer()
{
#ifdef HAS_UPNP
  if (g_guiSettings.GetBool("upnp.renderer"))
  {
    CLog::Log(LOGNOTICE, "starting upnp renderer");
    CUPnP::GetInstance()->StartRenderer();
  }
#endif
}

void CApplication::StopUPnPRenderer()
{
#ifdef HAS_UPNP
  if (CUPnP::IsInstantiated())
  {
    CLog::Log(LOGNOTICE, "stopping upnp renderer");
    CUPnP::GetInstance()->StopRenderer();
  }
#endif
}

void CApplication::StartUPnPClient()
{
#ifdef HAS_UPNP
  if (g_guiSettings.GetBool("upnp.client"))
  {
    CLog::Log(LOGNOTICE, "starting upnp client");
    CUPnP::GetInstance()->StartClient();
  }
#endif
}

void CApplication::StopUPnPClient()
{
#ifdef HAS_UPNP
  if (CUPnP::IsInstantiated())
  {
    CLog::Log(LOGNOTICE, "stopping upnp client");
    CUPnP::GetInstance()->StopClient();
  }
#endif
}

void CApplication::StartUPnPServer()
{
#ifdef HAS_UPNP
  if (g_guiSettings.GetBool("servers.upnpserver"))
  {
    CLog::Log(LOGNOTICE, "starting upnp server");
    CUPnP::GetInstance()->StartServer();
  }
#endif
}

void CApplication::StopUPnPServer()
{
#ifdef HAS_UPNP
  if (CUPnP::IsInstantiated())
  {
    CLog::Log(LOGNOTICE, "stopping upnp server");
    CUPnP::GetInstance()->StopServer();
  }
#endif
}

void CApplication::StartLEDControl(bool switchoff)
{
#ifdef HAS_XBOX_HARDWARE
  if (switchoff && g_guiSettings.GetInt("system.ledcolour") != LED_COLOUR_NO_CHANGE)
  {
    if ( IsPlayingVideo() && (g_guiSettings.GetInt("system.leddisableonplayback") == LED_PLAYBACK_VIDEO))
      ILED::CLEDControl(LED_COLOUR_OFF);
    if ( IsPlayingAudio() && (g_guiSettings.GetInt("system.leddisableonplayback") == LED_PLAYBACK_MUSIC))
      ILED::CLEDControl(LED_COLOUR_OFF);
    if ( ((IsPlayingVideo() || IsPlayingAudio())) && (g_guiSettings.GetInt("system.leddisableonplayback") == LED_PLAYBACK_VIDEO_MUSIC))
      ILED::CLEDControl(LED_COLOUR_OFF);
  }
  else if (!switchoff)
    ILED::CLEDControl(g_guiSettings.GetInt("system.ledcolour"));
#endif
}

void CApplication::DimLCDOnPlayback(bool dim)
{
#ifdef HAS_LCD
  if(g_lcd && dim && (g_guiSettings.GetInt("lcd.disableonplayback") != LED_PLAYBACK_OFF) && (g_guiSettings.GetInt("lcd.type") != LCD_TYPE_NONE))
  {
    if ( (IsPlayingVideo()) && g_guiSettings.GetInt("lcd.disableonplayback") == LED_PLAYBACK_VIDEO)
      g_lcd->SetBackLight(0);
    if ( (IsPlayingAudio()) && g_guiSettings.GetInt("lcd.disableonplayback") == LED_PLAYBACK_MUSIC)
      g_lcd->SetBackLight(0);
    if ( ((IsPlayingVideo() || IsPlayingAudio())) && g_guiSettings.GetInt("lcd.disableonplayback") == LED_PLAYBACK_VIDEO_MUSIC)
      g_lcd->SetBackLight(0);
  }
  else if(!dim)
    g_lcd->SetBackLight(g_guiSettings.GetInt("lcd.backlight"));
#endif
}

void CApplication::StartServices()
{
#ifdef HAS_XBOX_HARDWARE
  StartIdleThread();
#endif

  CheckDate();
  StartLEDControl(false);

  // Start Thread for DVD Mediatype detection
  CLog::Log(LOGNOTICE, "start dvd mediatype detection");
  m_DetectDVDType.Create(false, THREAD_MINSTACKSIZE);

  CLog::Log(LOGNOTICE, "initializing playlistplayer");
  g_playlistPlayer.SetRepeat(PLAYLIST_MUSIC, g_stSettings.m_bMyMusicPlaylistRepeat ? PLAYLIST::REPEAT_ALL : PLAYLIST::REPEAT_NONE);
  g_playlistPlayer.SetShuffle(PLAYLIST_MUSIC, g_stSettings.m_bMyMusicPlaylistShuffle);
  g_playlistPlayer.SetRepeat(PLAYLIST_VIDEO, g_stSettings.m_bMyVideoPlaylistRepeat ? PLAYLIST::REPEAT_ALL : PLAYLIST::REPEAT_NONE);
  g_playlistPlayer.SetShuffle(PLAYLIST_VIDEO, g_stSettings.m_bMyVideoPlaylistShuffle);
  CLog::Log(LOGNOTICE, "DONE initializing playlistplayer");

#ifdef HAS_LCD
  CLCDFactory factory;
  g_lcd = factory.Create();
  if (g_lcd)
  {
    g_lcd->Initialize();
  }
#endif

#ifdef HAS_XBOX_HARDWARE
  if (g_guiSettings.GetBool("system.autotemperature"))
  {
    CLog::Log(LOGNOTICE, "start fancontroller");
    CFanController::Instance()->Start(g_guiSettings.GetInt("system.targettemperature"), g_guiSettings.GetInt("system.minfanspeed"));
  }
  else if (g_guiSettings.GetBool("system.fanspeedcontrol"))
  {
    CLog::Log(LOGNOTICE, "setting fanspeed");
    CFanController::Instance()->SetFanSpeed(g_guiSettings.GetInt("system.fanspeed"));
  }
  int setting_level = g_guiSettings.GetInt("harddisk.aamlevel");
  if (setting_level == AAM_QUIET)
    XKHDD::SetAAMLevel(0x80);
  else if (setting_level == AAM_FAST)
    XKHDD::SetAAMLevel(0xFE);
  setting_level = g_guiSettings.GetInt("harddisk.apmlevel");
  switch(setting_level)
  {
  case APM_LOPOWER:
    XKHDD::SetAPMLevel(0x80);
    break;
  case APM_HIPOWER:
    XKHDD::SetAPMLevel(0xFE);
    break;
  case APM_LOPOWER_STANDBY:
    XKHDD::SetAPMLevel(0x01);
    break;
  case APM_HIPOWER_STANDBY:
    XKHDD::SetAPMLevel(0x7F);
    break;
  }
#endif
}

void CApplication::CheckDate()
{
  CLog::Log(LOGNOTICE, "Checking the Date!");
  // Check the Date: Year, if it is  above 2099 set to 2004!
  SYSTEMTIME CurTime;
  SYSTEMTIME NewTime;
  GetLocalTime(&CurTime);
  GetLocalTime(&NewTime);
  CLog::Log(LOGINFO, "- Current Date is: %i-%i-%i",CurTime.wDay, CurTime.wMonth, CurTime.wYear);
  if ((CurTime.wYear > 2099) || (CurTime.wYear < 2001) )        // XBOX MS Dashboard also uses min/max DateYear 2001/2099 !!
  {
    CLog::Log(LOGNOTICE, "- The Date is Wrong: Setting New Date!");
    NewTime.wYear       = 2004; // 2004
    NewTime.wMonth      = 1;    // January
    NewTime.wDayOfWeek  = 1;    // Monday
    NewTime.wDay        = 5;    // Monday 05.01.2004!!
    NewTime.wHour       = 12;
    NewTime.wMinute     = 0;

    FILETIME stNewTime, stCurTime;
    SystemTimeToFileTime(&NewTime, &stNewTime);
    SystemTimeToFileTime(&CurTime, &stCurTime);
#ifdef HAS_XBOX_HARDWARE
    NtSetSystemTime(&stNewTime, &stCurTime);    // Set a Default Year 2004!
#endif
    CLog::Log(LOGNOTICE, "- New Date is now: %i-%i-%i",NewTime.wDay, NewTime.wMonth, NewTime.wYear);
  }
  return ;
}

void CApplication::StopServices()
{
  m_network.NetworkMessage(CNetwork::SERVICES_DOWN, 0);

  CLog::Log(LOGNOTICE, "stop dvd detect media");
  m_DetectDVDType.StopThread();

#ifdef HAS_XBOX_HARDWARE
  CLog::Log(LOGNOTICE, "stop fancontroller");
  CFanController::Instance()->Stop();
  CFanController::RemoveInstance();
  StopIdleThread();
#endif
}

void CApplication::DelayLoadSkin()
{
  m_dwSkinTime = timeGetTime() + 2000;
  return ;
}

void CApplication::CancelDelayLoadSkin()
{
  m_dwSkinTime = 0;
}

void CApplication::ReloadSkin()
{
  CGUIMessage msg(GUI_MSG_LOAD_SKIN, (DWORD) -1, m_gWindowManager.GetActiveWindow());
  g_graphicsContext.SendMessage(msg);
  // Reload the skin, restoring the previously focused control
  CGUIWindow* pWindow = m_gWindowManager.GetWindow(m_gWindowManager.GetActiveWindow());
  unsigned iCtrlID = pWindow->GetFocusedControlID();
  g_application.LoadSkin(g_guiSettings.GetString("lookandfeel.skin"));
  pWindow = m_gWindowManager.GetWindow(m_gWindowManager.GetActiveWindow());
  if (pWindow)
  {
    CGUIMessage msg3(GUI_MSG_SETFOCUS, m_gWindowManager.GetActiveWindow(), iCtrlID, 0);
    pWindow->OnMessage(msg3);
  }
  
  // Make sure dialogs shown from Cocoa are redisplayed
  Cocoa_UpdateProgressDialog();
}

void CApplication::LoadSkin(const CStdString& strSkin)
{
  bool bPreviousPlayingState=false;
  bool bPreviousRenderingState=false;
  if (g_application.m_pPlayer && g_application.IsPlayingVideo())
  {
    bPreviousPlayingState = !g_application.m_pPlayer->IsPaused();
    if (bPreviousPlayingState)
      g_application.m_pPlayer->Pause();
#ifdef HAS_VIDEO_PLAYBACK
    if (!g_renderManager.Paused())
    {
      if (m_gWindowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
     {
        m_gWindowManager.ActivateWindow(WINDOW_HOME);
        bPreviousRenderingState = true;
      }
    }
#endif
  }
  //stop the busy renderer if it's running before we lock the graphiccontext or we could deadlock.
  g_ApplicationRenderer.Stop();
  // close the music and video overlays (they're re-opened automatically later)
  CSingleLock lock(g_graphicsContext);

  m_dwSkinTime = 0;

  CStdString strHomePath;
  CStdString strSkinPath = g_settings.GetSkinFolder(strSkin);

  CLog::Log(LOGINFO, "  load skin from:%s", strSkinPath.c_str());

  // save the current window details
  int currentWindow = m_gWindowManager.GetActiveWindow();
  CStdString currentDirectory;
  CStdString startDirectory;
  CDirectoryHistory directoryHistory;
  
  if (currentWindow != WINDOW_INVALID)
  {
    // Remember the current place and where the "start" is (for when
    // we used returns).
    //
    CGUIWindow* win = m_gWindowManager.GetWindow(currentWindow);
    if (win->IsMediaWindow())
    {
      CGUIMediaWindow* mediaWin = (CGUIMediaWindow* )win;
      currentDirectory = mediaWin->CurrentDirectory().m_strPath;
      startDirectory = mediaWin->StartDirectory();
      directoryHistory = mediaWin->DirectoryHistory();
    }
  }
  
  vector<DWORD> currentModelessWindows;
  m_gWindowManager.GetActiveModelessWindows(currentModelessWindows);

#ifdef HAS_KAI
  //  When the app is started the instance of the
  //  kai client should not be created until the
  //  skin is loaded the first time, but we must
  //  disconnect from the engine when the skin is
  //  changed
  bool bKaiConnected = false;
  if (!m_bInitializing && g_guiSettings.GetBool("xlinkkai.enabled"))
  {
    bKaiConnected = CKaiClient::GetInstance()->IsEngineConnected();
    if (bKaiConnected)
    {
      CLog::Log(LOGINFO, " Disconnecting Kai...");
      CKaiClient::GetInstance()->RemoveObserver();
    }
  }
#endif

  CLog::Log(LOGINFO, "  delete old skin...");
  UnloadSkin();

  // Load in the skin.xml file if it exists
  g_SkinInfo.Load(strSkinPath);

  CLog::Log(LOGINFO, "  load fonts for skin...");
  g_graphicsContext.SetMediaDir(strSkinPath);
  if (g_langInfo.ForceUnicodeFont() && !g_fontManager.IsFontSetUnicode(g_guiSettings.GetString("lookandfeel.font")))
  {
    CLog::Log(LOGINFO, "    language needs a ttf font, loading first ttf font available");
    CStdString strFontSet;
    if (g_fontManager.GetFirstFontSetUnicode(strFontSet))
    {
      CLog::Log(LOGINFO, "    new font is '%s'", strFontSet.c_str());
      g_guiSettings.SetString("lookandfeel.font", strFontSet);
      g_guiSettings.SetBool("lookandfeel.lastLoadRequiredUnicode", true);
      g_settings.Save();
    }
    else
      CLog::Log(LOGERROR, "    no ttf font found, but needed for the language %s.", g_settings.GetLanguage().c_str());
  }
  
  if (g_langInfo.ForceUnicodeFont())
  {
    // Remember that we forced Unicode.
    g_guiSettings.SetBool("lookandfeel.lastLoadRequiredUnicode", true);
  }
  else if (g_langInfo.ForceUnicodeFont() == false && g_guiSettings.GetBool("lookandfeel.lastLoadRequiredUnicode") == true)
  {
    // Reset to default.
    g_guiSettings.SetString("lookandfeel.font", "Default");
    g_guiSettings.SetBool("lookandfeel.lastLoadRequiredUnicode", false);
  }
  
  g_colorManager.Load(g_guiSettings.GetString("lookandfeel.skincolors"));

  g_fontManager.LoadFonts(g_guiSettings.GetString("lookandfeel.font"));

  // load in the skin strings
  CStdString skinPath, skinEnglishPath;
  CUtil::AddFileToFolder(strSkinPath, "language", skinPath);
  CUtil::AddFileToFolder(skinPath, g_settings.GetLanguage(), skinPath);
  CUtil::AddFileToFolder(skinPath, "strings.xml", skinPath);

  CUtil::AddFileToFolder(strSkinPath, "language", skinEnglishPath);
  CUtil::AddFileToFolder(skinEnglishPath, "English", skinEnglishPath);
  CUtil::AddFileToFolder(skinEnglishPath, "strings.xml", skinEnglishPath);

  g_localizeStrings.LoadSkinStrings(skinPath, skinEnglishPath);

  LARGE_INTEGER start;
  QueryPerformanceCounter(&start);

  CLog::Log(LOGINFO, "  load new skin...");
  CGUIWindowHome *pHome = (CGUIWindowHome *)m_gWindowManager.GetWindow(WINDOW_HOME);
  if (!g_SkinInfo.Check(strSkinPath) || !pHome || !pHome->Load("Home.xml"))
  {
    // failed to load home.xml
    // fallback to default skin
    if ( strcmpi(strSkin.c_str(), "MediaStream") != 0)
    {
      CLog::Log(LOGERROR, "failed to load home.xml for skin:%s, fallback to \"MediaStream\" skin", strSkin.c_str());
      LoadSkin("MediaStream");
      return ;
    }
  }

  // Load the user windows
  LoadUserWindows(strSkinPath);

  LARGE_INTEGER end, freq;
  QueryPerformanceCounter(&end);
  QueryPerformanceFrequency(&freq);
  CLog::Log(LOGDEBUG,"Load Skin XML: %.2fms", 1000.f * (end.QuadPart - start.QuadPart) / freq.QuadPart);

  CLog::Log(LOGINFO, "  initialize new skin...");
  m_guiPointer.AllocResources(true);
  m_guiDialogVolumeBar.AllocResources(true);
  m_guiDialogSeekBar.AllocResources(true);
  m_guiDialogKaiToast.AllocResources(true);
  m_guiDialogMuteBug.AllocResources(true);
  m_gWindowManager.AddMsgTarget(this);
  m_gWindowManager.AddMsgTarget(&g_playlistPlayer);
  m_gWindowManager.AddMsgTarget(&g_infoManager);
  m_gWindowManager.SetCallback(*this);
  m_gWindowManager.Initialize();
  g_audioManager.Initialize(CAudioContext::DEFAULT_DEVICE);
  g_audioManager.Load();

  CGUIDialogFullScreenInfo* pDialog = NULL;
  RESOLUTION res;
  CStdString strPath = g_SkinInfo.GetSkinPath("DialogFullScreenInfo.xml", &res);
  if (CFile::Exists(strPath))
    pDialog = new CGUIDialogFullScreenInfo;
    
  if (pDialog)
    m_gWindowManager.Add(pDialog); // window id = 142

  CLog::Log(LOGINFO, "  skin loaded...");

#ifdef HAS_KAI
  if (bKaiConnected)
  {
    CLog::Log(LOGINFO, " Reconnecting Kai...");

    CGUIWindowBuddies *pKai = (CGUIWindowBuddies *)m_gWindowManager.GetWindow(WINDOW_BUDDIES);
    CKaiClient::GetInstance()->SetObserver(pKai);
    Sleep(3000);  //  The client need some time to "resync"
  }
#endif

  // leave the graphics lock
  lock.Leave();
  g_ApplicationRenderer.Start();

  // restore windows
  if (currentWindow != WINDOW_INVALID)
  {
    m_gWindowManager.ActivateWindow(currentWindow, currentDirectory);
    if (startDirectory.size() > 0)
    {
      ((CGUIMediaWindow* )m_gWindowManager.GetWindow(currentWindow))->SetStartDirectory(startDirectory);
      ((CGUIMediaWindow* )m_gWindowManager.GetWindow(currentWindow))->SetDirectoryHistory(directoryHistory);
    }
    
    for (unsigned int i = 0; i < currentModelessWindows.size(); i++)
    {
      CGUIDialog *dialog = (CGUIDialog *)m_gWindowManager.GetWindow(currentModelessWindows[i]);
      if (dialog) dialog->Show();
    }
  }

  if (g_application.m_pPlayer && g_application.IsPlayingVideo())
  {
    if (bPreviousPlayingState)
      g_application.m_pPlayer->Pause();
    if (bPreviousRenderingState)
      m_gWindowManager.ActivateWindow(WINDOW_FULLSCREEN_VIDEO);
  }
}

void CApplication::UnloadSkin()
{
  g_ApplicationRenderer.Stop();
  g_audioManager.DeInitialize(CAudioContext::DEFAULT_DEVICE);

  m_gWindowManager.DeInitialize();

  //These windows are not handled by the windowmanager (why not?) so we should unload them manually
  CGUIMessage msg(GUI_MSG_WINDOW_DEINIT, 0, 0);
  m_guiPointer.OnMessage(msg);
  m_guiPointer.ResetControlStates();
  m_guiPointer.FreeResources(true);
  m_guiDialogMuteBug.OnMessage(msg);
  m_guiDialogMuteBug.ResetControlStates();
  m_guiDialogMuteBug.FreeResources(true);
  
  // remove the skin-dependent window
  m_gWindowManager.Delete(WINDOW_DIALOG_FULLSCREEN_INFO);

  CGUIWindow::FlushReferenceCache(); // flush the cache

  g_TextureManager.Cleanup();

  g_fontManager.Clear();

  g_colorManager.Clear();

  g_charsetConverter.reset();

  g_infoManager.Clear();
}

bool CApplication::LoadUserWindows(const CStdString& strSkinPath)
{
  WIN32_FIND_DATA FindFileData;
  WIN32_FIND_DATA NextFindFileData;
  HANDLE hFind;
  TiXmlDocument xmlDoc;
  RESOLUTION resToUse = INVALID;

  // Start from wherever home.xml is
  g_SkinInfo.GetSkinPath("Home.xml", &resToUse);
  std::vector<CStdString> vecSkinPath;
  if (resToUse == HDTV_1080i)
    vecSkinPath.push_back(strSkinPath+g_SkinInfo.GetDirFromRes(HDTV_1080i));
  if (resToUse == HDTV_720p)
    vecSkinPath.push_back(strSkinPath+g_SkinInfo.GetDirFromRes(HDTV_720p));
  if (resToUse == PAL_16x9 || resToUse == NTSC_16x9 || resToUse == HDTV_480p_16x9 || resToUse == HDTV_720p || resToUse == HDTV_1080i)
    vecSkinPath.push_back(strSkinPath+g_SkinInfo.GetDirFromRes(g_SkinInfo.GetDefaultWideResolution()));
  vecSkinPath.push_back(strSkinPath+g_SkinInfo.GetDirFromRes(g_SkinInfo.GetDefaultResolution()));
  for (unsigned int i=0;i<vecSkinPath.size();++i)
  {
    CStdString strPath;
#ifndef _LINUX
    strPath.Format("%s\\%s", vecSkinPath[i], "custom*.xml");
#else
    strPath.Format("%s/%s", vecSkinPath[i], "custom*.xml");
#endif
    CLog::Log(LOGINFO, "Loading user windows, path %s", vecSkinPath[i].c_str());
    hFind = FindFirstFile(strPath.c_str(), &NextFindFileData);

    CStdString strFileName;
    while (hFind != INVALID_HANDLE_VALUE)
    {
      FindFileData = NextFindFileData;

      if (!FindNextFile(hFind, &NextFindFileData))
      {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
      }

      // skip "up" directories, which come in all queries
#ifndef _LINUX
      if (!_tcscmp(FindFileData.cFileName, _T(".")) || !_tcscmp(FindFileData.cFileName, _T("..")))
        continue;
#else
      if (!strcmp(FindFileData.cFileName, ".") || !strcmp(FindFileData.cFileName, ".."))
        continue;
#endif

#ifndef _LINUX
      strFileName = vecSkinPath[i]+"\\"+FindFileData.cFileName;
#else
      strFileName = vecSkinPath[i]+"/"+FindFileData.cFileName;
#endif
      CLog::Log(LOGINFO, "Loading skin file: %s", strFileName.c_str());
      CStdString strLower(FindFileData.cFileName);
      strLower.MakeLower();
      strLower = vecSkinPath[i] + "/" + strLower;
      if (!xmlDoc.LoadFile(strFileName.c_str()) && !xmlDoc.LoadFile(strLower.c_str()))
      {
        CLog::Log(LOGERROR, "unable to load:%s, Line %d\n%s", strFileName.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
        continue;
      }

      // Root element should be <window>
      TiXmlElement* pRootElement = xmlDoc.RootElement();
      CStdString strValue = pRootElement->Value();
      if (!strValue.Equals("window"))
      {
        CLog::Log(LOGERROR, "file :%s doesnt contain <window>", strFileName.c_str());
        continue;
      }

      // Read the <type> element to get the window type to create
      // If no type is specified, create a CGUIWindow as default
      CGUIWindow* pWindow = NULL;
      CStdString strType;
      if (pRootElement->Attribute("type"))
        strType = pRootElement->Attribute("type");
      else
      {
        const TiXmlNode *pType = pRootElement->FirstChild("type");
        if (pType && pType->FirstChild())
          strType = pType->FirstChild()->Value();
      }
      if (strType.Equals("dialog"))
        pWindow = new CGUIDialog(0, "");
      else if (strType.Equals("submenu"))
        pWindow = new CGUIDialogSubMenu();
      //else if (strType.Equals("buttonmenu"))
      //  pWindow = new CGUIDialogButtonMenu();
      else
        pWindow = new CGUIStandardWindow();

      int id = WINDOW_INVALID;
      if (!pRootElement->Attribute("id", &id))
      {
        const TiXmlNode *pType = pRootElement->FirstChild("id");
        if (pType && pType->FirstChild())
          id = atol(pType->FirstChild()->Value());
      }
      // Check to make sure the pointer isn't still null
      if (pWindow == NULL || id == WINDOW_INVALID)
      {
        CLog::Log(LOGERROR, "Out of memory / Failed to create new object in LoadUserWindows");
        return false;
      }
      if (m_gWindowManager.GetWindow(WINDOW_HOME + id))
      {
        delete pWindow;
        continue;
      }
      // set the window's xml file, and add it to the window manager.
      pWindow->SetXMLFile(FindFileData.cFileName);
      pWindow->SetID(WINDOW_HOME + id);
      pWindow->SetCustom();
      m_gWindowManager.AddCustomWindow(pWindow);
    }
    CloseHandle(hFind);
  }
  return true;
}

#ifdef HAS_XBOX_D3D  // needed for screenshot
void CApplication::Render()
{
#else
void CApplication::RenderNoPresent()
{
#endif
  MEASURE_FUNCTION;

  // don't do anything that would require graphiccontext to be locked before here in fullscreen.
  // that stuff should go into renderfullscreen instead as that is called from the renderin thread

  // dont show GUI when playing full screen video
  if (g_graphicsContext.IsFullScreenVideo() && IsPlaying() && !IsPaused())
  {
    //g_ApplicationRenderer.Render();

#ifdef HAS_SDL
    if (g_videoConfig.GetVSyncMode()==VSYNC_VIDEO)
      g_graphicsContext.getScreenSurface()->EnableVSync(true);
#endif
    // release the context so the async renderer can draw to it
#ifdef HAS_SDL_OPENGL
    // Video rendering occuring from main thread for OpenGL
    if (m_bPresentFrame)
      g_renderManager.Present();
    else
      g_renderManager.RenderUpdate(true, 0, 255);

    ResetScreenSaver();
    g_infoManager.ResetCache();
#else
    //g_graphicsContext.ReleaseCurrentContext();
    g_graphicsContext.Unlock(); // unlock to allow the async renderer to render
    Sleep(25);
    g_graphicsContext.Lock();
    ResetScreenSaver();
    g_infoManager.ResetCache();
#endif
    return;
  }

  g_graphicsContext.AcquireCurrentContext();

#ifdef HAS_SDL
  if (g_videoConfig.GetVSyncMode()==VSYNC_ALWAYS)
    g_graphicsContext.getScreenSurface()->EnableVSync(true);
  else
    g_graphicsContext.getScreenSurface()->EnableVSync(false);
#endif

  g_ApplicationRenderer.Render();
}

void CApplication::DoRender()
{
#ifndef HAS_SDL
  if(!m_pd3dDevice)
    return;
#endif

  g_graphicsContext.Lock();

#ifndef HAS_SDL
  m_pd3dDevice->BeginScene();
#endif

  m_gWindowManager.UpdateModelessVisibility();

  // draw GUI
  g_graphicsContext.Clear();
  //SWATHWIDTH of 4 improves fillrates (performance investigator)
#ifdef HAS_XBOX_D3D
  m_pd3dDevice->SetRenderState(D3DRS_SWATHWIDTH, 4);
#endif
  m_gWindowManager.Render();


  // if we're recording an audio stream then show blinking REC
  if (!g_graphicsContext.IsFullScreenVideo())
  {
    if (m_pPlayer && m_pPlayer->IsRecording() )
    {
      static int iBlinkRecord = 0;
      iBlinkRecord++;
      if (iBlinkRecord > 25)
      {
        CGUIFont* pFont = g_fontManager.GetFont("font13");
        CGUITextLayout::DrawText(pFont, 60, 50, 0xffff0000, 0, "REC", 0);
      }

      if (iBlinkRecord > 50)
        iBlinkRecord = 0;
    }
  }

  // Now render any dialogs
  m_gWindowManager.RenderDialogs();

  // Render the mouse pointer
  if (g_Mouse.IsActive() && g_Mouse.IsEnabled())
  {
    m_guiPointer.Render();
  }

  {
    // free memory if we got les then 10megs free ram
    MEMORYSTATUS stat;
    GlobalMemoryStatus(&stat);
    DWORD dwMegFree = (DWORD)(stat.dwAvailPhys / (1024 * 1024));
    if (dwMegFree <= 10)
    {
      g_TextureManager.Flush();
    }

    // reset image scaling and effect states
    g_graphicsContext.SetRenderingResolution(g_graphicsContext.GetVideoResolution(), 0, 0, false);

    // If we have the remote codes enabled, then show them
    if (g_advancedSettings.m_displayRemoteCodes)
    {
#ifdef HAS_IR_REMOTE
      XBIR_REMOTE* pRemote = &m_DefaultIR_Remote;
      static iRemoteCode = 0;
      static iShowRemoteCode = 0;
      if (pRemote->wButtons)
      {
        iRemoteCode = 255 - pRemote->wButtons; // remote OBC code is 255-wButtons
        iShowRemoteCode = 50;
      }
      if (iShowRemoteCode > 0)
      {
        CStdStringW wszText;
        wszText.Format(L"Remote Code: %i", iRemoteCode);
        float x = 0.08f * g_graphicsContext.GetWidth();
        float y = 0.12f * g_graphicsContext.GetHeight();
#ifndef _DEBUG
        if (LOG_LEVEL_DEBUG_FREEMEM > g_advancedSettings.m_logLevel)
          y = 0.08f * g_graphicsContext.GetHeight();
#endif
        CGUITextLayout::DrawOutlineText(g_fontManager.GetFont("font13"), x, y, 0xffffffff, 0xff000000, 2, wszText);
        iShowRemoteCode--;
      }
#endif
    }

    RenderMemoryStatus();
  }

#ifndef HAS_SDL
  m_pd3dDevice->EndScene();
#endif

#ifdef HAS_XBOX_D3D
  m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
#endif
  g_graphicsContext.Unlock();

  // reset our info cache - we do this at the end of Render so that it is
  // fresh for the next process(), or after a windowclose animation (where process()
  // isn't called)
  g_infoManager.ResetCache();
}

void CApplication::NewFrame()
{
#ifdef HAS_SDL
  SDL_mutexP(m_frameMutex);

  // We just posted another frame. Keep track and notify.
  m_frameCount++;
  SDL_CondSignal(m_frameCond);

  SDL_mutexV(m_frameMutex);
#endif
}

void CApplication::SetQuiet(bool bQuiet)
{
  m_bQuiet = bQuiet;
}

#ifndef HAS_XBOX_D3D
void CApplication::Render()
{
  if (!m_AppActive && !m_bStop && (IsPlaying() == false || IsPaused() == true))
  {
    Sleep(1); 
    ResetScreenSaver(); 
    return;
  }

#ifdef __APPLE__
  // Make sure the system mouse cursor is hidden. I'm not sure if this
  // is a question of needing to call it once a bit later than we do
  // (see http://lists.apple.com/archives/Mac-opengl/2006/Nov/msg00005.html)
  // or a question of something making the cursor appear at a later time.
  // Either way, this should fix it!
  //
  static int frameCount = 0;
  if (frameCount++ % 10 == 0)
    if (g_advancedSettings.m_fullScreen)
      Cocoa_HideMouse();
#endif

  MEASURE_FUNCTION;

  {
    // Frame rate limiter.
    unsigned int singleFrameTime = 1000 / (g_graphicsContext.GetFPS() != 0 ? g_graphicsContext.GetFPS() : 75); // Default limit ~77 FPS
    static unsigned int lastFrameTime = 0;
    unsigned int currentTime = timeGetTime();
    int nDelayTime = 0;
    
#ifdef HAS_SDL
    m_bPresentFrame = false;
    if (g_graphicsContext.IsFullScreenVideo() && !IsPaused())
    {
      SDL_mutexP(m_frameMutex);

      // If we have frames or if we get notified of one, consume it.
      if (m_frameCount > 0 || SDL_CondWaitTimeout(m_frameCond, m_frameMutex, 100) == 0)
      {
        m_frameCount--;
        m_bPresentFrame = true;
      }
      SDL_mutexV(m_frameMutex);
    }
    else
    {
      // only "limit frames" if we are not using vsync.
      double graphicsFPS = (double)g_infoManager.GetFPS();
      double screenFPS = (double)g_graphicsContext.GetFPS();

      if (g_videoConfig.GetVSyncMode() != VSYNC_ALWAYS ||
          (graphicsFPS > screenFPS + 10) && graphicsFPS > 1000/singleFrameTime)
      {
        if (lastFrameTime + singleFrameTime > currentTime)
          nDelayTime = lastFrameTime + singleFrameTime - currentTime;
        
        // This doesn't reliably work, since on NVidia hardware screenFPS != 0 and vsync stops working when the 
        // application is hidden. Since the frame rate limiter now works really well, it's not a big deal.
        //
        //if (screenFPS > 0 && g_videoConfig.GetVSyncMode() == VSYNC_ALWAYS)
        //  CLog::Log(LOGWARNING, "VSYNC ignored by driver (FPS=%.0f) enabling framerate limiter to sleep (%d)", graphicsFPS, nDelayTime);

        Sleep(nDelayTime);
      }
    }
#else
    if (lastFrameTime + singleFrameTime > currentTime)
      nDelayTime = lastFrameTime + singleFrameTime - currentTime;

    m_bPresentFrame = true;
    Sleep(nDelayTime);
#endif

    lastFrameTime = timeGetTime();
  }
  g_graphicsContext.Lock();
  RenderNoPresent();
  // Present the backbuffer contents to the display
#ifndef HAS_SDL
  if (m_pd3dDevice) m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
#elif defined(HAS_SDL_2D)
  g_graphicsContext.Flip();
#elif defined(HAS_SDL_OPENGL)
  g_graphicsContext.Flip();
#endif
  g_graphicsContext.Unlock();
}
#endif

void CApplication::RenderMemoryStatus()
{
  MEASURE_FUNCTION;

  g_infoManager.UpdateFPS();
  g_cpuInfo.getUsedPercentage(); // must call it to recalculate pct values

#if !defined(_DEBUG) && !defined(PROFILE)
  if (LOG_LEVEL_DEBUG_FREEMEM <= g_advancedSettings.m_logLevel)
#endif
  {
    // reset the window scaling and fade status
    RESOLUTION res = g_graphicsContext.GetVideoResolution();
    g_graphicsContext.SetRenderingResolution(res, 0, 0, false);

    if (!m_bQuiet)
    {
      CStdStringW wszText;
      MEMORYSTATUS stat;
      GlobalMemoryStatus(&stat);
#ifdef __APPLE__
      double dCPU = m_resourceCounter.GetCPUUsage();
      wszText.Format(L"FreeMem %ju/%ju MB, FPS %2.1f, CPU-Total %d%%. CPU-XBMC %4.2f%%", stat.dwAvailPhys/(1024*1024), stat.dwTotalPhys/(1024*1024),
               g_infoManager.GetFPS(), g_cpuInfo.getUsedPercentage(), dCPU);
#elif !defined(_LINUX)
      wszText.Format(L"FreeMem %d/%d Kb, FPS %2.1f, CPU %2.0f%%", stat.dwAvailPhys/1024, stat.dwTotalPhys/1024, g_infoManager.GetFPS(), (1.0f - m_idleThread.GetRelativeUsage())*100);
#else
      double dCPU = m_resourceCounter.GetCPUUsage();
      CStdString strCores = g_cpuInfo.GetCoresUsageString();
      wszText.Format(L"FreeMem %d/%d Kb, FPS %2.1f, %s. CPU-XBMC %4.2f%%", stat.dwAvailPhys/1024, stat.dwTotalPhys/1024,
               g_infoManager.GetFPS(), strCores.c_str(), dCPU);
#endif

      static int yShift = 20;
      static int xShift = 40;
      static unsigned int lastShift = time(NULL);
      time_t now = time(NULL);
      if (now - lastShift > 10)
      {
        yShift *= -1;
        if (now % 5 == 0)
          xShift *= -1;
        lastShift = now;
      }

#ifndef __APPLE__
      float x = xShift + 0.04f * g_graphicsContext.GetWidth() + g_settings.m_ResInfo[res].Overscan.left;
      float y = yShift + 0.04f * g_graphicsContext.GetHeight() + g_settings.m_ResInfo[res].Overscan.top;

      // Disable this for now as it might still be responsible for some crashes.
      CGUITextLayout::DrawOutlineText(g_fontManager.GetFont("font13"), x, y, 0xffffffff, 0xff000000, 2, wszText);
#endif
    }
  }
}
// OnKey() translates the key into a CAction which is sent on to our Window Manager.
// The window manager will return true if the event is processed, false otherwise.
// If not already processed, this routine handles global keypresses.  It returns
// true if the key has been processed, false otherwise.

bool CApplication::OnKey(CKey& key)
{
  // Turn the mouse off, as we've just got a keypress from controller or remote
  g_Mouse.SetInactive();
  CAction action;

  action.kKey = g_Keyboard.GetSVKey();
  action.unicode = g_Keyboard.GetUnicode();
  
  // Look for special key combinations.
  if (g_Keyboard.GetCtrl() && g_Keyboard.GetAlt() && g_Keyboard.GetShift())
  {
    if (m_pPlayer != 0)
    {
      m_pPlayer->ExecuteKeyCommand('a' + g_Keyboard.GetAscii() - 1, true, true, false, true);
      g_Keyboard.Reset();
      return true;
    }
  }
  
  if (g_Keyboard.GetAlt()  == true  ||
      g_Keyboard.GetApple() == true ||
      g_Keyboard.GetCtrl() == true  ||
      g_Keyboard.GetRAlt()  == true ||
      Cocoa_IsGUIShowing())
  {
      g_Keyboard.Reset();
      return false;
  }
  
  // a key has been pressed.
  // Reset the screensaver timer
  // but not for the analog thumbsticks/triggers
  if (!key.IsAnalogButton())
  {
#ifdef HAS_XBOX_HARDWARE
    // reset harddisk spindown timer
    m_bSpinDown = false;
    m_bNetworkSpinDown = false;
#endif

    // reset Idle Timer
    m_idleTimer.StartZero();

    ResetScreenSaver();
    if (ResetScreenSaverWindow()) {
      g_Keyboard.Reset();
      return true;
    }
  }

  // get the current active window
  int iWin = m_gWindowManager.GetActiveWindow() & WINDOW_ID_MASK;
  // change this if we have a dialog up
  if (m_gWindowManager.HasModalDialog())
  {
    iWin = m_gWindowManager.GetTopMostModalDialogID() & WINDOW_ID_MASK;
  }
  if (iWin == WINDOW_FULLSCREEN_VIDEO)
  {
    // current active window is full screen video.
    if (g_application.m_pPlayer && g_application.m_pPlayer->IsInMenu())
    {
      // if player is in some sort of menu, (ie DVDMENU) map buttons differently
      g_buttonTranslator.GetAction(WINDOW_VIDEO_MENU, key, action);
    }
    else
    {
      // no then use the fullscreen window section of keymap.xml to map key->action
      g_buttonTranslator.GetAction(iWin, key, action);
    }
  }
  else
  {
    // current active window isnt the fullscreen window
    // just use corresponding section from keymap.xml
    // to map key->action
    // first determine if we should use keyboard input directly
    bool useKeyboard = key.FromKeyboard() && (iWin == WINDOW_DIALOG_KEYBOARD || iWin == WINDOW_DIALOG_NUMERIC);
    CGUIWindow *window = m_gWindowManager.GetWindow(iWin);
    if (window)
    {
      CGUIControl *control = window->GetFocusedControl();
      if (control)
      {
        if (/* control->GetControlType() == CGUIControl::GUICONTROL_EDIT ||*/
            (control->IsContainer() && g_Keyboard.GetShift()) ||
            (key.GetFromHttpApi() && key.GetButtonCode() > KEY_ASCII))
          useKeyboard = true;
      }
    }
    if (useKeyboard)
    {
      if (key.GetFromHttpApi())
      {
        if (key.GetButtonCode() != KEY_INVALID)
          action.wID = (WORD) key.GetButtonCode();
          action.unicode = key.GetUnicode();
      }
      else
      { // see if we've got an ascii key
        if (g_Keyboard.GetUnicode())
        {
#ifdef __APPLE__
          // If not plain ASCII, use the button translator.
          if (g_Keyboard.GetAscii() < 32 || g_Keyboard.GetAscii() > 126)
            g_buttonTranslator.GetAction(iWin, key, action);
          else
#endif
          {
            action.wID = (WORD)g_Keyboard.GetAscii() | KEY_ASCII; // Only for backwards compatibility
            action.unicode = g_Keyboard.GetUnicode();
          }
        }
        else
        {
          action.wID = (WORD)g_Keyboard.GetKey() | KEY_VKEY;
          action.unicode = 0;
        }
      }
    }
    else
    {
      if (key.GetFromHttpApi())
      {
        if (key.GetButtonCode() != KEY_INVALID)
        {
          action.wID = (WORD) key.GetButtonCode();
          g_buttonTranslator.GetAction(iWin, key, action);
        }
      }
      else
        g_buttonTranslator.GetAction(iWin, key, action);
    }
  }
  if (!key.IsAnalogButton())
    CLog::Log(LOGDEBUG, "%s: %i pressed, action is %i", __FUNCTION__, (int) key.GetButtonCode(), action.wID);

  //  Play a sound based on the action
  g_audioManager.PlayActionSound(action);

#ifdef HAS_SDL
  g_Keyboard.Reset();
#endif

  return OnAction(action);
}

bool CApplication::OnAction(const CAction &action)
{
#ifdef HAS_WEB_SERVER
  // Let's tell the outside world about this action
  if (m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=2)
  {
    CStdString tmp;
    tmp.Format("%i",action.wID);
    getApplicationMessenger().HttpApi("broadcastlevel; OnAction:"+tmp+";2");
  }
#endif

  // special case for switching between GUI & fullscreen mode.
  if (action.wID == ACTION_SHOW_GUI)
  { // Switch to fullscreen mode if we can
    if (SwitchToFullScreen())
    {
      m_navigationTimer.StartZero();
      return true;
    }
  }

  if (action.wID == ACTION_TOGGLE_FULLSCREEN)
  {
    g_graphicsContext.ToggleFullScreenRoot();
    return true;
  }

  // in normal case
  // just pass the action to the current window and let it handle it
  if (m_gWindowManager.OnAction(action))
  {
    m_navigationTimer.StartZero();
    return true;
  }

  // handle extra global presses

  // screenshot : take a screenshot :)
  if (action.wID == ACTION_TAKE_SCREENSHOT)
  {
    CUtil::TakeScreenshot();
    return true;
  }
  // built in functions : execute the built-in
  if (action.wID == ACTION_BUILT_IN_FUNCTION)
  {
    CUtil::ExecBuiltIn(action.strAction);
    m_navigationTimer.StartZero();
    return true;
  }

  // power down : turn off after 3 seconds of button down
  static bool PowerButtonDown = false;
  static DWORD PowerButtonCode;
  static DWORD MarkTime;
  if (action.wID == ACTION_POWERDOWN)
  {
    // Hold button for 3 secs to power down
    if (!PowerButtonDown)
    {
      MarkTime = GetTickCount();
      PowerButtonDown = true;
      PowerButtonCode = action.m_dwButtonCode;
    }
  }
  if (PowerButtonDown)
  {
    if (g_application.IsButtonDown(PowerButtonCode))
    {
      if (GetTickCount() >= MarkTime + 3000)
      {
        m_applicationMessenger.Shutdown();
        return true;
      }
    }
    else
      PowerButtonDown = false;
  }
  // show info : Shows the current video or song information
  if (action.wID == ACTION_SHOW_INFO)
  {
    g_infoManager.ToggleShowInfo();
    return true;
  }

  // codec info : Shows the current song, video or picture codec information
  if (action.wID == ACTION_SHOW_CODEC)
  {
    g_infoManager.ToggleShowCodec();
    return true;
  }

  if (action.wID == ACTION_INCREASE_RATING || action.wID == ACTION_DECREASE_RATING && IsPlayingAudio())
  {
    const CMusicInfoTag *tag = g_infoManager.GetCurrentSongTag();
    if (tag)
    {
      *m_itemCurrentFile->GetMusicInfoTag() = *tag;
      char rating = tag->GetRating();
      bool needsUpdate(false);
      if (rating > '0' && action.wID == ACTION_DECREASE_RATING)
      {
        m_itemCurrentFile->GetMusicInfoTag()->SetRating(rating - 1);
        needsUpdate = true;
      }
      else if (rating < '5' && action.wID == ACTION_INCREASE_RATING)
      {
        m_itemCurrentFile->GetMusicInfoTag()->SetRating(rating + 1);
        needsUpdate = true;
      }
      if (needsUpdate)
      {
        CMusicDatabase db;
        if (db.Open())      // OpenForWrite() ?
        {
          db.SetSongRating(m_itemCurrentFile->m_strPath, m_itemCurrentFile->GetMusicInfoTag()->GetRating());
          db.Close();
        }
        // send a message to all windows to tell them to update the fileitem (eg playlistplayer, media windows)
        CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_ITEM, 0, m_itemCurrentFile);
        g_graphicsContext.SendMessage(msg);
      }
    }
    return true;
  }

  // stop : stops playing current audio song
  if (action.wID == ACTION_STOP)
  {
    StopPlaying();
    return true;
  }

  // previous : play previous song from playlist
  if (action.wID == ACTION_PREV_ITEM)
  {
    // first check whether we're within 3 seconds of the start of the track
    // if not, we just revert to the start of the track
    if (GetTime() > 3)
    {
      if (m_pPlayer && m_pPlayer->CanSeek())
      {
        SeekTime(0);
        SetPlaySpeed(1);
      }
      else
      {
        g_playlistPlayer.Play(g_playlistPlayer.GetCurrentSong());
      }

    }
    else
    {
      SaveCurrentFileSettings();
      g_playlistPlayer.PlayPrevious();
    }
    return true;
  }

  // next : play next song from playlist
  if (action.wID == ACTION_NEXT_ITEM)
  {
    if (IsPlaying() && m_pPlayer->SkipNext())
      return true;

    SaveCurrentFileSettings();
    g_playlistPlayer.PlayNext();

    return true;
  }

  if ( IsPlaying())
  {
    // pause : pauses current audio song
    if (action.wID == ACTION_PAUSE && m_iPlaySpeed == 1)
    {
      m_pPlayer->Pause();
      if (!m_pPlayer->IsPaused())
      { // unpaused - set the playspeed back to normal
        SetPlaySpeed(1);
      }
      if (!g_guiSettings.GetBool("lookandfeel.soundsduringplayback"))
        g_audioManager.Enable(m_pPlayer->IsPaused());
      return true;
    }
    if (!m_pPlayer->IsPaused())
    {
      // if we do a FF/RW in my music then map PLAY action togo back to normal speed
      // if we are playing at normal speed, then allow play to pause
      if (action.wID == ACTION_PLAYER_PLAY || action.wID == ACTION_PAUSE)
      {
        if (m_iPlaySpeed != 1)
        {
          SetPlaySpeed(1);
        }
        else
        {
          m_pPlayer->Pause();
        }
        return true;
      }
      if (action.wID == ACTION_PLAYER_FORWARD || action.wID == ACTION_PLAYER_REWIND)
      {
        int iPlaySpeed = m_iPlaySpeed;
        if (action.wID == ACTION_PLAYER_REWIND && iPlaySpeed == 1) // Enables Rewinding
          iPlaySpeed *= -2;
        else if (action.wID == ACTION_PLAYER_REWIND && iPlaySpeed > 1) //goes down a notch if you're FFing
          iPlaySpeed /= 2;
        else if (action.wID == ACTION_PLAYER_FORWARD && iPlaySpeed < 1) //goes up a notch if you're RWing
          iPlaySpeed /= 2;
        else
          iPlaySpeed *= 2;

        if (action.wID == ACTION_PLAYER_FORWARD && iPlaySpeed == -1) //sets iSpeed back to 1 if -1 (didn't plan for a -1)
          iPlaySpeed = 1;
        if (iPlaySpeed > 32 || iPlaySpeed < -32)
          iPlaySpeed = 1;

        SetPlaySpeed(iPlaySpeed);
        return true;
      }
      else if ((action.fAmount1 || GetPlaySpeed() != 1) && (action.wID == ACTION_ANALOG_REWIND || action.wID == ACTION_ANALOG_FORWARD))
      {
        // calculate the speed based on the amount the button is held down
        int iPower = (int)(action.fAmount1 * MAX_FFWD_SPEED + 0.5f);
        // returns 0 -> MAX_FFWD_SPEED
        int iSpeed = 1 << iPower;
        if (iSpeed != 1 && action.wID == ACTION_ANALOG_REWIND)
          iSpeed = -iSpeed;
        g_application.SetPlaySpeed(iSpeed);
        if (iSpeed == 1)
          CLog::Log(LOGDEBUG,"Resetting playspeed");
        return true;
      }
    }
    // allow play to unpause
    else
    {
      if (action.wID == ACTION_PLAYER_PLAY)
      {
        // unpause, and set the playspeed back to normal
        m_pPlayer->Pause();
        if (!g_guiSettings.GetBool("lookandfeel.soundsduringplayback"))
          g_audioManager.Enable(m_pPlayer->IsPaused());

        g_application.SetPlaySpeed(1);
        return true;
      }
    }
  }
  if (action.wID == ACTION_MUTE)
  {
    Mute();
    return true;
  }

  // Check for global volume control
  if (action.fAmount1 && (action.wID == ACTION_VOLUME_UP || action.wID == ACTION_VOLUME_DOWN))
  {
    // increase or decrease the volume
    int volume = g_stSettings.m_nVolumeLevel + g_stSettings.m_dynamicRangeCompressionLevel;

    // calculate speed so that a full press will equal 1 second from min to max
    float speed = float(VOLUME_MAXIMUM - VOLUME_MINIMUM);
    if( action.fRepeat )
      speed *= action.fRepeat;
    else
      speed /= 50; //50 fps

    if (action.wID == ACTION_VOLUME_UP)
    {
      volume += (int)((float)fabs(action.fAmount1) * action.fAmount1 * speed);
    }
    else
    {
      volume -= (int)((float)fabs(action.fAmount1) * action.fAmount1 * speed);
    }

    SetHardwareVolume(volume);
#ifndef HAS_SDL_AUDIO
    g_audioManager.SetVolume(g_stSettings.m_nVolumeLevel);
#else
    g_audioManager.SetVolume((int)(128.f * (g_stSettings.m_nVolumeLevel - VOLUME_MINIMUM) / (float)(VOLUME_MAXIMUM - VOLUME_MINIMUM)));
#endif

    // show visual feedback of volume change...
    m_guiDialogVolumeBar.Show();
    m_guiDialogVolumeBar.OnAction(action);
    return true;
  }
  // Check for global seek control
  if (IsPlaying() && action.fAmount1 && (action.wID == ACTION_ANALOG_SEEK_FORWARD || action.wID == ACTION_ANALOG_SEEK_BACK))
  {
    if (!m_pPlayer->CanSeek()) return false;
    CScrobbler::GetInstance()->SetSubmitSong(false);  // Do not submit songs to Audioscrobbler when seeking, see CheckAudioScrobblerStatus()
    m_guiDialogSeekBar.OnAction(action);
    return true;
  }
  return false;
}

#ifdef HAS_KAI
void CApplication::SetKaiNotification(const CStdString& aCaption, const CStdString& aDescription, CGUIImage* aIcon/*=NULL*/)
{
  // queue toast notification
  if (g_guiSettings.GetBool("xlinkkai.enablenotifications"))
  {
    if (aIcon==NULL)
      m_guiDialogKaiToast.QueueNotification(aCaption, aDescription);
    else
      m_guiDialogKaiToast.QueueNotification(aIcon->GetFileName(), aCaption, aDescription);
  }
}
#endif

void CApplication::UpdateLCD()
{
#ifdef HAS_LCD
  static long lTickCount = 0;

  if (!g_lcd || g_guiSettings.GetInt("lcd.type") == LCD_TYPE_NONE)
    return ;
  long lTimeOut = 1000;
  if ( m_iPlaySpeed != 1)
    lTimeOut = 0;
  if ( ((long)GetTickCount() - lTickCount) >= lTimeOut)
  {
    if (g_application.NavigationIdleTime() < 5)
      g_lcd->Render(ILCD::LCD_MODE_NAVIGATION);
    else if (IsPlayingVideo())
      g_lcd->Render(ILCD::LCD_MODE_VIDEO);
    else if (IsPlayingAudio())
      g_lcd->Render(ILCD::LCD_MODE_MUSIC);
    else if (IsInScreenSaver())
      g_lcd->Render(ILCD::LCD_MODE_SCREENSAVER);
    else
      g_lcd->Render(ILCD::LCD_MODE_GENERAL);

    // reset tick count
    lTickCount = GetTickCount();
  }
#endif
}

void CApplication::FrameMove()
{
  MEASURE_FUNCTION;

  // currently we calculate the repeat time (ie time from last similar keypress) just global as fps
  float frameTime = m_frameTime.GetElapsedSeconds();
  m_frameTime.StartZero();
  // never set a frametime less than 2 fps to avoid problems when debuggin and on breaks
  if( frameTime > 0.5 ) frameTime = 0.5;

#ifdef HAS_KAI
  if (g_guiSettings.GetBool("xlinkkai.enabled"))
  {
    CKaiClient::GetInstance()->DoWork();
  }
#endif

  // check if there are notifications to display
  if (m_guiDialogKaiToast.DoWork())
  {
    if (!m_guiDialogKaiToast.IsDialogRunning())
    {
      m_guiDialogKaiToast.Show();
    }
  }

  UpdateLCD();

  // read raw input from controller, remote control, mouse and keyboard
  ReadInput();
  // process input actions
  ProcessMouse();
  ProcessHTTPApiButtons();
  ProcessKeyboard();
  ProcessRemote(frameTime);
  ProcessGamepad(frameTime);
  ProcessEventServer(frameTime);
}

bool CApplication::ProcessGamepad(float frameTime)
{
#ifdef HAS_GAMEPAD
  // Handle the gamepad button presses.  We check for button down,
  // then call OnKey() which handles the translation to actions, and sends the
  // action to our window manager's OnAction() function, which filters the messages
  // to where they're supposed to end up, returning true if the message is successfully
  // processed.  If OnKey() returns false, then the key press wasn't processed at all,
  // and we can safely process the next key (or next check on the same key in the
  // case of the analog sticks which can produce more than 1 key event.)

  WORD wButtons = m_DefaultGamepad.wButtons;
  WORD wDpad = wButtons & (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_RIGHT);

  BYTE bLeftTrigger = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER];
  BYTE bRightTrigger = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER];
  BYTE bButtonA = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_A];
  BYTE bButtonB = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_B];
  BYTE bButtonX = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_X];
  BYTE bButtonY = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_Y];

  // pass them through the delay
  WORD wDir = m_ctrDpad.DpadInput(wDpad, 0 != bLeftTrigger, 0 != bRightTrigger);

  // map all controller & remote actions to their keys
  if (m_DefaultGamepad.fX1 || m_DefaultGamepad.fY1)
  {
    CKey key(KEY_BUTTON_LEFT_THUMB_STICK, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.fX2 || m_DefaultGamepad.fY2)
  {
    CKey key(KEY_BUTTON_RIGHT_THUMB_STICK, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  // direction specific keys (for defining different actions for each direction)
  // We need to be able to know when it last had a direction, so that we can
  // post the reset direction code the next time around (to reset scrolling,
  // fastforwarding and other analog actions)

  // For the sticks, once it is pushed in one direction (eg up) it will only
  // detect movement in that direction of movement (eg up or down) - the other
  // direction (eg left and right) will not be registered until the stick has
  // been recentered for at least 2 frames.

  // first the right stick
  static lastRightStickKey = 0;
  int newRightStickKey = 0;
  if (lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_UP || lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_DOWN)
  {
    if (m_DefaultGamepad.fY2 > 0)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_UP;
    else if (m_DefaultGamepad.fY2 < 0)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_DOWN;
    else if (m_DefaultGamepad.fX2 != 0)
    {
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_UP;
      //m_DefaultGamepad.fY2 = 0.00001f; // small amount of movement
    }
  }
  else if (lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_LEFT || lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT)
  {
    if (m_DefaultGamepad.fX2 > 0)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
    else if (m_DefaultGamepad.fX2 < 0)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_LEFT;
    else if (m_DefaultGamepad.fY2 != 0)
    {
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
      //m_DefaultGamepad.fX2 = 0.00001f; // small amount of movement
    }
  }
  else
  {
    if (m_DefaultGamepad.fY2 > 0 && m_DefaultGamepad.fX2*2 < m_DefaultGamepad.fY2 && -m_DefaultGamepad.fX2*2 < m_DefaultGamepad.fY2)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_UP;
    else if (m_DefaultGamepad.fY2 < 0 && m_DefaultGamepad.fX2*2 < -m_DefaultGamepad.fY2 && -m_DefaultGamepad.fX2*2 < -m_DefaultGamepad.fY2)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_DOWN;
    else if (m_DefaultGamepad.fX2 > 0 && m_DefaultGamepad.fY2*2 < m_DefaultGamepad.fX2 && -m_DefaultGamepad.fY2*2 < m_DefaultGamepad.fX2)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
    else if (m_DefaultGamepad.fX2 < 0 && m_DefaultGamepad.fY2*2 < -m_DefaultGamepad.fX2 && -m_DefaultGamepad.fY2*2 < -m_DefaultGamepad.fX2)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_LEFT;
  }
  if (lastRightStickKey && newRightStickKey != lastRightStickKey)
  { // was held down last time - and we have a new key now
    // post old key reset message...
    CKey key(lastRightStickKey, 0, 0, 0, 0, 0, 0);
    lastRightStickKey = newRightStickKey;
    if (OnKey(key)) return true;
  }
  lastRightStickKey = newRightStickKey;
  // post the new key's message
  if (newRightStickKey)
  {
    CKey key(newRightStickKey, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }

  // now the left stick
  static lastLeftStickKey = 0;
  int newLeftStickKey = 0;
  if (lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_UP || lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_DOWN)
  {
    if (m_DefaultGamepad.fY1 > 0)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_UP;
    else if (m_DefaultGamepad.fY1 < 0)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_DOWN;
  }
  else if (lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_LEFT || lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_RIGHT)
  {
    if (m_DefaultGamepad.fX1 > 0)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_RIGHT;
    else if (m_DefaultGamepad.fX1 < 0)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_LEFT;
  }
  else
  { // check for a new control movement
    if (m_DefaultGamepad.fY1 > 0 && m_DefaultGamepad.fX1 < m_DefaultGamepad.fY1 && -m_DefaultGamepad.fX1 < m_DefaultGamepad.fY1)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_UP;
    else if (m_DefaultGamepad.fY1 < 0 && m_DefaultGamepad.fX1 < -m_DefaultGamepad.fY1 && -m_DefaultGamepad.fX1 < -m_DefaultGamepad.fY1)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_DOWN;
    else if (m_DefaultGamepad.fX1 > 0 && m_DefaultGamepad.fY1 < m_DefaultGamepad.fX1 && -m_DefaultGamepad.fY1 < m_DefaultGamepad.fX1)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_RIGHT;
    else if (m_DefaultGamepad.fX1 < 0 && m_DefaultGamepad.fY1 < -m_DefaultGamepad.fX1 && -m_DefaultGamepad.fY1 < -m_DefaultGamepad.fX1)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_LEFT;
  }

  if (lastLeftStickKey && newLeftStickKey != lastLeftStickKey)
  { // was held down last time - and we have a new key now
    // post old key reset message...
    CKey key(lastLeftStickKey, 0, 0, 0, 0, 0, 0);
    lastLeftStickKey = newLeftStickKey;
    if (OnKey(key)) return true;
  }
  lastLeftStickKey = newLeftStickKey;
  // post the new key's message
  if (newLeftStickKey)
  {
    CKey key(newLeftStickKey, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }

  // Trigger detection
  static lastTriggerKey = 0;
  int newTriggerKey = 0;
  if (bLeftTrigger)
    newTriggerKey = KEY_BUTTON_LEFT_ANALOG_TRIGGER;
  else if (bRightTrigger)
    newTriggerKey = KEY_BUTTON_RIGHT_ANALOG_TRIGGER;
  if (lastTriggerKey && newTriggerKey != lastTriggerKey)
  { // was held down last time - and we have a new key now
    // post old key reset message...
    CKey key(lastTriggerKey, 0, 0, 0, 0, 0, 0);
    lastTriggerKey = newTriggerKey;
    if (OnKey(key)) return true;
  }
  lastTriggerKey = newTriggerKey;
  // post the new key's message
  if (newTriggerKey)
  {
    CKey key(newTriggerKey, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }

  // Now the digital buttons...
  if ( wDir & DC_LEFTTRIGGER)
  {
    CKey key(KEY_BUTTON_LEFT_TRIGGER, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if ( wDir & DC_RIGHTTRIGGER)
  {
    CKey key(KEY_BUTTON_RIGHT_TRIGGER, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if ( wDir & DC_LEFT )
  {
    CKey key(KEY_BUTTON_DPAD_LEFT, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if ( wDir & DC_RIGHT)
  {
    CKey key(KEY_BUTTON_DPAD_RIGHT, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if ( wDir & DC_UP )
  {
    CKey key(KEY_BUTTON_DPAD_UP, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if ( wDir & DC_DOWN )
  {
    CKey key(KEY_BUTTON_DPAD_DOWN, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }

  if (m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_BACK )
  {
    CKey key(KEY_BUTTON_BACK, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_START)
  {
    CKey key(KEY_BUTTON_START, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_LEFT_THUMB)
  {
    CKey key(KEY_BUTTON_LEFT_THUMB_BUTTON, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
  {
    CKey key(KEY_BUTTON_RIGHT_THUMB_BUTTON, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_A])
  {
    CKey key(KEY_BUTTON_A, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_B])
  {
    CKey key(KEY_BUTTON_B, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }

  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_X])
  {
    CKey key(KEY_BUTTON_X, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_Y])
  {
    CKey key(KEY_BUTTON_Y, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_BLACK])
  {
    CKey key(KEY_BUTTON_BLACK, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_WHITE])
  {
    CKey key(KEY_BUTTON_WHITE, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
#endif
#ifdef HAS_SDL_JOYSTICK
  int iWin = m_gWindowManager.GetActiveWindow() & WINDOW_ID_MASK;
  if (m_gWindowManager.HasModalDialog())
  {
    iWin = m_gWindowManager.GetTopMostModalDialogID() & WINDOW_ID_MASK;
  }
  int bid;
  g_Joystick.Update();
  if (g_Joystick.GetButton(bid))
  {
    // reset Idle Timer
    m_idleTimer.StartZero();

    ResetScreenSaver();
    if (ResetScreenSaverWindow())
    {
      g_Joystick.Reset(true);
      return true;
    }

    CAction action;
    bool fullrange;
    string jname = g_Joystick.GetJoystick();
    if (g_buttonTranslator.TranslateJoystickString(iWin, jname.c_str(), bid, false, action.wID, action.strAction, fullrange))
    {
      action.fAmount1 = 1.0f;
      action.fRepeat = 0.0f;
      g_audioManager.PlayActionSound(action);
      g_Joystick.Reset();
      g_Mouse.SetInactive();
      return OnAction(action);
    }
    else
    {
      g_Joystick.Reset();
    }
  }
  if (g_Joystick.GetAxis(bid))
  {
    CAction action;
    bool fullrange;

    string jname = g_Joystick.GetJoystick();
    action.fAmount1 = g_Joystick.GetAmount();
    if (action.fAmount1<0)
    {
      bid = -bid;
    }
    if (g_buttonTranslator.TranslateJoystickString(iWin, jname.c_str(), bid, true, action.wID, action.strAction, fullrange))
    {
      ResetScreenSaver();
      if (ResetScreenSaverWindow())
      {
        return true;
      }

      if (fullrange)
      {
        action.fAmount1 = (action.fAmount1+1.0f)/2.0f;
      }
      else
      {
        action.fAmount1 = fabs(action.fAmount1);
      }
      action.fAmount2 = 0.0;
      action.fRepeat = 0.0;
      g_audioManager.PlayActionSound(action);
      g_Joystick.Reset();
      g_Mouse.SetInactive();
      return OnAction(action);
    }
    else
    {
      g_Joystick.ResetAxis(abs(bid));
    }
  }
#endif
  return false;
}

bool CApplication::ProcessRemote(float frameTime)
{
#ifdef HAS_IR_REMOTE
  if (m_DefaultIR_Remote.wButtons)
  {
    // time depends on whether the movement is repeated (held down) or not.
    // If it is, we use the FPS timer to get a repeatable speed.
    // If it isn't, we use 20 to get repeatable jumps.
    float time = (m_DefaultIR_Remote.bHeldDown) ? frameTime : 0.020f;
    CKey key(m_DefaultIR_Remote.wButtons, 0, 0, 0, 0, 0, 0, time);
    return OnKey(key);
  }
#endif
#ifdef HAS_LIRC
  if (g_RemoteControl.GetButton())
  {
    // time depends on whether the movement is repeated (held down) or not.
    // If it is, we use the FPS timer to get a repeatable speed.
    // If it isn't, we use 20 to get repeatable jumps.
    float time = (g_RemoteControl.IsHolding()) ? frameTime : 0.020f;
    CKey key(g_RemoteControl.GetButton(), 0, 0, 0, 0, 0, 0, time);
    g_RemoteControl.Reset();
    return OnKey(key);
  }
#endif
  return false;
}

bool CApplication::ProcessMouse()
{
  MEASURE_FUNCTION;

  if (!g_Mouse.IsActive())
    return false;

  // Reset the screensaver and idle timers
  m_idleTimer.StartZero();
  ResetScreenSaver();
  if (ResetScreenSaverWindow())
    return true;

  if (!g_Mouse.IsEnabled())
    return false;

  // call OnAction with ACTION_MOUSE
  CAction action;
  action.wID = ACTION_MOUSE;
  action.fAmount1 = (float) m_guiPointer.GetPosX();
  action.fAmount2 = (float) m_guiPointer.GetPosY();

  return m_gWindowManager.OnAction(action);
}

void  CApplication::CheckForTitleChange()
{
  if (g_stSettings.m_HttpApiBroadcastLevel>=1)
    if (IsPlayingVideo())
    {
      const CVideoInfoTag* tagVal = g_infoManager.GetCurrentMovieTag();
      if (m_pXbmcHttp && tagVal && !(tagVal->m_strTitle.IsEmpty()))
      {
        CStdString msg=m_pXbmcHttp->GetOpenTag()+"MovieTitle:"+tagVal->m_strTitle+m_pXbmcHttp->GetCloseTag();
        if (m_prevMedia!=msg && g_stSettings.m_HttpApiBroadcastLevel>=1)
        {
          getApplicationMessenger().HttpApi("broadcastlevel; MediaChanged:"+msg+";1");
          m_prevMedia=msg;
        }
      }
    }
    else if (IsPlayingAudio())
    {
      const CMusicInfoTag* tagVal=g_infoManager.GetCurrentSongTag();
      if (m_pXbmcHttp && tagVal)
	  {
	    CStdString msg="";
	    if (!tagVal->GetTitle().IsEmpty())
        msg=m_pXbmcHttp->GetOpenTag()+"AudioTitle:"+tagVal->GetTitle()+m_pXbmcHttp->GetCloseTag();
	    if (!tagVal->GetArtist().IsEmpty())
        msg+=m_pXbmcHttp->GetOpenTag()+"AudioArtist:"+tagVal->GetArtist()+m_pXbmcHttp->GetCloseTag();
	    if (m_prevMedia!=msg)
	    {
        getApplicationMessenger().HttpApi("broadcastlevel; MediaChanged:"+msg+";1");
	      m_prevMedia=msg;
	    }
      }
    }
}


bool CApplication::ProcessHTTPApiButtons()
{
#ifdef HAS_WEB_SERVER
  if (m_pXbmcHttp)
  {
    // copy key from webserver, and reset it in case we're called again before
    // whatever happens in OnKey()
    CKey keyHttp(m_pXbmcHttp->GetKey());
    m_pXbmcHttp->ResetKey();
    if (keyHttp.GetButtonCode() != KEY_INVALID)
    {
      if (keyHttp.GetButtonCode() == KEY_VMOUSE) //virtual mouse
      {
        CAction action;
        action.wID = ACTION_MOUSE;
        g_Mouse.SetLocation(CPoint(keyHttp.GetLeftThumbX(), keyHttp.GetLeftThumbY()));
        if (keyHttp.GetLeftTrigger()!=0)
          g_Mouse.bClick[keyHttp.GetLeftTrigger()-1]=true;
        if (keyHttp.GetRightTrigger()!=0)
          g_Mouse.bDoubleClick[keyHttp.GetRightTrigger()-1]=true;
        action.fAmount1 = keyHttp.GetLeftThumbX();
        action.fAmount2 = keyHttp.GetLeftThumbY();
        m_gWindowManager.OnAction(action);
      }
      else
        OnKey(keyHttp);
      return true;
    }
  }
  return false;
#endif
}

bool CApplication::ProcessEventServer(float frameTime)
{
#ifdef HAS_EVENT_SERVER
  CEventServer* es = CEventServer::GetInstance();
  if (!es || !es->Running())
    return false;

  // process any queued up actions
  if (es->ExecuteNextAction())
  {
    // reset idle timers
    m_idleTimer.StartZero();
    ResetScreenSaver();
    ResetScreenSaverWindow();
  }
  
  std::string joystickName;
  bool isAxis = false;
  float fAmount = 0.0;
  bool done = false;

  while (!done)
  {
    WORD wKeyID = es->GetButtonCode(joystickName, isAxis, fAmount);

    if (wKeyID)
    {
      // If it's an axis, save the value to repeat it.
      if (isAxis == true)
      {
        if (fabs(fAmount) >= 0.2)
          m_lastAxisMap[joystickName][wKeyID] = fAmount;
        else
          m_lastAxisMap[joystickName].erase(wKeyID);
      }
    }
    else if (m_lastAxisMap.size() > 0)
    {
      // Process all the stored axis.
      for (map<std::string, map<int, float> >::iterator iter = m_lastAxisMap.begin(); iter != m_lastAxisMap.end(); ++iter)
      {
        for (map<int, float>::iterator iterAxis = (*iter).second.begin(); iterAxis != (*iter).second.end(); ++iterAxis)
          ProcessJoystickEvent((*iter).first, (*iterAxis).first, true, (*iterAxis).second);
      }
    }

    if (wKeyID)
    {
      if (joystickName.length() > 0)
      {
        ProcessJoystickEvent(joystickName, wKeyID, isAxis, fAmount);
      }
      else
      {
        CKey key(wKeyID);
        return OnKey( key );
      }
    }
    else
    {
      done = true;
    }
  }

  done = false;
  while (!done)
  {
    CAction action;
    action.wID = ACTION_MOUSE;
    if (es->GetMousePos(action.fAmount1, action.fAmount2) && g_Mouse.IsEnabled())
    {
      CPoint point;
      point.x = action.fAmount1;
      point.y = action.fAmount2;
      g_Mouse.SetLocation(point, true);

      return m_gWindowManager.OnAction(action);
    }
    else
    {
      done = true;
    }
  }
#endif
  return false;
}

bool CApplication::ProcessJoystickEvent(const std::string& joystickName, int wKeyID, bool isAxis, float fAmount)
{
#ifdef HAS_EVENT_SERVER
  m_idleTimer.StartZero();

   // Make sure to reset screen saver, mouse.
   ResetScreenSaver();
   if (ResetScreenSaverWindow())
     return true;

#ifdef HAS_SDL_JOYSTICK
   g_Joystick.Reset();
#endif
   g_Mouse.SetInactive();

   // Figure out what window we're taking the event for.
   WORD iWin = m_gWindowManager.GetActiveWindow() & WINDOW_ID_MASK;
   if (m_gWindowManager.HasModalDialog())
       iWin = m_gWindowManager.GetTopMostModalDialogID() & WINDOW_ID_MASK;

   // This code is copied from the OnKey handler, it should be factored out.
   if (iWin == WINDOW_FULLSCREEN_VIDEO &&
       g_application.m_pPlayer &&
       g_application.m_pPlayer->IsInMenu())
   {
     // If player is in some sort of menu, (ie DVDMENU) map buttons differently.
     iWin = WINDOW_VIDEO_MENU;
   }

   bool fullRange = false;
   CAction action;
   action.fAmount1 = fAmount;

   //if (action.fAmount1 < 0.0)
   // wKeyID = -wKeyID;

   // Translate using regular joystick translator.
   if (g_buttonTranslator.TranslateJoystickString(iWin, joystickName.c_str(), wKeyID, isAxis, action.wID, action.strAction, fullRange))
   {
     action.fRepeat = 0.0f;
     g_audioManager.PlayActionSound(action);
     return OnAction(action);
   }
   else
   {
     CLog::Log(LOGDEBUG, "ERROR mapping joystick (%s)", joystickName.c_str());
   }
#endif

   return false;
}

bool CApplication::ProcessKeyboard()
{
  MEASURE_FUNCTION;

  // process the keyboard buttons etc.
  BYTE vkey = g_Keyboard.GetKey();
  WCHAR unicode = g_Keyboard.GetUnicode();
  if (vkey || unicode)
  {
    // got a valid keypress - convert to a key code
    WORD wkeyID;
    if (vkey) // FIXME, every ascii has a vkey so vkey would always and ascii would never be processed, but fortunately OnKey uses wkeyID only to detect keyboard use and the real key is recalculated correctly.
      wkeyID = (WORD)vkey | KEY_VKEY;
    else
      wkeyID = KEY_UNICODE;
    //  CLog::Log(LOGDEBUG,"Keyboard: time=%i key=%i", timeGetTime(), vkey);
    CKey key(wkeyID);
    return OnKey(key);
  }
  return false;
}

bool CApplication::IsButtonDown(DWORD code)
{
#ifdef HAS_GAMEPAD
  if (code >= KEY_BUTTON_A && code <= KEY_BUTTON_RIGHT_TRIGGER)
  {
    // analogue
    return (m_DefaultGamepad.bAnalogButtons[code - KEY_BUTTON_A + XINPUT_GAMEPAD_A] > XINPUT_GAMEPAD_MAX_CROSSTALK);
  }
  else if (code >= KEY_BUTTON_DPAD_UP && code <= KEY_BUTTON_RIGHT_THUMB_BUTTON)
  {
    // digital
    return (m_DefaultGamepad.wButtons & (1 << (code - KEY_BUTTON_DPAD_UP))) != 0;
  }
  else
  {
    // remote
    return m_DefaultIR_Remote.wButtons == code;
  }
#endif
  return false;
}

bool CApplication::AnyButtonDown()
{
  ReadInput();
#ifdef HAS_GAMEPAD
  if (m_DefaultGamepad.wPressedButtons || m_DefaultIR_Remote.wButtons)
    return true;

  for (int i = 0; i < 6; ++i)
  {
    if (m_DefaultGamepad.bPressedAnalogButtons[i])
      return true;
  }
#endif
  return false;
}

HRESULT CApplication::Cleanup()
{
  try
  {
    if (m_pXbmcHttp)
    {
	  if(g_stSettings.m_HttpApiBroadcastLevel>=1)
	    getApplicationMessenger().HttpApi("broadcastlevel; ShutDown;1");
	  m_pXbmcHttp->shuttingDown=true;
     //Sleep(100);
    }

    m_gWindowManager.Delete(WINDOW_MUSIC_PLAYLIST);
    m_gWindowManager.Delete(WINDOW_MUSIC_PLAYLIST_EDITOR);
    m_gWindowManager.Delete(WINDOW_MUSIC_FILES);
    m_gWindowManager.Delete(WINDOW_MUSIC_NAV);
    m_gWindowManager.Delete(WINDOW_MUSIC_INFO);
    m_gWindowManager.Delete(WINDOW_VIDEO_INFO);
    m_gWindowManager.Delete(WINDOW_VIDEO_FILES);
    m_gWindowManager.Delete(WINDOW_VIDEO_PLAYLIST);
    m_gWindowManager.Delete(WINDOW_VIDEO_NAV);
    m_gWindowManager.Delete(WINDOW_FILES);
    m_gWindowManager.Delete(WINDOW_MUSIC_INFO);
    m_gWindowManager.Delete(WINDOW_VIDEO_INFO);
    m_gWindowManager.Delete(WINDOW_DIALOG_YES_NO);
    m_gWindowManager.Delete(WINDOW_DIALOG_PROGRESS);
    m_gWindowManager.Delete(WINDOW_DIALOG_NUMERIC);
    m_gWindowManager.Delete(WINDOW_DIALOG_GAMEPAD);
    m_gWindowManager.Delete(WINDOW_DIALOG_SUB_MENU);
    m_gWindowManager.Delete(WINDOW_DIALOG_BUTTON_MENU);
    m_gWindowManager.Delete(WINDOW_DIALOG_CONTEXT_MENU);
    m_gWindowManager.Delete(WINDOW_DIALOG_MUSIC_SCAN);
    m_gWindowManager.Delete(WINDOW_DIALOG_PLAYER_CONTROLS);
    m_gWindowManager.Delete(WINDOW_DIALOG_MUSIC_OSD);
    m_gWindowManager.Delete(WINDOW_DIALOG_VIS_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_VIS_PRESET_LIST);
    m_gWindowManager.Delete(WINDOW_DIALOG_SELECT);
    m_gWindowManager.Delete(WINDOW_DIALOG_OK);
    m_gWindowManager.Delete(WINDOW_DIALOG_FILESTACKING);
    m_gWindowManager.Delete(WINDOW_DIALOG_INVITE);
    m_gWindowManager.Delete(WINDOW_DIALOG_HOST);
    m_gWindowManager.Delete(WINDOW_DIALOG_KEYBOARD);
    m_gWindowManager.Delete(WINDOW_FULLSCREEN_VIDEO);
    m_gWindowManager.Delete(WINDOW_DIALOG_TRAINER_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_PROFILE_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_LOCK_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_NETWORK_SETUP);
    m_gWindowManager.Delete(WINDOW_DIALOG_MEDIA_SOURCE);
    m_gWindowManager.Delete(WINDOW_DIALOG_VIDEO_OSD_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_AUDIO_OSD_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_VIDEO_BOOKMARKS);
    m_gWindowManager.Delete(WINDOW_DIALOG_VIDEO_SCAN);
    m_gWindowManager.Delete(WINDOW_DIALOG_CONTENT_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_FAVOURITES);
    m_gWindowManager.Delete(WINDOW_DIALOG_SONG_INFO);
    m_gWindowManager.Delete(WINDOW_DIALOG_SMART_PLAYLIST_EDITOR);
    m_gWindowManager.Delete(WINDOW_DIALOG_SMART_PLAYLIST_RULE);
    m_gWindowManager.Delete(WINDOW_DIALOG_BUSY);
    m_gWindowManager.Delete(WINDOW_DIALOG_PICTURE_INFO);
    m_gWindowManager.Delete(WINDOW_DIALOG_PLUGIN_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_ACCESS_POINTS);

    m_gWindowManager.Delete(WINDOW_STARTUP);
    m_gWindowManager.Delete(WINDOW_LOGIN_SCREEN);
    m_gWindowManager.Delete(WINDOW_VISUALISATION);
    m_gWindowManager.Delete(WINDOW_SETTINGS_MENU);
    m_gWindowManager.Delete(WINDOW_SETTINGS_PROFILES);
    m_gWindowManager.Delete(WINDOW_SETTINGS_MYPICTURES);  // all the settings categories
    m_gWindowManager.Delete(WINDOW_TEST_PATTERN);
    m_gWindowManager.Delete(WINDOW_SCREEN_CALIBRATION);
    m_gWindowManager.Delete(WINDOW_SYSTEM_INFORMATION);
    m_gWindowManager.Delete(WINDOW_SCREENSAVER);
    m_gWindowManager.Delete(WINDOW_OSD);
    m_gWindowManager.Delete(WINDOW_MUSIC_OVERLAY);
    m_gWindowManager.Delete(WINDOW_VIDEO_OVERLAY);
    m_gWindowManager.Delete(WINDOW_SCRIPTS_INFO);
    m_gWindowManager.Delete(WINDOW_SLIDESHOW);

    m_gWindowManager.Delete(WINDOW_HOME);
    m_gWindowManager.Delete(WINDOW_PROGRAMS);
    m_gWindowManager.Delete(WINDOW_PICTURES);
    m_gWindowManager.Delete(WINDOW_SCRIPTS);
    m_gWindowManager.Delete(WINDOW_GAMESAVES);
    m_gWindowManager.Delete(WINDOW_BUDDIES);
    m_gWindowManager.Delete(WINDOW_WEATHER);

    m_gWindowManager.Delete(WINDOW_SETTINGS_MYPICTURES);
    m_gWindowManager.Remove(WINDOW_SETTINGS_MYPROGRAMS);
    m_gWindowManager.Remove(WINDOW_SETTINGS_MYWEATHER);
    m_gWindowManager.Remove(WINDOW_SETTINGS_MYMUSIC);
    m_gWindowManager.Remove(WINDOW_SETTINGS_SYSTEM);
    m_gWindowManager.Remove(WINDOW_SETTINGS_MYVIDEOS);
    m_gWindowManager.Remove(WINDOW_SETTINGS_NETWORK);
    m_gWindowManager.Remove(WINDOW_SETTINGS_APPEARANCE);
    m_gWindowManager.Remove(WINDOW_DIALOG_KAI_TOAST);

    m_gWindowManager.Remove(WINDOW_DIALOG_SEEK_BAR);
    m_gWindowManager.Remove(WINDOW_DIALOG_VOLUME_BAR);

    CLog::Log(LOGNOTICE, "unload sections");
    CSectionLoader::UnloadAll();

#ifdef HAS_PERFORMANCE_SAMPLE
    CLog::Log(LOGNOTICE, "performance statistics");
    m_perfStats.DumpStats();
#endif

  // reset our d3d params before we destroy
#ifndef HAS_SDL
    g_graphicsContext.SetD3DDevice(NULL);
    g_graphicsContext.SetD3DParameters(NULL);
#endif

    //  Shutdown as much as possible of the
    //  application, to reduce the leaks dumped
    //  to the vc output window before calling
    //  _CrtDumpMemoryLeaks(). Most of the leaks
    //  shown are no real leaks, as parts of the app
    //  are still allocated.
    g_rssManager.Stop();
    g_localizeStrings.Clear();
    g_LangCodeExpander.Clear();
    g_charsetConverter.clear();
    g_directoryCache.Clear();
    g_buttonTranslator.Clear();
#ifdef HAS_KAI
    CKaiClient::RemoveInstance();
#endif
    CScrobbler::RemoveInstance();
    CLastFmManager::RemoveInstance();
#ifdef HAS_EVENT_SERVER
    CEventServer::RemoveInstance();
#endif
    DllLoaderContainer::Clear();
    g_playlistPlayer.Clear();
    g_settings.Clear();
    g_guiSettings.Clear();

#ifdef _LINUX
    CXHandle::DumpObjectTracker();
#endif

#ifdef _CRTDBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
    while(1); // execution ends
#endif
    return S_OK;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "Exception in CApplication::Cleanup()");
    return E_FAIL;
  }
}

void CApplication::Stop()
{
  try
  {
#ifdef __APPLE__
    
    // We've exited cleanly, so we can delete the crash file.
    CFile::Delete(CRASH_DETECTION_FILE);
    
    // Also, if we were supposed to restart the Plex Media Server, do so now.
    if (CPlexMediaServerPlayer::IsRestartRequired() == true &&
        g_guiSettings.GetBool("plexmediaserver.alwayson"))
    {
      PlexMediaServerHelper::Get().Restart();
    }
    
    // Shut down bonjour.
    Cocoa_StopLookingForRemotePlexSources();
    
    // Wait for any active source scanners.
    for (int i=0; CPlexSourceScanner::GetActiveScannerCount() != 0 && i<120; i++)
      Sleep(50);
    
#endif
    
    // Stop the texture manager.
    g_largeTextureManager.StopThread();

    CLog::Log(LOGNOTICE, "Storing total System Uptime");
    g_stSettings.m_iSystemTimeTotalUp = g_stSettings.m_iSystemTimeTotalUp + (int)(timeGetTime() / 60000);

    // Update the settings information (volume, uptime etc. need saving)
    if (CFile::Exists(g_settings.GetSettingsFile()))
    {
      CLog::Log(LOGNOTICE, "Saving settings");
      g_settings.Save();
    }
    else
      CLog::Log(LOGNOTICE, "Not saving settings (settings.xml is not present)");

    // Make sure background loader threads are all dead.
    CBackgroundRunner::StopAll();
    for (int i=0; CBackgroundRunner::GetNumActive() != 0 && i<120; i++)
    {
      m_applicationMessenger.ProcessMessages();
      Sleep(50);
    }
    
    m_bStop = true;
    CLog::Log(LOGNOTICE, "stop all");

#ifdef HAS_WEB_SERVER
    if (m_pXbmcHttp)
      getApplicationMessenger().HttpApi("broadcastlevel; ShutDown;1");
#endif

    StopServices();
    //Sleep(5000);

    StopPlaying();
    
    // Stop playing is asynchronous.
    if (m_pPlayer)
    {
      CLog::Log(LOGNOTICE, "stop mplayer");
      delete m_pPlayer;
      m_pPlayer = NULL;
    }

#if HAS_FILESYTEM_DAAP
    CLog::Log(LOGNOTICE, "stop daap clients");
    g_DaapClient.Release();
#endif
    //g_lcd->StopThread();
    m_applicationMessenger.Cleanup();

    CLog::Log(LOGNOTICE, "clean cached files!");
    g_RarManager.ClearCache(true);

    CLog::Log(LOGNOTICE, "unload skin");
    UnloadSkin();

#ifdef __APPLE__
    // Stop helpers.
    if (PlexRemoteHelper::Get().IsAlwaysOn() == false)
      PlexRemoteHelper::Get().Stop();
    if (PlexMediaServerHelper::Get().IsAlwaysOn() == false)
      PlexMediaServerHelper::Get().Stop();
    
    Cocoa_GL_UnblankOtherDisplays(Cocoa_GetCurrentDisplay());
    PlexMediaServerQueue::Get().StopThread();
#endif

/* Python resource freeing must be done after skin has been unloaded, not before
   some windows still need it when deinitializing during skin unloading. */
#ifdef HAS_PYTHON
  CLog::Log(LOGNOTICE, "stop python");
  g_pythonParser.FreeResources();
#endif
#ifdef HAS_LCD
    if (g_lcd)
    {
      g_lcd->Stop();
      delete g_lcd;
      g_lcd=NULL;
    }
#endif

    CLog::Log(LOGNOTICE, "stopped");
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "Exception in CApplication::Stop()");
  }

#if defined(_XBOX) || defined (_LINUX)
  //Both xbox and linux don't finish the run cycle but exit immediately after a call to g_application.Stop()
  //so they never get to Destroy() in CXBApplicationEx::Run(), we call it here.
  Destroy();
#endif

#ifdef __APPLE__
  // Restore system default audio device.
  if (m_defaultSystemDevice)
  {
    printf("Restoring system default of %s\n", m_defaultSystemDevice->getName().c_str());
    m_defaultSystemDevice->setDefault();
  }
#endif
}

bool CApplication::PlayMedia(const CFileItem& item, int iPlaylist)
{
  // if the GUI thread is creating the player then we do it in background in order not to block the gui
  if (GetCurrentThreadId() == g_application.GetThreadId())
  {
    CBackgroundPlayer *pBGPlayer = new CBackgroundPlayer(item, iPlaylist);
    pBGPlayer->Create(true); // will delete itself when done
  }
  else
    return PlayMediaSync(item, iPlaylist);

  return true;
}

bool CApplication::PlayMediaSync(const CFileItem& item, int iPlaylist)
{
  if (item.IsLastFM())
  {
    g_partyModeManager.Disable();
    return CLastFmManager::GetInstance()->ChangeStation(item.GetAsUrl());
  }
  if (item.IsSmartPlayList())
  {
    CDirectory dir;
    CFileItemList items;
    if (dir.GetDirectory(item.m_strPath, items) && items.Size())
    {
      CSmartPlaylist smartpl;
      //get name and type of smartplaylist, this will always succeed as GetDirectory also did this.
      smartpl.OpenAndReadName(item.m_strPath);
      CPlayList playlist;
      playlist.Add(items);
      return ProcessAndStartPlaylist(smartpl.GetName(), playlist, (smartpl.GetType() == "songs" || smartpl.GetType() == "albums") ? PLAYLIST_MUSIC:PLAYLIST_VIDEO);
    }
  }
  else if (item.IsPlayList() || item.IsInternetStream())
  {
#if 0
    CGUIDialogProgress* dlgProgress = (CGUIDialogProgress*)m_gWindowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
    if (item.IsInternetStream() && dlgProgress)
    {
       dlgProgress->ShowProgressBar(false);
       dlgProgress->SetHeading(260);
       dlgProgress->SetLine(0, 14003);
       dlgProgress->SetLine(1, "");
       dlgProgress->SetLine(2, "");
       dlgProgress->StartModal();
    }
#endif
    
    //is or could be a playlist
    auto_ptr<CPlayList> pPlayList (CPlayListFactory::Create(item));
    bool gotPlayList = (pPlayList.get() && pPlayList->Load(item.m_strPath));
    if (item.IsInternetStream() /*&& dlgProgress */)
    {
#if 0
       dlgProgress->Close();
       if (dlgProgress->IsCanceled())
          return true;
#endif
    }

    if (gotPlayList)
    {

      if (iPlaylist != PLAYLIST_NONE)
        return ProcessAndStartPlaylist(item.m_strPath, *pPlayList, iPlaylist);
      else
      {
        CLog::Log(LOGWARNING, "CApplication::PlayMedia called to play a playlist %s but no idea which playlist to use, playing first item", item.m_strPath.c_str());
        if(pPlayList->size())
          return PlayFile(*(*pPlayList)[0], false);
      }
    }
  }

  //nothing special just play
  return PlayFile(item, false);
}

// PlayStack()
// For playing a multi-file video.  Particularly inefficient
// on startup, as we are required to calculate the length
// of each video, so we open + close each one in turn.
// A faster calculation of video time would improve this
// substantially.
bool CApplication::PlayStack(const CFileItem& item, bool bRestart)
{
  if (!item.IsStack())
    return false;

  // see if we have the info in the database
  // TODO: If user changes the time speed (FPS via framerate conversion stuff)
  //       then these times will be wrong.
  //       Also, this is really just a hack for the slow load up times we have
  //       A much better solution is a fast reader of FPS and fileLength
  //       that we can use on a file to get it's time.
  vector<long> times;
  bool haveTimes(false);

  CVideoDatabase dbs;
  if (dbs.Open())
  {
    dbs.GetVideoSettings(item.m_strPath, g_stSettings.m_currentVideoSettings);
    haveTimes = dbs.GetStackTimes(item.m_strPath, times);
    dbs.Close();
  }
  
  // calculate the total time of the stack
  CStackDirectory dir;
  dir.GetDirectory(item.m_strPath, *m_currentStack);
  
  // Move local paths in.
  if (item.HasProperty("localPath"))
  {
    CFileItemList stackedItems;
    dir.GetDirectory(item.GetProperty("localPath"), stackedItems);
    for (int i=0; i<stackedItems.Size(); i++)
      (*m_currentStack)[i]->SetProperty("localPath", stackedItems[i]->m_strPath);
  }
  
  long totalTime = 0;
  for (int i = 0; i < m_currentStack->Size(); i++)
  {
    if (haveTimes)
      (*m_currentStack)[i]->m_lEndOffset = times[i];
    else
    {
      int duration;
      
      // Prefer local path.
      string path = (*m_currentStack)[i]->m_strPath;
      string localPath = (*m_currentStack)[i]->GetProperty("localPath");
      if (CFile::Exists(localPath))
        path = localPath;
      
      if (!CDVDFileInfo::GetFileDuration(path, duration))
      {
        m_currentStack->Clear();
        return false;
      }
      totalTime += duration / 1000;
      (*m_currentStack)[i]->m_lEndOffset = totalTime;
      times.push_back(totalTime);
    }
  }

  double seconds = item.m_lStartOffset / 75.0;

  if (!haveTimes || item.m_lStartOffset == STARTOFFSET_RESUME)
  { 
    // See if we have a view offset.
    if (item.HasProperty("viewOffset"))
      seconds = boost::lexical_cast<int>(item.GetProperty("viewOffset"))/1000.0;
    else
      seconds = 0.0f;
  }

  m_bPlaybackStarting = true;
  if (m_pPlayer) SAFE_DELETE(m_pPlayer);
  *m_itemCurrentFile = item;
  m_currentStackPosition = 0;
  m_eCurrentPlayer = EPC_NONE; // must be reset on initial play otherwise last player will be used 

  if (seconds > 0)
  {
    // work out where to seek to
    for (int i = 0; i < m_currentStack->Size(); i++)
    {
      if (seconds < (*m_currentStack)[i]->m_lEndOffset)
      {
        CFileItem item(*(*m_currentStack)[i]);
        long start = (i > 0) ? (*m_currentStack)[i-1]->m_lEndOffset : 0;
        item.m_lStartOffset = (long)(seconds - start) * 75;
        m_currentStackPosition = i;
        return PlayFile(item, true);
      }
    }
  }

  return PlayFile(*(*m_currentStack)[0], true);
}

bool CApplication::PlayFile(const CFileItem& item, bool bRestart)
{
  if (!bRestart)
  {
    SaveCurrentFileSettings();

    OutputDebugString("new file set audiostream:0\n");
    // Switch to default options
    g_stSettings.m_currentVideoSettings = g_stSettings.m_defaultVideoSettings;
    // see if we have saved options in the database

    m_iPlaySpeed = 1;
    *m_itemCurrentFile = item;

    m_nextPlaylistItem = -1;
    m_currentStackPosition = 0;
    m_currentStack->Clear();
  }

  if (item.IsPlayList())
    return false;

  // if we have a stacked set of files, we need to setup our stack routines for
  // "seamless" seeking and total time of the movie etc.
  // will recall with restart set to true
  if (item.IsStack())
    return PlayStack(item, bRestart);

  //Is TuxBox, this should probably be moved to CFileTuxBox
  if(item.IsTuxBox())
  {
    CLog::Log(LOGDEBUG, "%s - TuxBox URL Detected %s",__FUNCTION__, item.m_strPath.c_str());

    if(g_tuxboxService.IsRunning())
      g_tuxboxService.Stop();

    CFileItem item_new;
    if(g_tuxbox.CreateNewItem(item, item_new))
    {

      // Make sure it doesn't have a player
      // so we actually select one normally
      m_eCurrentPlayer = EPC_NONE;

      // keep the tuxbox:// url as playing url
      // and give the new url to the player
      if(PlayFile(item_new, true))
      {
        if(!g_tuxboxService.IsRunning())
          g_tuxboxService.Start();
        return true;
      }
    }
    return false;
  }

  CPlayerOptions options;
  EPLAYERCORES eNewCore = EPC_NONE;
  if( bRestart )
  {
    // have to be set here due to playstack using this for starting the file
    options.starttime = item.m_lStartOffset / 75.0;
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0 && m_itemCurrentFile->m_lStartOffset != 0)
      m_itemCurrentFile->m_lStartOffset = STARTOFFSET_RESUME; // to force fullscreen switching

    if( m_eForcedNextPlayer != EPC_NONE )
      eNewCore = m_eForcedNextPlayer;
    else if( m_eCurrentPlayer == EPC_NONE )
      eNewCore = CPlayerCoreFactory::GetDefaultPlayer(item);
    else
      eNewCore = m_eCurrentPlayer;
  }
  else
  {
    options.starttime = item.m_lStartOffset / 75.0;

    if (item.m_lStartOffset == STARTOFFSET_RESUME)
    {
      // See if we have a view offset.
      if (item.HasProperty("viewOffset"))
        options.starttime = boost::lexical_cast<int>(item.GetProperty("viewOffset"))/1000.0;
    }

    if (m_eForcedNextPlayer != EPC_NONE)
      eNewCore = m_eForcedNextPlayer;
    else
      eNewCore = CPlayerCoreFactory::GetDefaultPlayer(item);
  }

  // this really ought to be inside !bRestart, but since PlayStack
  // uses that to init playback, we have to keep it outside
  //
  int playlist = g_playlistPlayer.GetCurrentPlaylist();
  if (playlist == PLAYLIST_VIDEO && g_playlistPlayer.GetPlaylist(playlist).size() > 1)
  { // playing from a playlist by the looks
    // don't switch to fullscreen if we are not playing the first item...
    options.fullscreen = !g_playlistPlayer.HasPlayedFirstFile();
  }
  else if(m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
  {
    // TODO - this will fail if user seeks back to first file in stack
    if(m_currentStackPosition == 0
    || m_itemCurrentFile->m_lStartOffset == STARTOFFSET_RESUME)
      options.fullscreen = true;
    else
      options.fullscreen = false;
    // reset this so we don't think we are resuming on seek
    m_itemCurrentFile->m_lStartOffset = 0;
  }
  else
    options.fullscreen = true;

  // reset any forced player
  m_eForcedNextPlayer = EPC_NONE;

#ifdef HAS_KARAOKE
  //We have to stop parsing a cdg before mplayer is deallocated
  // WHY do we have to do this????
  if(m_pCdgParser)
    m_pCdgParser->Stop();
#endif

  // tell system we are starting a file
  m_bPlaybackStarting = true;
  m_bPlaybackInFullScreen = options.fullscreen;

  // We should restart the player, unless the previous and next tracks are using
  // one of the players that allows gapless playback (paplayer, dvdplayer)
  if (m_pPlayer)
  {
    if ( !(m_eCurrentPlayer == eNewCore && (m_eCurrentPlayer == EPC_DVDPLAYER || m_eCurrentPlayer  == EPC_PAPLAYER || m_eCurrentPlayer == EPC_PMSPLAYER)) )
    {
      delete m_pPlayer;
      m_pPlayer = NULL;
    }
  }

  if (!m_pPlayer)
  {
    m_eCurrentPlayer = eNewCore;
    m_pPlayer = CPlayerCoreFactory::CreatePlayer(eNewCore, *this);
  }

  bool bResult;
  if (m_pPlayer)
  {
#ifdef __APPLE__
    // Suspend the updater
    Cocoa_SetUpdateSuspended(true);

    // Stop the background music (if enabled)
    m_bBackgroundMusicEnabled = Cocoa_IsBackgroundMusicEnabled();
    Cocoa_SetBackgroundMusicEnabled(false);
#endif
    
    bResult = m_pPlayer->OpenFile(item, options);
  }
  else
  {
    CLog::Log(LOGERROR, "Error creating player for item %s (File doesn't exist?)", item.m_strPath.c_str());
    bResult = false;
  }

  // If the player is opening asynchronously, we'll finish up with a callback.
  // Otherwise complete the open synchronously.
  //
  if (m_pPlayer->CanOpenAsync() == false)
    FinishPlayingFile(bResult);

  return bResult;
}

void CApplication::FinishPlayingFile(bool bResult, const CStdString& error)
{
  if (bResult)
  {
    if (m_iPlaySpeed != 1)
    {
      int iSpeed = m_iPlaySpeed;
      m_iPlaySpeed = 1;
      SetPlaySpeed(iSpeed);
    }

#ifdef HAS_VIDEO_PLAYBACK
    if( IsPlayingVideo() )
    {
      // if player didn't manange to switch to fullscreen by itself do it here
      if(m_bPlaybackInFullScreen && g_renderManager.IsStarted()
       && m_gWindowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO )
       SwitchToFullScreen();
    }
#endif

    if (!g_guiSettings.GetBool("lookandfeel.soundsduringplayback"))
      g_audioManager.Enable(false);
  }

  if(!bResult || !IsPlaying())
  {
    // Display error message.
    if (g_playlistPlayer.GetPlaylist(g_playlistPlayer.GetCurrentPlaylist()).size() == 1)
    {
      CStdString err = error;
      if (err.size() == 0)
        err = g_localizeStrings.Get(42008);
        
      CGUIDialogOK::ShowAndGetInput(g_localizeStrings.Get(257), err + ".", "", "");
    }
    
    // Since we didn't manage to get playback started, send any queued up messages
    while(m_vPlaybackStarting.size())
    {
      m_gWindowManager.SendMessage(m_vPlaybackStarting.front());
      m_vPlaybackStarting.pop();
    }
  }

  // If we're supposed to activate the visualizer when playing audio, do so now.
  if (IsPlayingAudio() && 
      g_advancedSettings.m_bVisualizerOnPlay &&
      !g_playlistPlayer.HasPlayedFirstFile() && 
      !g_playlistPlayer.QueuedFirstFile())
  {
    ActivateVisualizer();
  }

  while(m_vPlaybackStarting.size()) m_vPlaybackStarting.pop();
  m_bPlaybackStarting = false;  
}

void CApplication::OnPlayBackEnded()
{
  //playback ended
  SetPlaySpeed(1);

  // informs python script currently running playback has ended
  // (does nothing if python is not loaded)
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackEnded();
#endif

#ifdef HAS_WEB_SERVER
  // Let's tell the outside world as well
  if (m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=1)
    getApplicationMessenger().HttpApi("broadcastlevel; OnPlayBackEnded;1");
#endif

  CLog::Log(LOGDEBUG, "Playback has finished");
  
  CGUIMessage msg(GUI_MSG_PLAYBACK_ENDED, 0, 0);

  if(m_bPlaybackStarting)
    m_vPlaybackStarting.push(msg);
  else
    m_gWindowManager.SendThreadMessage(msg);
}

void CApplication::OnPlayBackStarted()
{
  // Reset to the claimed video FPS.
  g_infoManager.ResetFPS(m_pPlayer->GetActualFPS());
  
#ifdef HAS_PYTHON
  // informs python script currently running playback has started
  // (does nothing if python is not loaded)
  g_pythonParser.OnPlayBackStarted();
#endif

#ifdef HAS_WEB_SERVER
  // Let's tell the outside world as well
  if (m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=1)
    getApplicationMessenger().HttpApi("broadcastlevel; OnPlayBackStarted;1");
#endif
  
  CLog::Log(LOGDEBUG, "Playback has started");

  CGUIMessage msg(GUI_MSG_PLAYBACK_STARTED, 0, 0);
  m_gWindowManager.SendThreadMessage(msg);

#ifdef HAS_XBOX_HARDWARE
  CheckNetworkHDSpinDown(true);

  StartLEDControl(true);
#endif
  DimLCDOnPlayback(true);
}

void CApplication::OnQueueNextItem()
{
  // informs python script currently running that we are requesting the next track
  // (does nothing if python is not loaded)
#ifdef HAS_PYTHON
  g_pythonParser.OnQueueNextItem(); // currently unimplemented
#endif

#ifdef HAS_WEB_SERVER
  // Let's tell the outside world as well
  if (m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=1)
    getApplicationMessenger().HttpApi("broadcastlevel; OnQueueNextItem;1");
#endif
  CLog::Log(LOGDEBUG, "Player has asked for the next item");

  CGUIMessage msg(GUI_MSG_QUEUE_NEXT_ITEM, 0, 0);
  m_gWindowManager.SendThreadMessage(msg);
}

void CApplication::OnPlayBackStopped()
{
  // Re-enable sounds.
  g_audioManager.Enable(true);

  // Reset FPS to the display FPS. 
  g_infoManager.ResetFPS(g_graphicsContext.GetFPS());
  
  // informs python script currently running playback has ended
  // (does nothing if python is not loaded)
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackStopped();
#endif

#ifdef HAS_WEB_SERVER
  // Let's tell the outside world as well
  if (m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=1)
    getApplicationMessenger().HttpApi("broadcastlevel; OnPlayBackStopped;1");
#endif

  CLog::Log(LOGDEBUG, "Playback was stopped\n");

  CGUIMessage msg( GUI_MSG_PLAYBACK_STOPPED, 0, 0 );
  if(m_bPlaybackStarting)
    m_vPlaybackStarting.push(msg);
  else
    m_gWindowManager.SendThreadMessage(msg);
}

bool CApplication::IsPlaying() const
{
  if (!m_pPlayer)
    return false;
  if (!m_pPlayer->IsPlaying())
    return false;
  return true;
}

bool CApplication::IsPaused() const
{
  if (!m_pPlayer)
    return false;
  if (!m_pPlayer->IsPlaying())
    return false;
  return m_pPlayer->IsPaused();
}

bool CApplication::IsPlayingAudio() const
{
  if (!m_pPlayer)
    return false;
  if (!m_pPlayer->IsPlaying())
    return false;
  if (m_pPlayer->HasVideo())
    return false;
  if (m_pPlayer->HasAudio())
    return true;

  return false;
}

bool CApplication::IsPlayingVideo() const
{
  if (!m_pPlayer)
    return false;
  if (!m_pPlayer->IsPlaying())
    return false;
  if (m_pPlayer->HasVideo())
    return true;

  return false;
}

bool CApplication::IsPlayingFullScreenVideo() const
{
  return IsPlayingVideo() && g_graphicsContext.IsFullScreenVideo();
}

void CApplication::StopPlaying()
{
  if ( IsPlaying() )
  {
    UpdateFileState(true);
    
#ifdef HAS_KARAOKE
    if( m_pCdgParser )
      m_pCdgParser->Stop();
#endif

    // turn off visualisation window when stopping
    if (IsVisualizerActive())
      m_gWindowManager.PreviousWindow();
    
    m_pPlayer->CloseFile();
    g_partyModeManager.Disable();
    OnPlayBackStopped();
  }
}

bool CApplication::NeedRenderFullScreen()
{
  if (m_gWindowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
  {
    m_gWindowManager.UpdateModelessVisibility();

    if (m_gWindowManager.HasDialogOnScreen()) return true;
    if (g_Mouse.IsActive() && g_Mouse.IsEnabled()) return true;

    CGUIWindowFullScreen *pFSWin = (CGUIWindowFullScreen *)m_gWindowManager.GetWindow(WINDOW_FULLSCREEN_VIDEO);
    if (!pFSWin)
      return false;
    return pFSWin->NeedRenderFullScreen();
  }
  return false;
}

void CApplication::RenderFullScreen()
{
  MEASURE_FUNCTION;

  g_ApplicationRenderer.Render(true);
}

void CApplication::DoRenderFullScreen()
{
  MEASURE_FUNCTION;

  if (g_graphicsContext.IsFullScreenVideo())
  {
    // make sure our overlays are closed
    CGUIDialog *overlay = (CGUIDialog *)m_gWindowManager.GetWindow(WINDOW_VIDEO_OVERLAY);
    if (overlay) overlay->Close(true);
    overlay = (CGUIDialog *)m_gWindowManager.GetWindow(WINDOW_MUSIC_OVERLAY);
    if (overlay) overlay->Close(true);

    CGUIWindowFullScreen *pFSWin = (CGUIWindowFullScreen *)m_gWindowManager.GetWindow(WINDOW_FULLSCREEN_VIDEO);
    if (!pFSWin)
      return ;
    pFSWin->RenderFullScreen();

    if (m_gWindowManager.HasDialogOnScreen())
      m_gWindowManager.RenderDialogs();
    // Render the mouse pointer, if visible...
    if (g_Mouse.IsActive() && g_Mouse.IsEnabled())
      g_application.m_guiPointer.Render();
  }
}

void CApplication::ResetScreenSaver()
{
  if (m_bInactive && !m_bScreenSave && m_iScreenSaveLock == 0)
  {
    m_dwShutdownTick = m_dwSaverTick = timeGetTime(); // Start the timer going ...
  }

#ifdef __APPLE__
  if (m_bDisplaySleeping)
  {
    m_bInactive = false;
    m_dwShutdownTick = m_dwSaverTick = timeGetTime(); // Start the timer going ...
    m_bDisplaySleeping = false;
  }
#endif
}

bool CApplication::ResetScreenSaverWindow()
{
  if (m_iScreenSaveLock == 2)
    return false;

  m_bInactive = false;  // reset the inactive flag as a key has been pressed

  // if Screen saver is active
  if (m_bScreenSave)
  {
    int iProfile = g_settings.m_iLastLoadedProfileIndex;
    if (m_iScreenSaveLock == 0)
      if (g_guiSettings.GetBool("screensaver.uselock")                           &&
          g_settings.m_vecProfiles[0].getLockMode() != LOCK_MODE_EVERYONE        &&
          g_settings.m_vecProfiles[iProfile].getLockMode() != LOCK_MODE_EVERYONE &&
         !g_guiSettings.GetString("screensaver.mode").Equals("Black")            &&
        !(g_guiSettings.GetBool("screensaver.usemusicvisinstead")                &&
         !g_guiSettings.GetString("screensaver.mode").Equals("Black")            &&
          g_application.IsPlayingAudio())                                          )
      {
        m_iScreenSaveLock = 2;
        CGUIMessage msg(GUI_MSG_CHECK_LOCK,0,0);
        m_gWindowManager.GetWindow(WINDOW_SCREENSAVER)->OnMessage(msg);
      }
    if (m_iScreenSaveLock == -1)
    {
      m_iScreenSaveLock = 0;
      return true;
    }

    // disable screensaver
    m_bScreenSave = false;
    m_iScreenSaveLock = 0;

    float fFadeLevel = 1.0f;
    if (m_screenSaverMode == "Visualisation" || m_screenSaverMode == "Slideshow" || m_screenSaverMode == "Fanart Slideshow")
    {
      // we can just continue as usual from vis mode
      return false;
    }
    else if (m_screenSaverMode == "Dim")
    {
      fFadeLevel = (float)g_guiSettings.GetInt("screensaver.dimlevel") / 100;
    }
    else if (m_screenSaverMode == "Black")
    {
      fFadeLevel = 0;
    }
    else if (m_screenSaverMode != "None")
    { // we're in screensaver window
      if (m_gWindowManager.GetActiveWindow() == WINDOW_SCREENSAVER)
        m_gWindowManager.PreviousWindow();  // show the previous window
      return true;
    }

    // Fade to dim or black screensaver is active --> fade in
#ifndef HAS_SDL
    D3DGAMMARAMP Ramp;
    for (float fade = fFadeLevel; fade <= 1; fade += 0.01f)
    {
      for (int i = 0;i < 256;i++)
      {
        Ramp.red[i] = (int)((float)m_OldRamp.red[i] * fade);
        Ramp.green[i] = (int)((float)m_OldRamp.green[i] * fade);
        Ramp.blue[i] = (int)((float)m_OldRamp.blue[i] * fade);
      }
      Sleep(5);
      m_pd3dDevice->SetGammaRamp(GAMMA_RAMP_FLAG, &Ramp); // use immediate to get a smooth fade
    }
    m_pd3dDevice->SetGammaRamp(0, &m_OldRamp); // put the old gamma ramp back in place
#else

   if (g_advancedSettings.m_fullScreen == true)
   {
     Uint16 RampRed[256];
     Uint16 RampGreen[256];
     Uint16 RampBlue[256];
     for (float fade = fFadeLevel; fade <= 1; fade += 0.01f)
     {
       for (int i = 0;i < 256;i++)
       {
         RampRed[i] = (Uint16)((float)m_OldRampRed[i] * fade);
         RampGreen[i] = (Uint16)((float)m_OldRampGreen[i] * fade);
         RampBlue[i] = (Uint16)((float)m_OldRampBlue[i] * fade);
       }
       Sleep(5);
#ifdef __APPLE__
       Cocoa_SetGammaRamp(RampRed, RampGreen, RampBlue);
#else
       SDL_SetGammaRamp(RampRed, RampGreen, RampBlue);
#endif
     }
#ifdef __APPLE__
     Cocoa_SetGammaRamp(RampRed, RampGreen, RampBlue);
#else
     SDL_SetGammaRamp(m_OldRampRed, m_OldRampGreen, m_OldRampBlue);
#endif
   }
#endif
    return true;
  }
  else
    return false;
}

void CApplication::CheckActive()
{
  if ((IsPlayingVideo() && !IsPaused()) || 
      m_gWindowManager.IsWindowActive(WINDOW_SCREENSAVER) || 
      (IsVisualizerActive() && !IsPaused()) || 
      m_gWindowManager.IsWindowActive(WINDOW_DIALOG_PROGRESS))
  {
    m_bInactive = false;
  }
  else if (!m_bInactive)
  {
    // Start the timer going ...
    m_dwShutdownTick = m_dwSaverTick = timeGetTime();
    m_bInactive = true;
  }
}

bool CApplication::IsVisualizerActive()
{
  return (m_gWindowManager.GetActiveWindow() == WINDOW_VISUALISATION || 
          m_gWindowManager.GetActiveWindow() == WINDOW_NOW_PLAYING);
}

void CApplication::CheckScreenSaver()
{
  if (!m_bInactive || m_bScreenSave || m_gWindowManager.IsWindowActive(WINDOW_SCREENSAVER) || g_guiSettings.GetString("screensaver.mode") == "None" || g_guiSettings.GetInt("screensaver.time") <= 0)
    return;

  
  // How long to screensaver? The setting, unless we're playing music, in which case much quicker.
  long timeToScreenSaver = g_guiSettings.GetInt("screensaver.time")*60*1000L;
  
  if (IsPlayingAudio())
    timeToScreenSaver = MIN(g_advancedSettings.m_secondsToVisualizer*1000L, timeToScreenSaver);
  
  if (timeGetTime() - m_dwSaverTick >= timeToScreenSaver)
    ActivateScreenSaver();
}

void CApplication::ActivateVisualizer()
{
  m_screenSaverMode = "Visualisation";
  
  // See which visualizer to activate.
  if (g_guiSettings.GetString("mymusic.visualisation") == "Now Playing.vis")
    m_gWindowManager.ActivateWindow(WINDOW_NOW_PLAYING);
  else
    m_gWindowManager.ActivateWindow(WINDOW_VISUALISATION);
}

// activate the screensaver.
// if forceType is true, we ignore the various conditions that can alter
// the type of screensaver displayed
void CApplication::ActivateScreenSaver(bool forceType /*= false */)
{
  FLOAT fFadeLevel = 0;

  m_bScreenSave = true;
  m_bInactive = true;
  m_dwShutdownTick = m_dwSaverTick = timeGetTime();  // Save the current time for the shutdown timeout

  // Get Screensaver Mode
  m_screenSaverMode = g_guiSettings.GetString("screensaver.mode");

  if (!forceType)
  {
    // set to Dim in the case of a dialog on screen or playing video
    if (m_gWindowManager.HasModalDialog() || (IsPlayingVideo() && g_guiSettings.GetBool("screensaver.usedimonpause")))
      m_screenSaverMode = "Dim";
    // Check if we are Playing Audio and Vis instead Screensaver!
    else if (IsPlayingAudio() && g_guiSettings.GetBool("screensaver.usemusicvisinstead") && g_guiSettings.GetString("mymusic.visualisation") != "None")
    { // activate the visualisation
      ActivateVisualizer();
      return;
    }
  }
  // Picture slideshow
  if (m_screenSaverMode == "SlideShow" || m_screenSaverMode == "Fanart Slideshow")
  {
    // reset our codec info - don't want that on screen
    g_infoManager.SetShowCodec(false);
    m_applicationMessenger.PictureSlideShow(g_guiSettings.GetString("screensaver.slideshowpath"), true);
    return;
  }
  else if (m_screenSaverMode == "Dim")
  {
    fFadeLevel = (FLOAT) g_guiSettings.GetInt("screensaver.dimlevel") / 100; // 0.07f;
  }
  else if (m_screenSaverMode == "Black")
  {
    fFadeLevel = 0;
  }
  else if (m_screenSaverMode != "None")
  {
    m_gWindowManager.ActivateWindow(WINDOW_SCREENSAVER);
    return ;
  }

  // Fade to fFadeLevel
#ifndef HAS_SDL
  D3DGAMMARAMP Ramp;
  m_pd3dDevice->GetGammaRamp(&m_OldRamp); // Store the old gamma ramp
  for (float fade = 1.f; fade >= fFadeLevel; fade -= 0.01f)
  {
    for (int i = 0;i < 256;i++)
    {
      Ramp.red[i] = (int)((float)m_OldRamp.red[i] * fade);
      Ramp.green[i] = (int)((float)m_OldRamp.green[i] * fade);
      Ramp.blue[i] = (int)((float)m_OldRamp.blue[i] * fade);
    }
    Sleep(5);
    m_pd3dDevice->SetGammaRamp(GAMMA_RAMP_FLAG, &Ramp); // use immediate to get a smooth fade
  }
#else
  if (g_advancedSettings.m_fullScreen == true)
  {
    SDL_GetGammaRamp(m_OldRampRed, m_OldRampGreen, m_OldRampBlue); // Store the old gamma ramp
    Uint16 RampRed[256];
    Uint16 RampGreen[256];
    Uint16 RampBlue[256];
    for (float fade = 1.f; fade >= fFadeLevel; fade -= 0.01f)
    {
      for (int i = 0;i < 256;i++)
      {
        RampRed[i] = (Uint16)((float)m_OldRampRed[i] * fade);
        RampGreen[i] = (Uint16)((float)m_OldRampGreen[i] * fade);
        RampBlue[i] = (Uint16)((float)m_OldRampBlue[i] * fade);
      }
      Sleep(5);
#ifdef __APPLE__
      Cocoa_SetGammaRamp(RampRed, RampGreen, RampBlue);
#else
      SDL_SetGammaRamp(RampRed, RampGreen, RampBlue);
#endif
    }
  }
#endif
}
  
#ifdef __APPLE__
void CApplication::CheckForUpdates()
{
  Cocoa_CheckForUpdates();
}
#endif

void CApplication::CheckShutdown()
{
#if defined(HAS_XBOX_HARDWARE) || defined(__APPLE__)

  if (IsPlayingAudio() && !IsPlayingVideo() && !IsPaused())
  {
    m_dwShutdownTick = timeGetTime();
    return;
  }

  if (!m_bInactive || g_guiSettings.GetInt("energy.shutdowntime") <= 0)
    return;

#ifdef HAS_FTP_SERVER
  // Don't shut down if we have active ftp connections, or we are scanning for music or video
  if (m_pFileZilla && m_pFileZilla->GetNoConnections() != 0) // is FTP active ?
    return;
#endif

  if ((timeGetTime() - m_dwShutdownTick) >= g_guiSettings.GetInt("energy.shutdowntime")*60*1000L)
  {
#ifdef __APPLE__
    // For Apple it's a sleep not a shutdown.
    SleepSystem();
#else
     // Turn off the box
    m_applicationMessenger.Shutdown();
#endif
  }
#endif
}

void CApplication::SleepSystem()
{
  if (m_bSystemSleeping == false)
  {
    // Request a sleep. This is an asynchronous request.
    CLog::Log(LOGDEBUG, "Sleeping system.");
    Cocoa_SleepSystem();
  }
}

void CApplication::SystemWillSleep()
{
  // We're actually going to sleep now!
  CLog::Log(LOGDEBUG, "System is going to sleep.");
  m_bSystemSleeping = true;
}

void CApplication::SystemWokeUp()
{
  // Set everything to reset when we wake up.
  CLog::Log(LOGDEBUG, "System woke up.");
  m_dwShutdownTick = m_dwSaverTick = timeGetTime();
  m_bInactive = false;
  m_bDisplaySleeping = true;
  m_bSystemSleeping = false;
}

void CApplication::ResetDisplaySleep()
{
  // TODO: Clean up m_idleTimer use
  m_idleTimer.StartZero();
  ResetScreenSaver();
  ResetScreenSaverWindow();
}

void CApplication::CheckDisplaySleep()
{
#ifdef __APPLE__
  if (!m_bInactive || m_bDisplaySleeping || (g_guiSettings.GetInt("energy.displaysleeptime") <= 0) ||
      // If the display timer is set to the same time or longer than the shutdown timer, then there is no need to dim the screen.
      ((g_guiSettings.GetInt("energy.shutdowntime") > 0) && (g_guiSettings.GetInt("energy.displaysleeptime") >= g_guiSettings.GetInt("energy.shutdowntime"))))
    return;

  if ((timeGetTime() - m_dwSaverTick) >= (g_guiSettings.GetInt("energy.displaysleeptime")*60*1000L))
  {
    CLog::Log(LOGDEBUG, "Dimming display.");
    Cocoa_DimDisplayNow();
    m_bDisplaySleeping = true;
  }
#endif
}

#ifdef HAS_XBOX_HARDWARE
//Check if hd spindown must be blocked
bool CApplication::MustBlockHDSpinDown(bool bCheckThisForNormalSpinDown)
{
  if (IsPlayingVideo())
  {
    //block immediate spindown when playing a video non-fullscreen (videocontrol is playing)
    if ((!bCheckThisForNormalSpinDown) && (!g_graphicsContext.IsFullScreenVideo()))
    {
      return true;
    }
    //allow normal hd spindown always if the movie is paused
    if ((bCheckThisForNormalSpinDown) && (m_pPlayer->IsPaused()))
    {
      return false;
    }
    //don't allow hd spindown when playing files with vobsub subtitles.
    CStdString strSubTitelExtension;
    if (m_pPlayer->GetSubtitleExtension(strSubTitelExtension))
    {
      return (strSubTitelExtension == ".idx");
    }
  }
  return false;
}

void CApplication::CheckNetworkHDSpinDown(bool playbackStarted)
{
  int iSpinDown = g_guiSettings.GetInt("harddisk.remoteplayspindown");
  if (iSpinDown == SPIN_DOWN_NONE)
    return ;
  if (m_gWindowManager.HasModalDialog())
    return ;
  if (MustBlockHDSpinDown(false))
    return ;

  if ((!m_bNetworkSpinDown) || playbackStarted)
  {
    int iDuration = 0;
    if (IsPlayingAudio())
    {
      //try to get duration from current tag because mplayer doesn't calculate vbr mp3 correctly
      if (m_itemCurrentFile->HasMusicInfoTag())
        iDuration = m_itemCurrentFile->GetMusicInfoTag()->GetDuration();
    }
    if (IsPlaying() && iDuration <= 0)
    {
      iDuration = (int)GetTotalTime();
    }
    //spin down harddisk when the current file being played is not on local harddrive and
    //duration is more then spindown timeoutsetting or duration is unknown (streams)
    if (
      !m_itemCurrentFile->IsHD() &&
      (
        (iSpinDown == SPIN_DOWN_VIDEO && IsPlayingVideo()) ||
        (iSpinDown == SPIN_DOWN_MUSIC && IsPlayingAudio()) ||
        (iSpinDown == SPIN_DOWN_BOTH && (IsPlayingVideo() || IsPlayingAudio()))
      ) &&
      (
        (iDuration <= 0) ||
        (iDuration > g_guiSettings.GetInt("harddisk.remoteplayhdspindownminduration")*60)
      )
    )
    {
      m_bNetworkSpinDown = true;
      if (!playbackStarted)
      { //if we got here not because of a playback start check what screen we are in
        // get the current active window
        int iWin = m_gWindowManager.GetActiveWindow();
        if (iWin == WINDOW_FULLSCREEN_VIDEO)
        {
          // check if OSD is visible, if so don't do immediate spindown
          CGUIWindowOSD *pOSD = (CGUIWindowOSD *)m_gWindowManager.GetWindow(WINDOW_OSD);
          if (pOSD)
            m_bNetworkSpinDown = !pOSD->IsDialogRunning();
        }
      }
      if (m_bNetworkSpinDown)
      {
        //do the spindown right now + delayseconds
        m_dwSpinDownTime = timeGetTime();
      }
    }
  }
  if (m_bNetworkSpinDown)
  {
    // check the elapsed time
    DWORD dwTimeSpan = timeGetTime() - m_dwSpinDownTime;
    if ( (m_dwSpinDownTime != 0) && (dwTimeSpan >= ((DWORD)g_guiSettings.GetInt("harddisk.remoteplayspindowndelay")*1000UL)) )
    {
      // time has elapsed, spin it down
#ifdef HAS_XBOX_HARDWARE
      XKHDD::SpindownHarddisk();
#endif
      //stop checking until a key is pressed.
      m_dwSpinDownTime = 0;
      m_bNetworkSpinDown = true;
    }
    else if (m_dwSpinDownTime == 0 && IsPlaying())
    {
      // we are currently spun down - let's spin back up again if we are playing media
      // and we're within 10 seconds (or 0.5*spindown time) of the end.  This should
      // make returning to the GUI a bit snappier + speed up stacked item changes.
      int iMinSpinUp = 10;
      if (iMinSpinUp > g_guiSettings.GetInt("harddisk.remoteplayspindowndelay")*0.5f)
        iMinSpinUp = (int)(g_guiSettings.GetInt("harddisk.remoteplayspindowndelay")*0.5f);
      if (g_infoManager.GetPlayTimeRemaining() == iMinSpinUp)
      { // spin back up
#ifdef HAS_XBOX_HARDWARE
        XKHDD::SpindownHarddisk(false);
#endif
      }
    }
  }
}

void CApplication::CheckHDSpindown()
{
  if (!g_guiSettings.GetInt("harddisk.spindowntime"))
    return ;
  if (m_gWindowManager.HasModalDialog())
    return ;
  if (MustBlockHDSpinDown())
    return ;

  if (!m_bSpinDown &&
      (
        !IsPlaying() ||
        (IsPlaying() && !m_itemCurrentFile->IsHD())
      )
     )
  {
    m_bSpinDown = true;
    m_bNetworkSpinDown = false; // let networkspindown override normal spindown
    m_dwSpinDownTime = timeGetTime();
  }

  //Can we do a spindown right now?
  if (m_bSpinDown)
  {
    // yes, then check the elapsed time
    DWORD dwTimeSpan = timeGetTime() - m_dwSpinDownTime;
    if ( (m_dwSpinDownTime != 0) && (dwTimeSpan >= ((DWORD)g_guiSettings.GetInt("harddisk.spindowntime")*60UL*1000UL)) )
    {
      // time has elapsed, spin it down
#ifdef HAS_XBOX_HARDWARE
      XKHDD::SpindownHarddisk();
#endif
      //stop checking until a key is pressed.
      m_dwSpinDownTime = 0;
      m_bSpinDown = true;
    }
  }
}
#endif

bool CApplication::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_NOTIFY_ALL:
    {
      if (message.GetParam1()==GUI_MSG_REMOVED_MEDIA)
      {
        // Update general playlist: Remove DVD playlist items
        int nRemoved = g_playlistPlayer.RemoveDVDItems();
        if ( nRemoved > 0 )
        {
          CGUIMessage msg( GUI_MSG_PLAYLIST_CHANGED, 0, 0 );
          m_gWindowManager.SendMessage( msg );
        }
        // stop the file if it's on dvd (will set the resume point etc)
        if (m_itemCurrentFile->IsOnDVD())
          StopPlaying();
      }
    }
    break;

  case GUI_MSG_PLAYBACK_STARTED:
    {
      // Update our infoManager with the new details etc.
      if (m_nextPlaylistItem >= 0)
      { // we've started a previously queued item
        CFileItemPtr item = g_playlistPlayer.GetPlaylist(g_playlistPlayer.GetCurrentPlaylist())[m_nextPlaylistItem];
        // update the playlist manager
        WORD currentSong = g_playlistPlayer.GetCurrentSong();
        DWORD dwParam = ((currentSong & 0xffff) << 16) | (m_nextPlaylistItem & 0xffff);
        CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_CHANGED, 0, 0, g_playlistPlayer.GetCurrentPlaylist(), dwParam, item);
        m_gWindowManager.SendThreadMessage(msg);
        g_playlistPlayer.SetCurrentSong(m_nextPlaylistItem);
        *m_itemCurrentFile = *item;
      }
      g_infoManager.SetCurrentItem(*m_itemCurrentFile);
      CLastFmManager::GetInstance()->OnSongChange(*m_itemCurrentFile);
      g_partyModeManager.OnSongChange(true);

      if (IsPlayingAudio())
      {
        // Start our cdg parser as appropriate
#ifdef HAS_KARAOKE
        if (m_pCdgParser && g_guiSettings.GetBool("karaoke.enabled") && !m_itemCurrentFile->IsInternetStream())
        {
          if (m_pCdgParser->IsRunning())
            m_pCdgParser->Stop();
          if (m_itemCurrentFile->IsMusicDb())
          {
            if (!m_itemCurrentFile->HasMusicInfoTag() || !m_itemCurrentFile->GetMusicInfoTag()->Loaded())
            {
              IMusicInfoTagLoader* tagloader = CMusicInfoTagLoaderFactory::CreateLoader(m_itemCurrentFile->m_strPath);
              tagloader->Load(m_itemCurrentFile->m_strPath,*m_itemCurrentFile->GetMusicInfoTag());
              delete tagloader;
            }
            m_pCdgParser->Start(m_itemCurrentFile->GetMusicInfoTag()->GetURL());
          }
          else
            m_pCdgParser->Start(m_itemCurrentFile->m_strPath);
        }
#endif
        //  Activate audio scrobbler
        if (CLastFmManager::GetInstance()->CanScrobble(*m_itemCurrentFile))
        {
          CScrobbler::GetInstance()->SetSongStartTime();
          CScrobbler::GetInstance()->SetSubmitSong(true);
        }
        else
          CScrobbler::GetInstance()->SetSubmitSong(false);

        // Notify that we'll allow the scrobble.
        PlexMediaServerQueue::Get().allowScrobble();
      }
      
      // Turn off the keyboard backlight if playing video
      if (IsPlayingVideo())
      {
        Cocoa_HW_SetKeyboardBacklightEnabled(false);
      }
      return true;
    }
    break;

  case GUI_MSG_QUEUE_NEXT_ITEM:
    {
      // Check to see if our playlist player has a new item for us,
      // and if so, we check whether our current player wants the file
      int iNext = g_playlistPlayer.GetNextSong();
      CPlayList& playlist = g_playlistPlayer.GetPlaylist(g_playlistPlayer.GetCurrentPlaylist());
      if (iNext < 0 || iNext >= playlist.size())
      {
        if (m_pPlayer) m_pPlayer->OnNothingToQueueNotify();
        return true; // nothing to do
      }
      // ok, grab the next song
      CFileItemPtr item = playlist[iNext];
      // ok - send the file to the player if it wants it
      if (m_pPlayer && m_pPlayer->QueueNextFile(*item))
      { // player wants the next file
        m_nextPlaylistItem = iNext;
      }
      return true;
    }
    break;

  case GUI_MSG_PLAYBACK_STOPPED:
  case GUI_MSG_PLAYBACK_ENDED:
  case GUI_MSG_PLAYLISTPLAYER_STOPPED:
    {
      // If the file ended naturally, notify.
      if (message.GetMessage() == GUI_MSG_PLAYBACK_ENDED)
        UpdateFileState(true, true);

      // Re-enable sounds.
      g_audioManager.Enable(true);
      
#ifdef __APPLE__
      // Restart the updater
      Cocoa_SetUpdateSuspended(false);
      
      // Start the background music (if enabled)
      Cocoa_SetBackgroundMusicEnabled(m_bBackgroundMusicEnabled);
      Cocoa_StartBackgroundMusic();
      
      // Enable the keyboard backlight again (will have no effect if not previously disabled)
      Cocoa_HW_SetKeyboardBacklightEnabled(true);
      
      // Require a server restart of the Plex Media Server if we just played a 5.1 surround file. 
      // This sucks, but no way around it for now, as it seems to screw up a WebKit process, and 
      // even reloading the WebKit plug-in doesn't help.
      //
      if (CoreAudioAUHAL::LastOpenWasSpdif())
        CPlexMediaServerPlayer::RequireServerRestart();
#endif

      // first check if we still have items in the stack to play
      if (message.GetMessage() == GUI_MSG_PLAYBACK_ENDED)
      {
        SaveCurrentFileSettings();

        if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0 && m_currentStackPosition < m_currentStack->Size() - 1)
        { // just play the next item in the stack
          PlayFile(*(*m_currentStack)[++m_currentStackPosition], true);
          return true;
        }
      }

      // reset our spindown
      m_bNetworkSpinDown = false;
      m_bSpinDown = false;

      // Save our settings for the current file for next time
      SaveCurrentFileSettings();

      // reset the current playing file
      m_itemCurrentFile->Reset();
      g_infoManager.ResetCurrentItem();
      m_currentStack->Clear();

      // Reset audioscrobbler submit status
      CScrobbler::GetInstance()->SetSubmitSong(false);

      // stop lastfm
      if (CLastFmManager::GetInstance()->IsRadioEnabled())
        CLastFmManager::GetInstance()->StopRadio();

      if (message.GetMessage() == GUI_MSG_PLAYBACK_ENDED)
      {
        // sending true to PlayNext() effectively passes bRestart to PlayFile()
        // which is not generally what we want (except for stacks, which are
        // handled above)
        g_playlistPlayer.PlayNext();
      }
      else
      {
        if (m_pPlayer)
        {
          delete m_pPlayer;
          m_pPlayer = 0;
        }
      }

#ifdef HAS_KARAOKE
      // no new player, free any cdg parser
      if (!m_pPlayer && m_pCdgParser)
        m_pCdgParser->Free();
#endif

      if (!IsPlayingVideo() && m_gWindowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO && m_bPlaybackStarting == false)
      {
        m_gWindowManager.PreviousWindow();
      }
      
      if (!IsPlayingAudio() && 
          g_playlistPlayer.GetCurrentPlaylist() == PLAYLIST_NONE && 
          IsVisualizerActive())
      {
        g_settings.Save();  // save vis settings
        ResetScreenSaverWindow();
        m_gWindowManager.PreviousWindow();
      }

      // reset the audio playlist on finish
      if (!IsPlayingAudio() && (g_guiSettings.GetBool("mymusic.clearplaylistsonend")) && (g_playlistPlayer.GetCurrentPlaylist() == PLAYLIST_MUSIC))
      {
        g_playlistPlayer.ClearPlaylist(PLAYLIST_MUSIC);
        g_playlistPlayer.Reset();
        g_playlistPlayer.SetCurrentPlaylist(PLAYLIST_NONE);
      }

      // DVD ejected while playing in vis ?
      if (!IsPlayingAudio() && (m_itemCurrentFile->IsCDDA() || m_itemCurrentFile->IsOnDVD()) && 
          !CDetectDVDMedia::IsDiscInDrive() && 
          IsVisualizerActive())
      {
        // yes, disable vis
        g_settings.Save();    // save vis settings
        ResetScreenSaverWindow();
        m_gWindowManager.PreviousWindow();
      }
      
      return true;
    }
    break;

  case GUI_MSG_PLAYLISTPLAYER_STARTED:
  case GUI_MSG_PLAYLISTPLAYER_CHANGED:
    {
      return true;
    }
    break;
  case GUI_MSG_FULLSCREEN:
    { // Switch to fullscreen, if we can
      SwitchToFullScreen();
      return true;
    }
    break;
  case GUI_MSG_EXECUTE:
    {
      // see if it is a user set string
      CLog::Log(LOGDEBUG,"%s : Translating %s", __FUNCTION__, message.GetStringParam().c_str());
      CGUIInfoLabel info(message.GetStringParam(), "");
      message.SetStringParam(info.GetLabel(0));
      CLog::Log(LOGDEBUG,"%s : To %s", __FUNCTION__, message.GetStringParam().c_str());

      // user has asked for something to be executed
      if (CUtil::IsBuiltIn(message.GetStringParam()))
        CUtil::ExecBuiltIn(message.GetStringParam());
      else
      {
        // try translating the action from our ButtonTranslator
        WORD actionID;
        if (g_buttonTranslator.TranslateActionString(message.GetStringParam().c_str(), actionID))
        {
          CAction action;
          action.wID = actionID;
          action.fAmount1 = 1.0f;
          m_gWindowManager.OnAction(action);
          return true;
        }
        CFileItem item(message.GetStringParam(), false);
#ifdef HAS_PYTHON
        if (item.IsPythonScript())
        { // a python script
          g_pythonParser.evalFile(item.m_strPath.c_str());
        }
        else
#endif
#ifdef HAS_XBOX_HARDWARE
        if (item.IsXBE())
        { // an XBE
          int iRegion;
          if (g_guiSettings.GetBool("myprograms.gameautoregion"))
          {
            CXBE xbe;
            iRegion = xbe.ExtractGameRegion(item.m_strPath);
            if (iRegion < 1 || iRegion > 7)
              iRegion = 0;
            iRegion = xbe.FilterRegion(iRegion);
          }
          else
            iRegion = 0;
          CUtil::RunXBE(item.m_strPath.c_str(),NULL,F_VIDEO(iRegion));
        }
        else
#endif
        if (item.IsAudio() || item.IsVideo())
        { // an audio or video file
          PlayFile(item);
        }
        else
          return false;
      }
      return true;
    }
  }
  return false;
}

void CApplication::Process()
{
  MEASURE_FUNCTION;

  // check if we need to load a new skin
  if (m_dwSkinTime && timeGetTime() >= m_dwSkinTime)
  {
    ReloadSkin();
  }

  // dispatch the messages generated by python or other threads to the current window
  m_gWindowManager.DispatchThreadMessages();

  // process messages which have to be send to the gui
  // (this can only be done after m_gWindowManager.Render())
  m_applicationMessenger.ProcessWindowMessages();

#ifdef HAS_PYTHON
  // process any Python scripts
  g_pythonParser.Process();
#endif

  // process messages, even if a movie is playing
  m_applicationMessenger.ProcessMessages();
  if (g_application.m_bStop) return; //we're done, everything has been unloaded

  // check for memory unit changes
#ifdef HAS_XBOX_HARDWARE
  if (g_memoryUnitManager.Update())
  { // changes have occured - update our shares
    CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_REMOVED_MEDIA);
    m_gWindowManager.SendThreadMessage(msg);
  }
#endif

  // check if we can free unused memory
#ifndef _LINUX
  g_audioManager.FreeUnused();
#endif

  // check how far we are through playing the current item
  // and do anything that needs doing (lastfm submission, playcount updates etc)
  CheckPlayingProgress();

  // update sound
  if (m_pPlayer)
    m_pPlayer->DoAudioWork();

  // process karaoke
#ifdef HAS_KARAOKE
  if (m_pCdgParser)
    m_pCdgParser->ProcessVoice();
#endif

  // do any processing that isn't needed on each run
  if( m_slowTimer.GetElapsedMilliseconds() > 500 )
  {
    m_slowTimer.Reset();
    ProcessSlow();
  }
}

void CApplication::UpdateFileState(bool final, bool ended)
{
  bool completed = false;
  
  if (m_itemCurrentFile->m_strPath.size() > 0)
  {
    if (final == true)
    {
      // Mark as watched if we are passed the usual amount
      if (ended == true || GetPercentage() >= g_advancedSettings.m_playCountMinimumPercent)
      {
        PlexMediaServerQueue::Get().onViewed(m_itemCurrentFile, true);
        m_itemCurrentFile->GetVideoInfoTag()->m_playCount++;
        m_itemCurrentFile->SetOverlayImage(CGUIListItem::ICON_OVERLAY_WATCHED);
        completed = true;
      }
    }

    // Ignore two minutes at start and either 2 minutes, or up to 5% at end (end credits)
    double current = GetTime();
    double total = GetTotalTime();
    if (current > 120 && total - current > 120 && total - current > 0.05 * total)
    {
      PlexMediaServerQueue::Get().onPlayingProgress(m_itemCurrentFile, (int)(current * 1000));
      m_itemCurrentFile->SetProperty("viewOffset", boost::lexical_cast<string>((int)(current*1000)));
      
      if (final == true && completed == false)
        m_itemCurrentFile->SetOverlayImage(CGUIListItem::ICON_OVERLAY_IN_PROGRESS);
    }      
    else
    {
      PlexMediaServerQueue::Get().onClearPlayingProgress(m_itemCurrentFile);
      m_itemCurrentFile->ClearProperty("viewOffset");
    }
    
    CGUIMediaWindow* mediaWindow = (CGUIMediaWindow* )m_gWindowManager.GetWindow(WINDOW_VIDEO_FILES);
    if (mediaWindow)
    {
      mediaWindow->SetUpdatedItem(m_itemCurrentFile);
    }
  }
}

void CApplication::ProcessSlow()
{
  // Every 5 seconds, update the progress.
  static int beat = 0;
  if (beat++ % 10 == 0 && IsPlaying())
    UpdateFileState();

  // Get the OS X volume if we're linked.
  if (g_guiSettings.GetBool("audiooutput.systemvolumefollows") && !g_audioConfig.UseDigitalOutput())
  {
    PlexAudioDevicePtr device = PlexAudioDevices::GetSelected();
    if (device)
    {    
      // Get the current system volume level.
      bool  isMuted = false;
      float volume = device->getVolume(isMuted);
      float plexVol = g_stSettings.m_nVolumeLevel/100.0;
      
      // Save the current system volume level if they differ substantially.
      if (fabs(volume - plexVol) > 0.5)
        g_stSettings.m_nVolumeLevel = volume*100.0;
      
      // Make sure the mute flag stays in sync.
      if (isMuted != g_stSettings.m_bMute)
        Mute();
    }
  }
  
  // Check to see if there is any activity going on
  CheckActive();

  if (m_bInactive)
  {
    // Check if we need to activate the screensaver.
    CheckScreenSaver();

#ifdef __APPLE__
    // Only activate display sleep if fullscreen mode.
    if (g_advancedSettings.m_fullScreen)
      CheckDisplaySleep();

    // Check if we need to shutdown (if enabled).
    if (g_advancedSettings.m_fullScreen)
#endif
      CheckShutdown();
  }

#ifdef __APPLE__
  // If not inactive tickle system, or else if in full-screen always tickle.
  if ((timeGetTime() - m_dwOSXscreensaverTicks) > 5000  && 
      (g_advancedSettings.m_fullScreen || !m_bInactive) &&
      m_bDisplaySleeping == false)
  {
    Cocoa_UpdateSystemActivity();
    m_dwOSXscreensaverTicks = timeGetTime();
  }
#endif

  // check if we should restart the player
  CheckDelayedPlayerRestart();

  //  check if we can unload any unreferenced dlls or sections
  CSectionLoader::UnloadDelayed();

  // check for any idle curl connections
  g_curlInterface.CheckIdle();

  // check for any idle myth sessions
  CCMythSession::CheckIdle();

#ifdef HAS_TIME_SERVER
  // check for any needed sntp update
  if(m_psntpClient && m_psntpClient->UpdateNeeded())
    m_psntpClient->Update();
#endif

  // LED - LCD SwitchOn On Paused! m_bIsPaused=TRUE -> LED/LCD is ON!
  if (IsPaused() != m_bIsPaused)
  {
    m_bIsPaused = IsPaused();
  }

  g_largeTextureManager.CleanupUnusedImages();

  // checks whats in the DVD drive and tries to autostart the content (xbox games, dvd, cdda, avi files...)
  m_Autorun.HandleAutorun();

  //Check to see if current playing Title has changed and whether we should broadcast the fact
  CheckForTitleChange();

#if defined(_LINUX) && defined(HAS_FILESYSTEM_SMB)
  smb.CheckIfIdle();
#endif

// Update HalManager to get newly connected media
#ifdef HAS_HAL
  while(g_HalManager.Update()) ;  //If there is 1 message it might be another one in queue, we take care of them directly
  if (CLinuxFileSystem::AnyDeviceChange())
  { // changes have occured - update our shares
    CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_REMOVED_MEDIA);
    m_gWindowManager.SendThreadMessage(msg);
  }
#endif
}

// Global Idle Time in Seconds
// idle time will be resetet if on any OnKey()
// int return: system Idle time in seconds! 0 is no idle!
int CApplication::GlobalIdleTime()
{
  if(!m_idleTimer.IsRunning())
  {
    m_idleTimer.Stop();
    m_idleTimer.StartZero();
  }
  return (int)m_idleTimer.GetElapsedSeconds();
}

float CApplication::NavigationIdleTime()
{
  if (!m_navigationTimer.IsRunning())
  {
    m_navigationTimer.Stop();
    m_navigationTimer.StartZero();
  }
  return m_navigationTimer.GetElapsedSeconds();
}

void CApplication::DelayedPlayerRestart()
{
  m_restartPlayerTimer.StartZero();
}

void CApplication::CheckDelayedPlayerRestart()
{
  if (m_restartPlayerTimer.GetElapsedSeconds() > 3)
  {
    m_restartPlayerTimer.Stop();
    m_restartPlayerTimer.Reset();
    Restart(true);
  }
}

void CApplication::RestartWithNewPlayer(CDlgCache* cacheDlg, const CStdString& newURL)
{
  CFileItem newFile(newURL, false);
  newFile.SetLabel(m_itemCurrentFile->GetLabel());
  *m_itemCurrentFile = newFile;
  
  // We're moving to a new player, so whack the old one.
  delete m_pPlayer;
  m_pPlayer = 0;
  
  // Create the new player.
  EPLAYERCORES eNewCore = CPlayerCoreFactory::GetDefaultPlayer(newFile);
  m_eCurrentPlayer = eNewCore;
  m_pPlayer = CPlayerCoreFactory::CreatePlayer(eNewCore, *this);
  
  // See if we're passing along the cache dialog.
  if (cacheDlg)
  {
    if (eNewCore == EPC_PMSPLAYER)
      ((CPlexMediaServerPlayer* )m_pPlayer)->SetCacheDialog(cacheDlg);
    else
      cacheDlg->Close();
  }
  
  PlayFile(*m_itemCurrentFile, false);
}

void CApplication::Restart(bool bSamePosition)
{
  // this function gets called when the user changes a setting (like noninterleaved)
  // and which means we gotta close & reopen the current playing file

  // first check if we're playing a file
  if ( !IsPlayingVideo() && !IsPlayingAudio())
    return ;

  if( !m_pPlayer )
    return ;

  // do we want to return to the current position in the file
  if (false == bSamePosition)
  {
    // no, then just reopen the file and start at the beginning
    PlayFile(*m_itemCurrentFile, true);
    return ;
  }

  // else get current position
  double time = GetTime();

  // get player state, needed for dvd's
  CStdString state = m_pPlayer->GetPlayerState();

  // set the requested starttime
  m_itemCurrentFile->m_lStartOffset = (long)(time * 75.0);

  // reopen the file
  if ( PlayFile(*m_itemCurrentFile, true) && m_pPlayer )
    m_pPlayer->SetPlayerState(state);
}

const CStdString& CApplication::CurrentFile()
{
  return m_itemCurrentFile->m_strPath;
}

CFileItem& CApplication::CurrentFileItem()
{
  return *m_itemCurrentFile;
}

void CApplication::Mute(void)
{
  if (g_stSettings.m_bMute)
  { // muted - unmute.
    // check so we don't get stuck in some muted state
    if( g_stSettings.m_iPreMuteVolumeLevel == 0 ) g_stSettings.m_iPreMuteVolumeLevel = 1;
    SetVolume(g_stSettings.m_iPreMuteVolumeLevel);
  }
  else
  { // mute
    g_stSettings.m_iPreMuteVolumeLevel = GetVolume();
    SetVolume(0);
  }
}

void CApplication::SetVolume(int iPercent)
{
  // convert the percentage to a mB (milliBell) value (*100 for dB)
  long hardwareVolume = (long)((float)iPercent * 0.01f * (VOLUME_MAXIMUM - VOLUME_MINIMUM) + VOLUME_MINIMUM);
  SetHardwareVolume(hardwareVolume);
  if (g_guiSettings.GetBool("audiooutput.systemvolumefollows") && !g_audioConfig.UseDigitalOutput())
  {
    g_audioManager.SetSystemVolumeScalar(iPercent);
  }
  else
  {
    g_audioManager.SetVolume(g_stSettings.m_nVolumeLevel);
  }
}

void CApplication::SetHardwareVolume(long hardwareVolume)
{
  // TODO DRC
  if (hardwareVolume >= VOLUME_MAXIMUM) // + VOLUME_DRC_MAXIMUM
    hardwareVolume = VOLUME_MAXIMUM;// + VOLUME_DRC_MAXIMUM;
  if (hardwareVolume <= VOLUME_MINIMUM)
  {
    hardwareVolume = VOLUME_MINIMUM;
  }
  // update our settings
  if (hardwareVolume > VOLUME_MAXIMUM)
  {
    g_stSettings.m_dynamicRangeCompressionLevel = hardwareVolume - VOLUME_MAXIMUM;
    g_stSettings.m_nVolumeLevel = VOLUME_MAXIMUM;
  }
  else
  {
    g_stSettings.m_dynamicRangeCompressionLevel = 0;
    g_stSettings.m_nVolumeLevel = hardwareVolume;
  }
  
  // update mute state
  if(!g_stSettings.m_bMute && hardwareVolume <= VOLUME_MINIMUM)
  {
    g_stSettings.m_bMute = true;
    if (!m_guiDialogMuteBug.IsDialogRunning())
      m_guiDialogMuteBug.Show();
  }
  else if(g_stSettings.m_bMute && hardwareVolume > VOLUME_MINIMUM)
  {
    g_stSettings.m_bMute = false;
    if (m_guiDialogMuteBug.IsDialogRunning())
      m_guiDialogMuteBug.Close();
  }
  
  // tell Cocoa objects that the volume has changed
  Cocoa_UpdateGlobalVolume(GetVolume());

  // and tell our player to update the volume
  if (m_pPlayer && !(g_guiSettings.GetBool("audiooutput.systemvolumefollows") && !g_audioConfig.UseDigitalOutput()))
  {
    m_pPlayer->SetVolume(g_stSettings.m_nVolumeLevel);
    // TODO DRC
    // m_pPlayer->SetDynamicRangeCompression(g_stSettings.m_dynamicRangeCompressionLevel);
  }
}

int CApplication::GetVolume() const
{
  // converts the hardware volume (in mB) to a percentage
  return int(((float)(g_stSettings.m_nVolumeLevel + g_stSettings.m_dynamicRangeCompressionLevel - VOLUME_MINIMUM)) / (VOLUME_MAXIMUM - VOLUME_MINIMUM)*100.0f + 0.5f);
}
  
#ifdef __APPLE__
float CApplication::GetPanelBrightness() const
{
  float panelBrightness = 0.0f;
  Cocoa_GetPanelBrightness(&panelBrightness);
  return panelBrightness;
}

void CApplication::SetPanelBrightness(float iPanelBrightness)
{
  Cocoa_SetPanelBrightness(iPanelBrightness);
}
#endif

void CApplication::SetPlaySpeed(int iSpeed)
{
  if (!IsPlayingAudio() && !IsPlayingVideo())
    return ;
  if (m_iPlaySpeed == iSpeed)
    return ;
  if (!m_pPlayer->CanSeek())
    return;
  if (m_pPlayer->IsPaused())
  {
    if (
      ((m_iPlaySpeed > 1) && (iSpeed > m_iPlaySpeed)) ||
      ((m_iPlaySpeed < -1) && (iSpeed < m_iPlaySpeed))
    )
    {
      iSpeed = m_iPlaySpeed; // from pause to ff/rw, do previous ff/rw speed
    }
    m_pPlayer->Pause();
  }
  m_iPlaySpeed = iSpeed;

  m_pPlayer->ToFFRW(m_iPlaySpeed);
  
  if (!(g_guiSettings.GetBool("audiooutput.systemvolumefollows") && !g_audioConfig.UseDigitalOutput()))
  {
    if (m_iPlaySpeed == 1)
    { // restore volume
      m_pPlayer->SetVolume(g_stSettings.m_nVolumeLevel);
    }
    else
    { // mute volume
      m_pPlayer->SetVolume(VOLUME_MINIMUM);
    }
  }
}
  
int CApplication::GetPlaySpeed() const
{
  return m_iPlaySpeed;
}
  

// Returns the total time in seconds of the current media.  Fractional
// portions of a second are possible - but not necessarily supported by the
// player class.  This returns a double to be consistent with GetTime() and
// SeekTime().
double CApplication::GetTotalTime() const
{
  double rc = 0.0;

  if (IsPlaying() && m_pPlayer)
  {
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
      rc = (*m_currentStack)[m_currentStack->Size() - 1]->m_lEndOffset;
    else
      rc = m_pPlayer->GetTotalTime();
  }

  return rc;
}

void CApplication::ResetPlayTime()
{
  if (IsPlaying() && m_pPlayer)
    m_pPlayer->ResetTime();
}

// Returns the current time in seconds of the currently playing media.
// Fractional portions of a second are possible.  This returns a double to
// be consistent with GetTotalTime() and SeekTime().
double CApplication::GetTime() const
{
  double rc = 0.0;

  if (IsPlaying() && m_pPlayer)
  {
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
    {
      long startOfCurrentFile = (m_currentStackPosition > 0) ? (*m_currentStack)[m_currentStackPosition-1]->m_lEndOffset : 0;
      rc = (double)startOfCurrentFile + m_pPlayer->GetTime() * 0.001;
    }
    else
      rc = static_cast<double>(m_pPlayer->GetTime() * 0.001f);
  }

  return rc;
}

// Sets the current position of the currently playing media to the specified
// time in seconds.  Fractional portions of a second are valid.  The passed
// time is the time offset from the beginning of the file as opposed to a
// delta from the current position.  This method accepts a double to be
// consistent with GetTime() and GetTotalTime().
void CApplication::SeekTime( double dTime )
{
  if (IsPlaying() && m_pPlayer && (dTime >= 0.0))
  {
    if (!m_pPlayer->CanSeek()) return;
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
    {
      // find the item in the stack we are seeking to, and load the new
      // file if necessary, and calculate the correct seek within the new
      // file.  Otherwise, just fall through to the usual routine if the
      // time is higher than our total time.
      for (int i = 0; i < m_currentStack->Size(); i++)
      {
        if ((*m_currentStack)[i]->m_lEndOffset > dTime)
        {
          long startOfNewFile = (i > 0) ? (*m_currentStack)[i-1]->m_lEndOffset : 0;
          if (m_currentStackPosition == i)
            m_pPlayer->SeekTime((__int64)((dTime - startOfNewFile) * 1000.0));
          else
          { // seeking to a new file
            m_currentStackPosition = i;
            CFileItem item(*(*m_currentStack)[i]);
            item.m_lStartOffset = (long)((dTime - startOfNewFile) * 75.0);
            // don't just call "PlayFile" here, as we are quite likely called from the
            // player thread, so we won't be able to delete ourselves.
            m_applicationMessenger.PlayFile(item, true);
          }
          return;
        }
      }
    }
    // convert to milliseconds and perform seek
    m_pPlayer->SeekTime( static_cast<__int64>( dTime * 1000.0 ) );
  }
}

float CApplication::GetPercentage() const
{
  if (IsPlaying() && m_pPlayer)
  {
    if (IsPlayingAudio() && m_itemCurrentFile->HasMusicInfoTag())
    {
      const CMusicInfoTag& tag = *m_itemCurrentFile->GetMusicInfoTag();
      if (tag.GetDuration() > 0)
        return (float)(GetTime() / tag.GetDuration() * 100);
    }

    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
      return (float)(GetTime() / GetTotalTime() * 100);
    else
      return m_pPlayer->GetPercentage();
  }
  return 0.0f;
}

void CApplication::SeekPercentage(float percent)
{
  if (IsPlaying() && m_pPlayer && (percent >= 0.0))
  {
    if (!m_pPlayer->CanSeek()) return;
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
      SeekTime(percent * 0.01 * GetTotalTime());
    else
      m_pPlayer->SeekPercentage(percent);
  }
}

// SwitchToFullScreen() returns true if a switch is made, else returns false
bool CApplication::SwitchToFullScreen()
{
  // if playing from the video info window, close it first!
  if (m_gWindowManager.HasModalDialog() && m_gWindowManager.GetTopMostModalDialogID() == WINDOW_VIDEO_INFO)
  {
    CGUIWindowVideoInfo* pDialog = (CGUIWindowVideoInfo*)m_gWindowManager.GetWindow(WINDOW_VIDEO_INFO);
    if (pDialog) pDialog->Close(true);
  }

  // don't switch if there is a dialog on screen or the slideshow is active
  if (/*m_gWindowManager.HasModalDialog() ||*/ m_gWindowManager.GetActiveWindow() == WINDOW_SLIDESHOW)
    return false;

  // See if we're playing a video, and are in GUI mode
  if ( IsPlayingVideo() && m_gWindowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO)
  {
#ifdef HAS_SDL
    // Reset frame count so that timing is FPS will be correct.
    SDL_mutexP(m_frameMutex);
    m_frameCount = 0;
    SDL_mutexV(m_frameMutex);
#endif

    // then switch to fullscreen mode
    m_gWindowManager.ActivateWindow(WINDOW_FULLSCREEN_VIDEO);
    g_TextureManager.Flush();
    return true;
  }
  // special case for switching between GUI & visualisation mode. (only if we're playing an audio song)
  if (IsPlayingAudio() && IsVisualizerActive() == false) 
  { 
    // then switch to visualisation
    ActivateVisualizer();
    g_TextureManager.Flush();
    return true;
  }
  return false;
}

const EPLAYERCORES CApplication::GetCurrentPlayer()
{
  return m_eCurrentPlayer;
}

// when a scan is initiated, save current settings
// and enable tag reading and remote thums
void CApplication::SaveMusicScanSettings()
{
  CLog::Log(LOGINFO,"Music scan has started ... enabling Tag Reading, and Remote Thumbs");
  g_stSettings.m_bMyMusicIsScanning = true;
  g_settings.Save();
}

void CApplication::RestoreMusicScanSettings()
{
  g_stSettings.m_bMyMusicIsScanning = false;
  g_settings.Save();
}

void CApplication::CheckPlayingProgress()
{
  // check if we haven't rewound past the start of the file
  if (IsPlaying())
  {
    int iSpeed = g_application.GetPlaySpeed();
    if (iSpeed < 1)
    {
      iSpeed *= -1;
      int iPower = 0;
      while (iSpeed != 1)
      {
        iSpeed >>= 1;
        iPower++;
      }
      if (g_infoManager.GetPlayTime() / 1000 < iPower)
      {
        g_application.SetPlaySpeed(1);
        g_application.SeekTime(0);
      }
    }
  }

  if (!IsPlayingAudio()) return;

  CheckAudioScrobblerStatus();

  // work out where we are in the playing item
  if (GetPercentage() >= g_advancedSettings.m_playCountMinimumPercent)
  { // consider this item as played
    if (m_playCountUpdated)
      return;
    m_playCountUpdated = true;
    if (IsPlayingAudio())
    {
      //musicdatabase.IncrTop100CounterByFileName(m_itemCurrentFile->m_strPath);
      //musicdatabase.Close();
    }
  }
  else
    m_playCountUpdated = false;
}

void CApplication::CheckAudioScrobblerStatus()
{
  if (IsPlayingAudio() && GetTime()==0.0)
  {
    if (CLastFmManager::GetInstance()->CanScrobble(*m_itemCurrentFile) &&
        !CScrobbler::GetInstance()->ShouldSubmit())
    {
      //  We seeked to the beginning of the file
      //  reinit audio scrobbler
      CScrobbler::GetInstance()->SetSongStartTime();
      CScrobbler::GetInstance()->SetSubmitSong(true);
    }
    
    // Notify that we'll allow the scrobble.
    PlexMediaServerQueue::Get().allowScrobble();
    return;
  }

  if (!IsPlayingAudio())
    return;

  //  Don't submit songs to audioscrobber when the user seeks.
  //  Rule from audioscrobbler:
  //  http://www.audioscrobbler.com/development/protocol.php
  if (GetPlaySpeed()!=1)
  {
    CScrobbler::GetInstance()->SetSubmitSong(false);
    return;
  }

  //  Submit the song if 50% or 240 seconds are played
  double dTime=(double)g_infoManager.GetPlayTime()/1000.0;
  const CMusicInfoTag* tag=g_infoManager.GetCurrentSongTag();
  double dLength=0.f;
  if (tag)
    dLength=(tag->GetDuration()>0) ? (tag->GetDuration()/2.0f) : (GetTotalTime()/2.0f);

  if (!tag || !tag->Loaded() || dLength==0.0f)
  {
    CScrobbler::GetInstance()->SetSubmitSong(false);
    return;
  }
  
  if ((dLength)>240.0f)
    dLength=240.0f;
  int iTimeTillSubmit=(int)(dLength-dTime);
  CScrobbler::GetInstance()->SetSecsTillSubmit(iTimeTillSubmit);

  if (dTime>dLength)
  {
    if (CScrobbler::GetInstance()->ShouldSubmit())
    {
      CScrobbler::GetInstance()->AddSong(*tag);
      CScrobbler::GetInstance()->SetSubmitSong(false);
    }
    
    // Submit to Plex Media Server.
    PlexMediaServerQueue::Get().onViewed(m_itemCurrentFile);
  }
}

bool CApplication::ProcessAndStartPlaylist(const CStdString& strPlayList, CPlayList& playlist, int iPlaylist)
{
  CLog::Log(LOGDEBUG,"CApplication::ProcessAndStartPlaylist(%s, %i)",strPlayList.c_str(), iPlaylist);

  // initial exit conditions
  // no songs in playlist just return
  if (playlist.size() == 0)
    return false;

  // illegal playlist
  if (iPlaylist < PLAYLIST_MUSIC || iPlaylist > PLAYLIST_VIDEO)
    return false;

  // setup correct playlist
  g_playlistPlayer.ClearPlaylist(iPlaylist);

  // if the playlist contains an internet stream, this file will be used
  // to generate a thumbnail for musicplayer.cover
  g_application.m_strPlayListFile = strPlayList;

  // add the items to the playlist player
  g_playlistPlayer.Add(iPlaylist, playlist);

  // if we have a playlist
  if (g_playlistPlayer.GetPlaylist(iPlaylist).size())
  {
    // start playing it
    g_playlistPlayer.SetCurrentPlaylist(iPlaylist);
    g_playlistPlayer.Reset();
    g_playlistPlayer.Play();
    return true;
  }
  return false;
}

void CApplication::CheckForDebugButtonCombo()
{
#ifdef HAS_GAMEPAD
  ReadInput();
  if (m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_X] && m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_Y])
  {
    g_advancedSettings.m_logLevel = LOG_LEVEL_DEBUG_FREEMEM;
    CLog::Log(LOGINFO, "Key combination detected for full debug logging (X+Y)");
  }
#endif
}

void CApplication::StartFtpEmergencyRecoveryMode()
{
#ifdef HAS_FTP_SERVER
  m_pFileZilla = new CXBFileZilla(NULL);
  m_pFileZilla->Start();

  // Default settings
  m_pFileZilla->mSettings.SetMaxUsers(0);
  m_pFileZilla->mSettings.SetWelcomeMessage("XBMC emergency recovery console FTP.");

  // default user
  CXFUser* pUser;
  m_pFileZilla->AddUser("xbox", pUser);
  pUser->SetPassword("xbox");
  pUser->SetShortcutsEnabled(false);
  pUser->SetUseRelativePaths(false);
  pUser->SetBypassUserLimit(false);
  pUser->SetUserLimit(0);
  pUser->SetIPLimit(0);
  pUser->AddDirectory("/", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS | XBDIR_HOME);
  pUser->AddDirectory("C:\\", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS);
  pUser->AddDirectory("D:\\", XBFILE_READ | XBDIR_LIST | XBDIR_SUBDIRS);
  pUser->AddDirectory("E:\\", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS);
  pUser->AddDirectory("Q:\\", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS);
  //Add. also Drive F/G
  if (CIoSupport::DriveExists('F')){
    pUser->AddDirectory("F:\\", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS);
  }
  if (CIoSupport::DriveExists('G')){
    pUser->AddDirectory("G:\\", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS);
  }
  pUser->CommitChanges();
#endif
}

void CApplication::SaveCurrentFileSettings()
{    
  // FIXME, this is deprecated and the data should be pushed to PMS.
  if (m_itemCurrentFile->IsVideo())
  { 
    // Save video settings
    if (g_stSettings.m_currentVideoSettings != g_stSettings.m_defaultVideoSettings)
    {
      CVideoDatabase dbs;
      dbs.Open();
      dbs.SetVideoSettings(m_itemCurrentFile->m_strPath, g_stSettings.m_currentVideoSettings);
      dbs.Close();
    }
  }
}

CApplicationMessenger& CApplication::getApplicationMessenger()
{
   return m_applicationMessenger;
}

bool CApplication::IsCurrentThread() const
{
  return m_threadID == GetCurrentThreadId(); 
}

#ifdef HAS_LINUX_NETWORK
CNetworkLinux& CApplication::getNetwork()
{
  return m_network;
}
#else
CNetwork& CApplication::getNetwork()
{
  return m_network;
}
#endif
#ifdef HAS_PERFORMANCE_SAMPLE
CPerformanceStats &CApplication::GetPerformanceStats()
{
  return m_perfStats;
}
#endif


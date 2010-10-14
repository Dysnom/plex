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
#include "GUIWindow.h"
#include "GUIWindowVideoInfo.h"
#include "Util.h"
#include "Picture.h"
#include "guiImage.h"
#include "StringUtils.h"
#include "GUIWindowVideoBase.h"
#include "GUIWindowVideoFiles.h"
#include "GUIDialogFileBrowser.h"
#include "utils/GUIInfoManager.h"
#include "VideoInfoScanner.h"
#include "Application.h"
#include "VideoInfoTag.h"
#include "GUIWindowManager.h"
#include "GUIDialogOK.h"
#include "GUIDialogYesNo.h"
#include "GUIDialogSelect.h"
#include "GUIDialogProgress.h"
#include "FileSystem/Directory.h"
#include "FileSystem/File.h"
#include "FileItem.h"
#include "MediaManager.h"
#include "utils/AsyncFileCopy.h"
#include "XMLUtils.h"

#include "HTTP.h"
#include "PlexDirectory.h"

using namespace std;
using namespace XFILE;
using namespace DIRECTORY;

#define CONTROL_TITLE               20
#define CONTROL_DIRECTOR            21
#define CONTROL_CREDITS             22
#define CONTROL_GENRE               23
#define CONTROL_YEAR                24
#define CONTROL_TAGLINE             25
#define CONTROL_PLOTOUTLINE         26
#define CONTROL_CAST                29
#define CONTROL_RATING_AND_VOTES    30
#define CONTROL_RUNTIME             31
#define CONTROL_MPAARATING          32
#define CONTROL_TITLE_AND_YEAR      33
#define CONTROL_STUDIO              36
#define CONTROL_TOP250              37
#define CONTROL_TRAILER             38

#define CONTROL_TEXTAREA             4
#define CONTROL_BTN_TRACKS           5
#define CONTROL_BTN_REFRESH          6
#define CONTROL_DISC                 7
#define CONTROL_BTN_PLAY             8
#define CONTROL_BTN_RESUME           9
#define CONTROL_BTN_GET_THUMB       10
#define CONTROL_BTN_PLAY_TRAILER    11
#define CONTROL_BTN_GET_FANART      12
#define CONTROL_BTN_GET_BANNER      13

#define CONTROL_LIST                50

CGUIWindowVideoInfo::CGUIWindowVideoInfo(void)
    : CGUIDialog(WINDOW_VIDEO_INFO, "DialogVideoInfo.xml")
    , m_movieItem(new CFileItem)
{
  m_bRefreshAll = true;
  m_bRefresh = false;
  m_castList = new CFileItemList;
}

CGUIWindowVideoInfo::~CGUIWindowVideoInfo(void)
{
  delete m_castList;
}

bool CGUIWindowVideoInfo::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_WINDOW_DEINIT:
    {
      ClearCastList();
    }
    break;

  case GUI_MSG_WINDOW_INIT:
    {
      m_dlgProgress = (CGUIDialogProgress*)m_gWindowManager.GetWindow(WINDOW_DIALOG_PROGRESS);

      m_bRefresh = false;
      m_bRefreshAll = true;

      CGUIDialog::OnMessage(message);
      m_bViewReview = true;
      CGUIMessage msg(GUI_MSG_LABEL_RESET, GetID(), CONTROL_DISC);
      OnMessage(msg);
      for (int i = 0; i < 1000; ++i)
      {
        CStdString strItem;
        strItem.Format("DVD#%03i", i);
        CGUIMessage msg2(GUI_MSG_LABEL_ADD, GetID(), CONTROL_DISC);
        msg2.SetLabel(strItem);
        OnMessage(msg2);
      }

      SET_CONTROL_HIDDEN(CONTROL_DISC);
/*      CONTROL_DISABLE(CONTROL_DISC);
      int iItem = 0;
      CFileItem movie(m_Movie.m_strFileNameAndPath, false);
      if ( movie.IsOnDVD() )
      {
        SET_CONTROL_VISIBLE(CONTROL_DISC);
        CONTROL_ENABLE(CONTROL_DISC);
        char szNumber[1024];
        int iPos = 0;
        bool bNumber = false;
        for (int i = 0; i < (int)m_Movie.m_strDVDLabel.size();++i)
        {
          char kar = m_Movie.m_strDVDLabel.GetAt(i);
          if (kar >= '0' && kar <= '9' )
          {
            szNumber[iPos] = kar;
            iPos++;
            szNumber[iPos] = 0;
            bNumber = true;
          }
          else
          {
            if (bNumber) break;
          }
        }
        int iDVD = 0;
        if (strlen(szNumber))
        {
          int x = 0;
          while (szNumber[x] == '0' && x < (int)strlen(szNumber) ) x++;
          if (x < (int)strlen(szNumber))
          {
            sscanf(&szNumber[x], "%i", &iDVD);
            if (iDVD < 0 && iDVD >= 1000)
              iDVD = -1;
          }
        }
        if (iDVD <= 0) iDVD = 0;
        iItem = iDVD;

        CGUIMessage msgSet(GUI_MSG_ITEM_SELECT, GetID(), CONTROL_DISC, iItem, 0, NULL);
        OnMessage(msgSet);
      }*/
      Refresh();

      // dont allow refreshing of manual info
      if (m_movieItem->GetVideoInfoTag()->m_strIMDBNumber.Left(2).Equals("xx"))
        CONTROL_DISABLE(CONTROL_BTN_REFRESH);
      // dont allow get thumb for plugin entries
      if (m_movieItem->GetVideoInfoTag()->m_strIMDBNumber.Mid(2).Equals("plugin"))
        CONTROL_DISABLE(CONTROL_BTN_GET_THUMB);
      return true;
    }
    break;


  case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();
      if (iControl == CONTROL_BTN_REFRESH)
      {
        if (m_movieItem->GetVideoInfoTag()->m_iSeason < 0 && !m_movieItem->GetVideoInfoTag()->m_strShowTitle.IsEmpty()) // tv show
        {
          bool bCanceled=false;
          if (CGUIDialogYesNo::ShowAndGetInput(20377,20378,-1,-1,bCanceled))
          {
            m_bRefreshAll = true;
            CVideoDatabase db;
            if (db.Open())
            {
              db.SetPathHash(m_movieItem->GetVideoInfoTag()->m_strPath,"");
              db.Close();
            }
          }
          else
            m_bRefreshAll = false;

          if (bCanceled)
            return false;
        }
        m_bRefresh = true;
        Close();
        return true;
      }
      else if (iControl == CONTROL_BTN_TRACKS)
      {
        m_bViewReview = !m_bViewReview;
        Update();
      }
      else if (iControl == CONTROL_BTN_PLAY)
      {
        Play();
      }
      else if (iControl == CONTROL_BTN_RESUME)
      {
        Play(true);
      }
      else if (iControl == CONTROL_BTN_GET_THUMB)
      {
        OnGetThumb();
      }
      else if (iControl == CONTROL_BTN_PLAY_TRAILER)
      {
        PlayTrailer();
      }
      else if (iControl == CONTROL_BTN_GET_FANART)
      {
        OnGetFanart();
      }
      else if (iControl == CONTROL_BTN_GET_BANNER)
      {
        OnGetBanner();
      }
      
/*      else if (iControl == CONTROL_DISC)
      {
        int iItem = 0;
        CGUIMessage msg(GUI_MSG_ITEM_SELECTED, GetID(), iControl, 0, 0, NULL);
        OnMessage(msg);
        CStdString strItem = msg.GetLabel();
        if (strItem != "HD" && strItem != "share")
        {
          long lMovieId;
          sscanf(m_Movie.m_strSearchString.c_str(), "%i", &lMovieId);
          if (lMovieId > 0)
          {
            CStdString label;
            //m_database.GetDVDLabel(lMovieId, label);
            int iPos = label.Find("DVD#");
            if (iPos >= 0)
            {
              label.Delete(iPos, label.GetLength());
            }
            label = label.TrimRight(" ");
            label += " ";
            label += strItem;
            //m_database.SetDVDLabel( lMovieId, label);
          }
        }
      }*/
      else if (iControl == CONTROL_LIST)
      {
        int iAction = message.GetParam1();
        if (ACTION_SELECT_ITEM == iAction)
        {
          CGUIMessage msg(GUI_MSG_ITEM_SELECTED, GetID(), iControl);
          OnMessage(msg);
          int iItem = msg.GetParam1();
          if (iItem < 0 || iItem >= m_castList->Size())
            break;
          CStdString strItem = m_castList->Get(iItem)->GetLabel();
          CStdString strFind;
          strFind.Format(" %s ",g_localizeStrings.Get(20347));
          int iPos = strItem.Find(strFind);
          if (iPos == -1)
            iPos = strItem.size();
          CStdString tmp = strItem.Left(iPos);
          OnSearch(tmp);
        }
      }
    }
    break;
  }

  return CGUIDialog::OnMessage(message);
}

void CGUIWindowVideoInfo::SetMovie(const CFileItemPtr& item)
{
  m_movieItem = item;
  ClearCastList();
  
  // Set the cast list appropriately.
  m_castList->SetContent(m_movieItem->GetProperty("mediaType"));
  
  if (m_castList->GetContent() == "movies")
  {
    // Compute the URL for the movie information.
    CURL url(item->m_strPath);
    url.SetFileName("library/metadata/" + item->GetProperty("ratingKey"));

    // Download the data.
    CHTTP set;
    CStdString strData;
    set.Open(url.GetURL(), "GET", 0);
    set.ReadData(strData);
    set.Close();

    // Parse document.
    TiXmlDocument xmlDoc;
    if (!xmlDoc.Parse(strData)) return;

    // The container node.
    TiXmlElement* root = xmlDoc.RootElement();
    if (!root) return;

    // The Video node.
    TiXmlElement* video = root->FirstChildElement();
    if (!video) return;

    // The child nodes.
    TiXmlElement* role = video->FirstChildElement("Role");
    if (!role) return;

    while (role)
    {
      const char* strActor = role->Attribute("tag");
      const char* strRole  = role->Attribute("role");

      CStdString character;
      if (strRole == 0 || strlen(strRole) == 0)
        character = strActor;
      else
        character.Format("%s %s %s", strActor, g_localizeStrings.Get(20347).c_str(), strRole);

      CFileItemPtr item(new CFileItem(strActor));
      item->SetIconImage("DefaultActor.png");
      item->SetLabel(character);
      m_castList->Add(item);

      role = role->NextSiblingElement();
    }
  }
  
#if 0
  
  // setup cast list + determine type.  We need to do this here as it makes
  // sure that content type (among other things) is set correctly for the
  // old fixed id labels that we have floating around (they may be using
  // content type to determine visibility, so we'll set the wrong label)
  ClearCastList();
  VIDEODB_CONTENT_TYPE type = GetContentType(m_movieItem.get());
  if (type == VIDEODB_CONTENT_MUSICVIDEOS)
  { // music video
    CStdStringArray artists;
    StringUtils::SplitString(m_movieItem->GetVideoInfoTag()->m_strArtist, g_advancedSettings.m_videoItemSeparator, artists);
    for (std::vector<CStdString>::const_iterator it = artists.begin(); it != artists.end(); ++it)
    {
      CFileItemPtr item(new CFileItem(*it));
      if (CFile::Exists(item->GetCachedArtistThumb()))
        item->SetThumbnailImage(item->GetCachedArtistThumb());
      item->SetIconImage("DefaultArtist.png");
      m_castList->Add(item);
    }
    m_castList->SetContent("musicvideos");
  }
  else
  { // movie/show/episode
    for (CVideoInfoTag::iCast it = m_movieItem->GetVideoInfoTag()->m_cast.begin(); it != m_movieItem->GetVideoInfoTag()->m_cast.end(); ++it)
    {
      CStdString character;
      if (it->strRole.IsEmpty())
        character = it->strName;
      else
        character.Format("%s %s %s", it->strName.c_str(), g_localizeStrings.Get(20347).c_str(), it->strRole.c_str());
      CFileItemPtr item(new CFileItem(it->strName));
      if (CFile::Exists(item->GetCachedActorThumb()))
        item->SetThumbnailImage(item->GetCachedActorThumb());
      item->SetIconImage("DefaultActor.png");
      item->SetLabel(character);
      m_castList->Add(item);
    }
    // set fanart property for tvshows and movies
    if (type == VIDEODB_CONTENT_TVSHOWS || type == VIDEODB_CONTENT_MOVIES)
    {
      m_movieItem->CacheFanart();
      if (CFile::Exists(m_movieItem->GetCachedFanart()))
        m_movieItem->SetProperty("fanart_image",m_movieItem->GetCachedFanart());
    }
    // determine type:
    if (type == VIDEODB_CONTENT_TVSHOWS)
    {
      m_castList->SetContent("tvshows");
      // special case stuff for shows (not currently retrieved from the library in filemode (ref: GetTvShowInfo vs GetTVShowsByWhere)
      m_movieItem->m_dateTime.SetFromDateString(m_movieItem->GetVideoInfoTag()->m_strPremiered);
      m_movieItem->GetVideoInfoTag()->m_iYear = m_movieItem->m_dateTime.GetYear();
      m_movieItem->SetProperty("watchedepisodes", m_movieItem->GetVideoInfoTag()->m_playCount);
      m_movieItem->SetProperty("unwatchedepisodes", m_movieItem->GetVideoInfoTag()->m_iEpisode - m_movieItem->GetVideoInfoTag()->m_playCount);
      m_movieItem->GetVideoInfoTag()->m_playCount = (m_movieItem->GetVideoInfoTag()->m_iEpisode == m_movieItem->GetVideoInfoTag()->m_playCount) ? 1 : 0;
    }
    else if (type == VIDEODB_CONTENT_EPISODES)
    {
      m_castList->SetContent("episodes");
      // special case stuff for episodes (not currently retrieved from the library in filemode (ref: GetEpisodeInfo vs GetEpisodesByWhere)
      m_movieItem->m_dateTime.SetFromDateString(m_movieItem->GetVideoInfoTag()->m_strFirstAired);
      m_movieItem->GetVideoInfoTag()->m_iYear = m_movieItem->m_dateTime.GetYear();
      // retrieve the season thumb.
      // NOTE: This is overly complicated. Perhaps we should cache season thumbs by showtitle and season number,
      //       rather than bothering with show path and the localized strings involved?
      if (m_movieItem->GetVideoInfoTag()->m_iSeason > -1)
      {
        CStdString label;
        if (m_movieItem->GetVideoInfoTag()->m_iSeason == 0)
          label = g_localizeStrings.Get(20381);
        else
          label.Format(g_localizeStrings.Get(20358), m_movieItem->GetVideoInfoTag()->m_iSeason);
        CFileItem season(label);
        season.m_bIsFolder = true;
        // grab show path
        CVideoDatabase db;
        if (db.Open())
        {
          CFileItemList items;
          CStdString where = db.FormatSQL("where c%02d='%s'", VIDEODB_ID_TV_TITLE, m_movieItem->GetVideoInfoTag()->m_strShowTitle.c_str());
          if (db.GetTvShowsByWhere("", where, items) && items.Size())
            season.GetVideoInfoTag()->m_strPath = items[0]->GetVideoInfoTag()->m_strPath;
          db.Close();
        }
        season.SetCachedSeasonThumb();
        if (season.HasThumbnail())
          m_movieItem->SetProperty("seasonthumb", season.GetThumbnailImage());
      }
    }
    else
      m_castList->SetContent("movies");
  }
  
#endif
}

void CGUIWindowVideoInfo::Update()
{
  CStdString strTmp;
  strTmp = m_movieItem->GetVideoInfoTag()->m_strTitle; strTmp.Trim();
  SetLabel(CONTROL_TITLE, strTmp);

  strTmp = m_movieItem->GetVideoInfoTag()->m_strDirector; strTmp.Trim();
  SetLabel(CONTROL_DIRECTOR, strTmp);

  strTmp = m_movieItem->GetVideoInfoTag()->m_strStudio; strTmp.Trim();
  SetLabel(CONTROL_STUDIO, strTmp);

  strTmp = m_movieItem->GetVideoInfoTag()->m_strWritingCredits; strTmp.Trim();
  SetLabel(CONTROL_CREDITS, strTmp);

  strTmp = m_movieItem->GetVideoInfoTag()->m_strGenre; strTmp.Trim();
  SetLabel(CONTROL_GENRE, strTmp);

  strTmp = m_movieItem->GetVideoInfoTag()->m_strTagLine; strTmp.Trim();
  SetLabel(CONTROL_TAGLINE, strTmp);

  strTmp = m_movieItem->GetVideoInfoTag()->m_strPlotOutline; strTmp.Trim();
  SetLabel(CONTROL_PLOTOUTLINE, strTmp);

  strTmp = m_movieItem->GetVideoInfoTag()->m_strTrailer; strTmp.Trim();
  SetLabel(CONTROL_TRAILER, strTmp);

  strTmp = m_movieItem->GetVideoInfoTag()->m_strMPAARating; strTmp.Trim();
  SetLabel(CONTROL_MPAARATING, strTmp);

  CStdString strTop250;
  if (m_movieItem->GetVideoInfoTag()->m_iTop250)
    strTop250.Format("%i", m_movieItem->GetVideoInfoTag()->m_iTop250);
  SetLabel(CONTROL_TOP250, strTop250);

  CStdString strYear;
  if (m_movieItem->GetVideoInfoTag()->m_iYear)
    strYear.Format("%i", m_movieItem->GetVideoInfoTag()->m_iYear);
  else
    strYear = g_infoManager.GetItemLabel(m_movieItem.get(),LISTITEM_PREMIERED);
  SetLabel(CONTROL_YEAR, strYear);

  CStdString strRating_And_Votes;
  if (m_movieItem->GetVideoInfoTag()->m_fRating != 0.0f)  // only non-zero ratings are of interest
    strRating_And_Votes.Format("%03.1f (%s %s)", m_movieItem->GetVideoInfoTag()->m_fRating, m_movieItem->GetVideoInfoTag()->m_strVotes, g_localizeStrings.Get(20350));
  SetLabel(CONTROL_RATING_AND_VOTES, strRating_And_Votes);

  strTmp = m_movieItem->GetVideoInfoTag()->m_strRuntime; strTmp.Trim();
  SetLabel(CONTROL_RUNTIME, strTmp);

  // setup plot text area
  strTmp = m_movieItem->GetVideoInfoTag()->m_strPlot;
  if (!(!m_movieItem->GetVideoInfoTag()->m_strShowTitle.IsEmpty() && m_movieItem->GetVideoInfoTag()->m_iSeason == 0)) // dont apply to tvshows
    if (m_movieItem->GetVideoInfoTag()->m_playCount == 0 && g_guiSettings.GetBool("videolibrary.hideplots"))
      strTmp = g_localizeStrings.Get(20370);

  strTmp.Trim();
  SetLabel(CONTROL_TEXTAREA, strTmp);

  CGUIMessage msg(GUI_MSG_LABEL_BIND, GetID(), CONTROL_LIST, 0, 0, m_castList);
  OnMessage(msg);

  if (m_bViewReview)
  {
    if (!m_movieItem->GetVideoInfoTag()->m_strArtist.IsEmpty())
    {
      SET_CONTROL_LABEL(CONTROL_BTN_TRACKS, 133);
    }
    else
    {
      SET_CONTROL_LABEL(CONTROL_BTN_TRACKS, 206);
    }

    SET_CONTROL_HIDDEN(CONTROL_LIST);
    SET_CONTROL_VISIBLE(CONTROL_TEXTAREA);
  }
  else
  {
    SET_CONTROL_LABEL(CONTROL_BTN_TRACKS, 207);

    SET_CONTROL_HIDDEN(CONTROL_TEXTAREA);
    SET_CONTROL_VISIBLE(CONTROL_LIST);
  }

  // Check for resumability
  CGUIWindowVideoFiles *window = (CGUIWindowVideoFiles *)m_gWindowManager.GetWindow(WINDOW_VIDEO_FILES);
  if (window && window->GetResumeItemOffset(m_movieItem.get()) > 0)
  {
    CONTROL_ENABLE(CONTROL_BTN_RESUME);
  }
  else
  {
    CONTROL_DISABLE(CONTROL_BTN_RESUME);
  }

  if (m_movieItem->GetVideoInfoTag()->m_strEpisodeGuide.IsEmpty()) // disable the play button for tv show info
  {
    CONTROL_ENABLE(CONTROL_BTN_PLAY);
  }
  else
  {
    CONTROL_DISABLE(CONTROL_BTN_PLAY);
  }

  // tell our GUI to completely reload all controls (as some of them
  // are likely to have had this image in use so will need refreshing)
  CGUIMessage reload(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_REFRESH_THUMBS);
  g_graphicsContext.SendMessage(reload);
}

void CGUIWindowVideoInfo::Refresh()
{
  Update();
}
bool CGUIWindowVideoInfo::NeedRefresh() const
{
  return m_bRefresh;
}

bool CGUIWindowVideoInfo::RefreshAll() const
{
  return m_bRefreshAll;
}

/// \brief Search the current directory for a string got from the virtual keyboard
void CGUIWindowVideoInfo::OnSearch(CStdString& strSearch)
{
#if 0
  if (m_dlgProgress)
  {
    m_dlgProgress->SetHeading(194);
    m_dlgProgress->SetLine(0, strSearch);
    m_dlgProgress->SetLine(1, "");
    m_dlgProgress->SetLine(2, "");
    m_dlgProgress->StartModal();
    m_dlgProgress->Progress();
  }
  CFileItemList items;
  DoSearch(strSearch, items);

  if (items.Size())
  {
    CGUIDialogSelect* pDlgSelect = (CGUIDialogSelect*)m_gWindowManager.GetWindow(WINDOW_DIALOG_SELECT);
    pDlgSelect->Reset();
    pDlgSelect->SetHeading(283);
    items.Sort(SORT_METHOD_LABEL, SORT_ORDER_ASC);

    for (int i = 0; i < (int)items.Size(); i++)
    {
      CFileItemPtr pItem = items[i];
      pDlgSelect->Add(pItem->GetLabel());
    }

    pDlgSelect->DoModal();

    int iItem = pDlgSelect->GetSelectedLabel();
    if (iItem < 0)
    {
      if (m_dlgProgress) m_dlgProgress->Close();
      return ;
    }

    CFileItem* pSelItem = new CFileItem(*items[iItem]);

    OnSearchItemFound(pSelItem);

    delete pSelItem;
    if (m_dlgProgress) m_dlgProgress->Close();
  }
  else
  {
    if (m_dlgProgress) m_dlgProgress->Close();
    CGUIDialogOK::ShowAndGetInput(194, 284, 0, 0);
  }
#endif
}

/// \brief Make the actual search for the OnSearch function.
/// \param strSearch The search string
/// \param items Items Found
void CGUIWindowVideoInfo::DoSearch(CStdString& strSearch, CFileItemList& items)
{
#if 0
  VECMOVIES movies;
  CVideoDatabase db;
  if (!db.Open())
    return;

  db.GetMoviesByActor(strSearch, movies);
  for (int i = 0; i < (int)movies.size(); ++i)
  {
    CStdString strItem;
    strItem.Format("[%s] %s (%i)", g_localizeStrings.Get(20338), movies[i].m_strTitle, movies[i].m_iYear);  // Movie
    CFileItemPtr pItem(new CFileItem(strItem));
    *pItem->GetVideoInfoTag() = movies[i];
    pItem->m_strPath = movies[i].m_strFileNameAndPath;
    items.Add(pItem);
  }
  movies.clear();
  db.GetTvShowsByActor(strSearch, movies);
  for (int i = 0; i < (int)movies.size(); ++i)
  {
    CStdString strItem;
    strItem.Format("[%s] %s", g_localizeStrings.Get(20364), movies[i].m_strTitle);  // Movie
    CFileItemPtr pItem(new CFileItem(strItem));
    *pItem->GetVideoInfoTag() = movies[i];
    pItem->m_strPath.Format("videodb://1/%u",movies[i].m_iDbId);
    items.Add(pItem);
  }
  movies.clear();
  db.GetEpisodesByActor(strSearch, movies);
  for (int i = 0; i < (int)movies.size(); ++i)
  {
    CStdString strItem;
    strItem.Format("[%s] %s", g_localizeStrings.Get(20359), movies[i].m_strTitle);  // Movie
    CFileItemPtr pItem(new CFileItem(strItem));
    *pItem->GetVideoInfoTag() = movies[i];
    pItem->m_strPath = movies[i].m_strFileNameAndPath;
    items.Add(pItem);
  }

  CFileItemList mvids;
  db.GetMusicVideosByArtist(strSearch, mvids);
  for (int i = 0; i < (int)mvids.Size(); ++i)
  {
    CStdString strItem;
    strItem.Format("[%s] %s", g_localizeStrings.Get(20391), mvids[i]->GetVideoInfoTag()->m_strTitle);  // Movie
    CFileItemPtr pItem(new CFileItem(strItem));
    *pItem->GetVideoInfoTag() = *mvids[i]->GetVideoInfoTag();
    pItem->m_strPath = mvids[i]->GetVideoInfoTag()->m_strFileNameAndPath;
    items.Add(pItem);
  }
  db.Close();
  
#endif
}

VIDEODB_CONTENT_TYPE CGUIWindowVideoInfo::GetContentType(const CFileItem *pItem) const
{
  VIDEODB_CONTENT_TYPE type = VIDEODB_CONTENT_MOVIES;
  if (pItem->HasVideoInfoTag() && !pItem->GetVideoInfoTag()->m_strShowTitle.IsEmpty()) // tvshow
    type = VIDEODB_CONTENT_TVSHOWS;
  if (pItem->HasVideoInfoTag() && pItem->GetVideoInfoTag()->m_iSeason > -1 && !pItem->m_bIsFolder) // episode
    type = VIDEODB_CONTENT_EPISODES;
  if (pItem->HasVideoInfoTag() && !pItem->GetVideoInfoTag()->m_strArtist.IsEmpty())
    type = VIDEODB_CONTENT_MUSICVIDEOS;
  return type;
}

/// \brief React on the selected search item
/// \param pItem Search result item
void CGUIWindowVideoInfo::OnSearchItemFound(const CFileItem* pItem)
{
}

void CGUIWindowVideoInfo::ClearCastList()
{
  CGUIMessage msg(GUI_MSG_LABEL_RESET, GetID(), CONTROL_LIST);
  OnMessage(msg);
  m_castList->Clear();
}

void CGUIWindowVideoInfo::Play(bool resume)
{
  CFileItem movie(m_movieItem->m_strPath, false);
  printf("Path: %s\n", m_movieItem->m_strPath.c_str());
  
  CGUIWindowVideoFiles* pWindow = (CGUIWindowVideoFiles*)m_gWindowManager.GetWindow(WINDOW_VIDEO_FILES);
  if (pWindow)
  {
    // close our dialog
    Close(true);
    if (resume)
      movie.m_lStartOffset = STARTOFFSET_RESUME;
    pWindow->PlayMovie(&movie);
  }
}

void CGUIWindowVideoInfo::OnGetThumb()
{
  string newPoster = OnGetMedia("posters", m_movieItem->GetThumbnailImage(), 20016);
  if (newPoster.size() > 0)
  {
    string newPosterFile = CFileItem::GetCachedPlexMediaServerThumb(newPoster);
    bool   success = true;
    
    if (CFile::Exists(newPosterFile) == false)
      success = AsyncDownloadMedia(newPoster, newPosterFile);

    if (success)
    {
      m_movieItem->SetThumbnailImage(newPosterFile);
      
      // Tell our GUI to completely reload all controls.
      CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_REFRESH_THUMBS);
      g_graphicsContext.SendMessage(msg);
    }
  }
}

extern bool Cocoa_IsHostLocal(const string& host);

string CGUIWindowVideoInfo::OnGetMedia(const string& mediaType, const string& currentCachedMedia, int label)
{
  CFileItemList items;

  // Current one.
  if (currentCachedMedia.size() > 0 && CFile::Exists(currentCachedMedia))
  {
    CFileItemPtr itemCurrent(new CFileItem("media://Current", false));
    itemCurrent->SetThumbnailImage(currentCachedMedia);
    itemCurrent->SetLabel(g_localizeStrings.Get(label));
    items.Add(itemCurrent);
  }

  // Get a list of available ones.
  CFileItemList fileItems;
  CPlexDirectory plexDir;
  string url = m_movieItem->GetProperty("rootKey").c_str() + string("/") + mediaType;
  plexDir.GetDirectory(url, fileItems);

  m_mediaMap.clear();
  for (int i=0; i<fileItems.Size(); i++)
  {
    CFileItemPtr item = fileItems.Get(i);
    if (item->GetProperty("selected") == "0")
    {
      m_mediaMap[item->m_strPath] = item->GetProperty("ratingKey");
      item->SetLabel("");
      items.Add(item);
    }
  }
  
  // Have the user pick one.
  CStdString result;
  VECSOURCES sources(g_settings.m_videoSources);
  g_mediaManager.GetLocalDrives(sources);
  if (!CGUIDialogFileBrowser::ShowAndGetImage(items, sources, g_localizeStrings.Get(label), result, false))
    return "";
  
  if (result == "media://Current")
    return "";
  
  // Take the result and send it back, to the singular URL (e.g. PUT .../art)
  CStdString selectedKey = m_mediaMap[result];
  CUtil::URLEncode(selectedKey);

  CURL finalURL(CStdString(url.substr(0, url.size()-1)) + "?url=" + selectedKey);
  finalURL.SetProtocol("http");
  finalURL.SetPort(32400);

  CHTTP set;
  CStdString strData;
  set.Open(finalURL.GetURL(), "PUT", 0);
  set.ReadData(strData);
  set.Close();
  
  // Compute the new URL.
  finalURL.SetFileName(strData.substr(1));
  finalURL.SetOptions("");

  bool local = Cocoa_IsHostLocal(finalURL.GetHostName());
  return CPlexDirectory::BuildImageURL(url, finalURL.GetURL(), local);
}

// Allow user to select a Banner
void CGUIWindowVideoInfo::OnGetBanner()
{
  string newBanner = OnGetMedia("banners", m_movieItem->GetCachedPlexMediaServerBanner(), 20036);
  if (newBanner.size() > 0)
  {
    string newBannerFile = CFileItem::GetCachedPlexMediaServerThumb(newBanner);
    bool   success = true;
    
    if (CFile::Exists(newBannerFile) == false)
      success = AsyncDownloadMedia(newBanner, newBannerFile);

    if (success)
    {
      m_movieItem->SetQuickBanner(newBanner);
      m_movieItem->SetProperty("banner_image", newBannerFile);
      
      // Tell our GUI to completely reload all controls.
      CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_REFRESH_THUMBS);
      g_graphicsContext.SendMessage(msg);
    }
  }
}

// Allow user to select a Fanart
void CGUIWindowVideoInfo::OnGetFanart()
{
  string newFanart = OnGetMedia("arts", m_movieItem->GetCachedFanart(), 20035);
  if (newFanart.size() > 0)
  {
    string newFanartFile = CFileItem::GetCachedPlexMediaServerFanart(newFanart);
    bool   success = true;
    
    if (CFile::Exists(newFanartFile) == false)
      success = AsyncDownloadMedia(newFanart, newFanartFile);

    if (success)
    {
      m_movieItem->SetQuickFanart(newFanart);
      m_movieItem->SetProperty("fanart_image", newFanartFile);
      
      // Tell our GUI to completely reload all controls.
      CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_REFRESH_THUMBS);
      g_graphicsContext.SendMessage(msg);
    }
  }
}

bool CGUIWindowVideoInfo::AsyncDownloadMedia(const string& remoteFile, const string& localFile)
{
  CStdString tempFile = _P("Z:\\fanart_download.jpg");
  CAsyncFileCopy downloader;
  bool success = downloader.Copy(remoteFile, tempFile, g_localizeStrings.Get(13413));
  CPicture pic;
  pic.CacheImage(tempFile, localFile);
  CFile::Delete(tempFile);
  
  return success;
}

void CGUIWindowVideoInfo::PlayTrailer()
{
  CFileItem item;
  item.m_strPath = m_movieItem->GetVideoInfoTag()->m_strTrailer;
  *item.GetVideoInfoTag() = *m_movieItem->GetVideoInfoTag();
  item.GetVideoInfoTag()->m_strTitle.Format("%s (%s)",m_movieItem->GetVideoInfoTag()->m_strTitle.c_str(),g_localizeStrings.Get(20410));
  item.SetThumbnailImage(m_movieItem->GetThumbnailImage());
  item.GetVideoInfoTag()->m_iDbId = -1;

  // Close the dialog.
  Close(true);
  g_application.getApplicationMessenger().PlayFile(item);
}

void CGUIWindowVideoInfo::SetLabel(int iControl, const CStdString &strLabel)
{
  if (strLabel.IsEmpty())
  {
    SET_CONTROL_LABEL(iControl, 416);  // "Not available"
  }
  else
  {
    SET_CONTROL_LABEL(iControl, strLabel);
  }
}

const CStdString& CGUIWindowVideoInfo::GetThumbnail() const
{
  return m_movieItem->GetThumbnailImage();
}

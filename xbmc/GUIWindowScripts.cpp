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
#include "GUIWindowScripts.h"
#include "Util.h"
#ifdef HAS_PYTHON
#include "lib/libPython/XBPython.h"
#endif
#include "GUIWindowScriptsInfo.h"
#include "GUIWindowManager.h"
#include "FileSystem/File.h"
#include "FileItem.h"
#include "Directory.h"
#include "MultiPathDirectory.h"
#include <vector>
#include "CocoaUtils.h"

using namespace XFILE;
using namespace std;
using namespace DIRECTORY;

#define CONTROL_BTNVIEWASICONS     2
#define CONTROL_BTNSORTBY          3
#define CONTROL_BTNSORTASC         4
#define CONTROL_LIST              50
#define CONTROL_THUMBS            51
#define CONTROL_LABELFILES        12

CGUIWindowScripts::CGUIWindowScripts()
    : CGUIMediaWindow(WINDOW_SCRIPTS, "MyScripts.xml")
{
  m_bViewOutput = false;
  m_scriptSize = 0;
}

CGUIWindowScripts::~CGUIWindowScripts()
{
}

bool CGUIWindowScripts::OnAction(const CAction &action)
{
  if (action.wID == ACTION_SHOW_INFO)
  {
    OnInfo();
    return true;
  }
  return CGUIMediaWindow::OnAction(action);
}

bool CGUIWindowScripts::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_WINDOW_INIT:
    {
      if (m_vecItems->m_strPath == "?")
      {
        // Combine scripts from bundle & app support folders
        vector<CStdString> scriptPaths;
        
        CStdString userScripts = _P("U:\\scripts");
        CStdString bundleScripts = _P("Q:\\scripts");
        
        if (CDirectory::Exists(userScripts))
          scriptPaths.push_back(userScripts);
        if (CDirectory::Exists(bundleScripts))
          scriptPaths.push_back(bundleScripts);
        
        m_vecItems->m_strPath = CMultiPathDirectory::ConstructMultiPath(scriptPaths);
        scriptPaths.clear();
      }
      return CGUIMediaWindow::OnMessage(message);
    }
    break;
  }
  return CGUIMediaWindow::OnMessage(message);
}

bool CGUIWindowScripts::Update(const CStdString &strDirectory)
{
  // Look if baseclass can handle it
  if (!CGUIMediaWindow::Update(strDirectory))
    return false;

#ifdef HAS_PYTHON
  /* check if any python scripts are running. If true, place "(Running)" after the item.
   * since stopping a script can take up to 10 seconds or more,we display 'stopping'
   * after the filename for now.
   */
  int iSize = g_pythonParser.ScriptsSize();
  for (int i = 0; i < iSize; i++)
  {
    int id = g_pythonParser.GetPythonScriptId(i);
    if (g_pythonParser.isRunning(id))
    {
      const char* filename = g_pythonParser.getFileName(id);

      for (int i = 0; i < m_vecItems->Size(); i++)
      {
        CFileItemPtr pItem = m_vecItems->Get(i);
        if (pItem->m_strPath == filename)
        {
          char tstr[1024];
          strcpy(tstr, pItem->GetLabel());
          if (g_pythonParser.isStopping(id))
            strcat(tstr, " (Stopping)");
          else
            strcat(tstr, " (Running)");
          pItem->SetLabel(tstr);
        }
      }
    }
  }
#endif

  return true;
}

bool CGUIWindowScripts::OnPlayMedia(int iItem)
{
  CFileItemPtr pItem=m_vecItems->Get(iItem);
  CStdString strPath = pItem->m_strPath;

	if (CUtil::GetExtension(pItem->m_strPath) == ".applescript")
	{
		Cocoa_ExecAppleScriptFile(pItem->m_strPath.c_str());
		return true;
	}
	
#ifdef HAS_PYTHON
  /* execute script...
    * if script is already running do not run it again but stop it.
    */
  int id = g_pythonParser.getScriptId(strPath);
  if (id != -1)
  {
    /* if we are here we already know that this script is running.
      * But we will check it again to be sure :)
      */
    if (g_pythonParser.isRunning(id))
    {
      g_pythonParser.stopScript(id);

      // update items
      int selectedItem = m_viewControl.GetSelectedItem();
      Update(m_vecItems->m_strPath);
      m_viewControl.SetSelectedItem(selectedItem);
      return true;
    }
  }
  g_pythonParser.evalFile(strPath);
#endif

  return true;
}

void CGUIWindowScripts::OnInfo()
{
  CGUIWindowScriptsInfo* pDlgInfo = (CGUIWindowScriptsInfo*)m_gWindowManager.GetWindow(WINDOW_SCRIPTS_INFO);
  if (pDlgInfo) pDlgInfo->DoModal();
}

void CGUIWindowScripts::Render()
{
#ifdef HAS_PYTHON
  // update control_list / control_thumbs if one or more scripts have stopped / started
  if (g_pythonParser.ScriptsSize() != m_scriptSize)
  {
    int selectedItem = m_viewControl.GetSelectedItem();
    Update(m_vecItems->m_strPath);
    m_viewControl.SetSelectedItem(selectedItem);
    m_scriptSize = g_pythonParser.ScriptsSize();
  }
#endif

  CGUIWindow::Render();
}

bool CGUIWindowScripts::GetDirectory(const CStdString& strDirectory, CFileItemList& items)
{
  if (!CGUIMediaWindow::GetDirectory(strDirectory,items))
    return false;

  // flatten any folders
  for (int i = 0; i < items.Size(); i++)
  {
    CFileItemPtr item = items[i];
    if (item->m_bIsFolder && !item->IsParentFolder() && !item->m_bIsShareOrDrive && !item->GetLabel().Left(1).Equals("."))
    { // folder item - let's check for a default.py file, and flatten if we have one
      CStdString defaultPY;
      
      // If using a multipath directory, get the first path
      CStdString firstPath;
      if (CUtil::IsMultiPath(item->m_strPath)) firstPath = CMultiPathDirectory::GetFirstPath(item->m_strPath);
      else firstPath = item->m_strPath;
      
      CUtil::AddFileToFolder(firstPath, "default.py", defaultPY);
      if (!CFile::Exists(defaultPY)) {
         CUtil::AddFileToFolder(firstPath, "Default.py", defaultPY);
         if (!CFile::Exists(defaultPY)) {
            CUtil::AddFileToFolder(firstPath, "DEFAULT.PY", defaultPY);
         }
      }

      if (CFile::Exists(defaultPY))
      { // yes, format the item up
        item->m_strPath = defaultPY;
        item->m_bIsFolder = false;
        item->FillInDefaultIcon();
        item->SetLabelPreformated(true);
      }
    }
    if (item->GetLabel().Equals("autoexec.py") || (item->GetLabel().Left(1).Equals(".") && !item->IsParentFolder()))
    {
      items.Remove(i);
      i--;
    }
		// Remove extension & set thumbnail AppleScripts
		CStdString itemLabel = item->GetLabel();
		if (CUtil::GetExtension(itemLabel) == ".applescript")
		{
			CUtil::RemoveExtension(itemLabel);
			item->SetLabel(itemLabel);
			item->SetThumbnailImage(Cocoa_GetIconFromBundle("/Applications/AppleScript/Script Editor.app", "SECompiledScript"));
		}
  }

  items.SetProgramThumbs();

  return true;
}

void CGUIWindowScripts::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
  CGUIMediaWindow::GetContextButtons(itemNumber, buttons);
  buttons.Add(CONTEXT_BUTTON_INFO, 654);
}

bool CGUIWindowScripts::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
  if (button == CONTEXT_BUTTON_INFO)
  {
    OnInfo();
    return true;
  }
  return CGUIMediaWindow::OnContextButton(itemNumber, button);
}


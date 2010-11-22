/******************************************************************************
 * Project:  wxGIS (GIS Remote)
 * Purpose:  wxGxRemoteServersUI class.
 * Author:   Bishop (aka Barishnikov Dmitriy), polimax@mail.ru
 ******************************************************************************
*   Copyright (C) 2010 Bishop
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "wxgis/remoteserverui/gxremoteserversui.h"
#include "wxgis/remoteserverui/gxremoteserverui.h"
#include "wxgis/catalogui/gxcatalogui.h"

#include "../../art/remoteservers_16.xpm"
#include "../../art/remoteservers_48.xpm"
#include "../../art/remoteserver_16.xpm"
#include "../../art/remoteserver_48.xpm"

IMPLEMENT_DYNAMIC_CLASS(wxGxRemoteServersUI, wxGxRemoteServers)

wxGxRemoteServersUI::wxGxRemoteServersUI(void) : wxGxRemoteServers()
{
    m_RemServ16 = wxIcon(remoteserver_16_xpm);
    m_RemServ48 = wxIcon(remoteserver_48_xpm);
    wxBitmap RemServDsbld16 = wxBitmap(remoteserver_16_xpm).ConvertToImage().ConvertToGreyscale();
    wxBitmap RemServDsbld48 = wxBitmap(remoteserver_48_xpm).ConvertToImage().ConvertToGreyscale();
    m_RemServDsbld16.CopyFromBitmap(RemServDsbld16);
    m_RemServDsbld48.CopyFromBitmap(RemServDsbld48);
}

wxGxRemoteServersUI::~wxGxRemoteServersUI(void)
{
}

wxIcon wxGxRemoteServersUI::GetLargeImage(void)
{
	return wxIcon(remoteservers_48_xpm);
}

wxIcon wxGxRemoteServersUI::GetSmallImage(void)
{
	return wxIcon(remoteservers_16_xpm);
}

void wxGxRemoteServersUI::EmptyChildren(void)
{
	for(size_t i = 0; i < m_Children.size(); i++)
	{
        wxGxCatalogUI* pCatalog = dynamic_cast<wxGxCatalogUI*>(m_pCatalog);
        if(pCatalog)
        {
            IGxSelection* pSel = pCatalog->GetSelection();
            if(pSel)
                pSel->Unselect(m_Children[i], IGxSelection::INIT_ALL);
        }
		m_Children[i]->Detach();
		wxDELETE(m_Children[i]);
	}
	m_Children.clear();
	m_bIsChildrenLoaded = false;
}

void wxGxRemoteServersUI::LoadChildren(wxXmlNode* pConf)
{
	if(!pConf || m_bIsChildrenLoaded)
		return;	
	wxXmlNode* pChild = pConf->GetChildren();
	while(pChild)
	{
		wxGxRemoteServerUI* pServerConn = new wxGxRemoteServerUI(pChild, m_RemServ16, m_RemServ48, m_RemServDsbld16, m_RemServDsbld48);
		IGxObject* pGxObj = static_cast<IGxObject*>(pServerConn);
		if(!AddChild(pGxObj))
			wxDELETE(pGxObj);
		pChild = pChild->GetNext();
	}
	m_bIsChildrenLoaded = true;
}


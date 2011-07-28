/******************************************************************************
 * Project:  wxGIS (GIS Map)
 * Purpose:  Main application class.
 * Author:   Bishop (aka Barishnikov Dmitriy), polimax@mail.ru
 ******************************************************************************
*   Copyright (C) 20011 Bishop
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

#pragma once

#include "wxgis/base.h"
#include "wxgis/core/config.h"

#include <wx/app.h>
#include <wx/dynload.h>
#include <wx/dynlib.h>

#include <map>

class wxGISMapApp :
	public wxApp
{
public:
	wxGISMapApp(void);
	~wxGISMapApp(void);
	virtual bool OnInit();

    typedef std::map<wxString, wxDynamicLibrary*> LIBMAP;
protected:
	virtual void LoadLibs(wxXmlNode* pRootNode);
	virtual void UnLoadLibs(void);
private:
	wxGISAppConfig* m_pConfig;
    LIBMAP m_LibMap;
};

DECLARE_APP(wxGISMapApp);

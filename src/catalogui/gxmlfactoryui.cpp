/******************************************************************************
 * Project:  wxGIS (GIS Catalog)
 * Purpose:  wxGxMLFactoryUI class.
 * Author:   Bishop (aka Barishnikov Dmitriy), polimax@mail.ru
 ******************************************************************************
*   Copyright (C) 2010-2011 Bishop
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

#include "wxgis/catalogui/gxmlfactoryui.h"
#include "wxgis/catalogui/gxkmldatasetui.h"
#include "wxgis/catalogui/gxdatasetui.h"

#include "../../art/dxf_dset_16.xpm"
#include "../../art/dxf_dset_48.xpm"
#include "../../art/kml_dset_16.xpm"
#include "../../art/kml_dset_48.xpm"
#include "../../art/kmz_dset_16.xpm"
#include "../../art/kmz_dset_48.xpm"
#include "../../art/gml_dset_16.xpm"
#include "../../art/gml_dset_48.xpm"
#include "../../art/kml_subdset_16.xpm"
#include "../../art/kml_subdset_48.xpm"

IMPLEMENT_DYNAMIC_CLASS(wxGxMLFactoryUI, wxGxMLFactory)

wxGxMLFactoryUI::wxGxMLFactoryUI(void) : wxGxMLFactory()
{
    m_SmallDXFIcon = wxIcon(dxf_dset_16_xpm);
    m_LargeDXFIcon = wxIcon(dxf_dset_48_xpm);
    m_SmallKMLIcon = wxIcon(kml_dset_16_xpm);
    m_LargeKMLIcon = wxIcon(kml_dset_48_xpm);
    m_SmallKMZIcon = wxIcon(kmz_dset_16_xpm);
    m_LargeKMZIcon = wxIcon(kmz_dset_48_xpm);
    m_SmallGMLIcon = wxIcon(gml_dset_16_xpm);
    m_LargeGMLIcon = wxIcon(gml_dset_48_xpm);
    m_LargeSubKMLIcon = wxIcon(kml_subdset_48_xpm);
    m_SmallSubKMLIcon = wxIcon(kml_subdset_16_xpm);

}

wxGxMLFactoryUI::~wxGxMLFactoryUI(void)
{
}

IGxObject* wxGxMLFactoryUI::GetGxDataset(CPLString path, wxString name, wxGISEnumVectorDatasetType type)
{
    switch(type)
    {
    case enumVecKML:
        {
        wxGxKMLDatasetUI* pDataset = new wxGxKMLDatasetUI(path, name, enumVecKML, m_LargeKMLIcon, m_SmallKMLIcon, m_LargeSubKMLIcon, m_SmallSubKMLIcon);
        pDataset->SetEncoding(wxFONTENCODING_UTF8);
        return static_cast<IGxObject*>(pDataset);
        }
    case enumVecKMZ:
        {
        wxGxKMLDatasetUI* pDataset = new wxGxKMLDatasetUI(path, name, enumVecKMZ, m_LargeKMZIcon, m_SmallKMZIcon, m_LargeSubKMLIcon, m_SmallSubKMLIcon);
        pDataset->SetEncoding(wxFONTENCODING_UTF8);
        return static_cast<IGxObject*>(pDataset);
        }
    case enumVecDXF:
        {
	    wxGxFeatureDatasetUI* pDataset = new wxGxFeatureDatasetUI(path, name, enumVecDXF, m_LargeDXFIcon, m_SmallDXFIcon);
        return static_cast<IGxObject*>(pDataset);
        }
    case enumVecGML:
        {
	    wxGxKMLDatasetUI* pDataset = new wxGxKMLDatasetUI(path, name, enumVecGML, m_LargeGMLIcon, m_SmallGMLIcon, m_LargeGMLIcon, m_SmallGMLIcon);
        return static_cast<IGxObject*>(pDataset);
        }
    default:
        return NULL;
    }
    return NULL;
}
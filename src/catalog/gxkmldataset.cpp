/******************************************************************************
 * Project:  wxGIS (GIS Catalog)
 * Purpose:  wxGxKMLDataset class.
 * Author:   Bishop (aka Barishnikov Dmitriy), polimax@mail.ru
 ******************************************************************************
*   Copyright (C) 2009-2011 Bishop
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
#include "wxgis/catalog/gxkmldataset.h"
#include "wxgis/datasource/featuredataset.h"
#include "wxgis/datasource/sysop.h"

//--------------------------------------------------------------
//class wxGxKMLDataset
//--------------------------------------------------------------

wxGxKMLDataset::wxGxKMLDataset(wxString Path, wxString Name, wxGISEnumVectorDatasetType Type)
{
	m_type = Type;

	m_sName = Name;
	m_sPath = Path;

	//m_pwxGISDataset = NULL;

    m_Encoding = wxFONTENCODING_UTF8;
    m_pPathEncoding = wxConvCurrent;

    m_bIsChildrenLoaded = false;

}

wxGxKMLDataset::~wxGxKMLDataset(void)
{
    //wsDELETE(m_pwxGISDataset);
}

void wxGxKMLDataset::SetEncoding(wxFontEncoding Encoding)
{
    m_Encoding = Encoding;
}

void wxGxKMLDataset::Detach(void)
{
    EmptyChildren();
}

wxString wxGxKMLDataset::GetBaseName(void)
{
    wxFileName FileName(m_sName);
    FileName.SetEmptyExt();
    return FileName.GetName();
}

wxString wxGxKMLDataset::GetCategory(void)
{
	return wxString(_("KML Dataset"));
}

bool wxGxKMLDataset::Delete(void)
{
    bool bRet = false;
    if(m_pwxGISDataset)
    {
        EmptyChildren();
        wxGISFeatureDatasetSPtr pDSet = boost::dynamic_pointer_cast<wxGISFeatureDataset>(m_pwxGISDataset);
        if(pDSet)
            bRet = pDSet->Delete();
    }
    else
    {
        wxGISFeatureDatasetSPtr pwxGISFeatureDataset = boost::make_shared<wxGISFeatureDataset>(m_sPath, m_type);
        bRet = pwxGISFeatureDataset->Delete();
    }

    //LoadChildren();
    //wxGISFeatureDataset* pDSet = dynamic_cast<wxGISFeatureDataset*>(m_pwxGISDataset);
    //if(!pDSet)
    //    return false;

    //pDSet->Reference();
    //bool bRet = pDSet->Delete();
    if(bRet)
	{
		IGxObjectContainer* pGxObjectContainer = dynamic_cast<IGxObjectContainer*>(m_pParent);
		if(pGxObjectContainer == NULL)
			return false;
		return pGxObjectContainer->DeleteChild(this);
	}
	else
    {
        const char* err = CPLGetLastErrorMsg();
        wxLogError(_("Delete failed! GDAL error: %s, file '%s'"), wgMB2WX(err), m_sPath.c_str());
		return false;
    }
    return false;
}

bool wxGxKMLDataset::Rename(wxString NewName)
{
	NewName = ClearExt(NewName);
	wxFileName PathName(m_sPath);
	PathName.SetName(NewName);

	wxString m_sNewPath = PathName.GetFullPath();

	EmptyChildren();
    if(RenameFile(m_sPath, m_sNewPath))
	{
		m_sPath = m_sNewPath;
		m_sName = NewName;
		m_pCatalog->ObjectChanged(this);
		Refresh();
		return true;
	}
	else
    {
        const char* err = CPLGetLastErrorMsg();
        wxLogError(_("Rename failed! GDAL error: %s, file '%s'"), wgMB2WX(err), m_sPath.c_str());
		return false;
    }
}

void wxGxKMLDataset::EmptyChildren(void)
{
	for(size_t i = 0; i < m_Children.size(); i++)
	{
		m_Children[i]->Detach();
		wxDELETE( m_Children[i] );
	}
    m_Children.clear();
	m_bIsChildrenLoaded = false;
}

void wxGxKMLDataset::LoadChildren(void)
{
	if(m_bIsChildrenLoaded)
		return;

	if(m_pwxGISDataset == NULL)
	{
        wxGISFeatureDatasetSPtr pwxGISFeatureDataset = boost::make_shared<wxGISFeatureDataset>(m_sPath, m_type);

        if(!pwxGISFeatureDataset->Open())
        {
		    const char* err = CPLGetLastErrorMsg();
		    wxString sErr = wxString::Format(_("Open failed! GDAL error: %s"), wgMB2WX(err));
            wxLogError(sErr);

            //wxDELETE(pwxGISFeatureDataset);
			return;
        }

        m_pwxGISDataset = boost::static_pointer_cast<wxGISDataset>(pwxGISFeatureDataset);
        m_pwxGISDataset->SetSubType(m_type);
        //m_pwxGISDataset->Reference();
	}

    for(size_t i = 0; i < m_pwxGISDataset->GetSubsetsCount(); i++)
    {
        wxGISFeatureDatasetSPtr pwxGISFeatureSuDataset = boost::dynamic_pointer_cast<wxGISFeatureDataset>(m_pwxGISDataset->GetSubset(i));
        pwxGISFeatureSuDataset->SetSubType(m_type);
        wxGxKMLSubDataset* pGxSubDataset = new wxGxKMLSubDataset(pwxGISFeatureSuDataset->GetName(), pwxGISFeatureSuDataset, m_type);
		bool ret_code = AddChild(pGxSubDataset);
		if(!ret_code)
			wxDELETE(pGxSubDataset);
	}
	m_bIsChildrenLoaded = true;
}

bool wxGxKMLDataset::DeleteChild(IGxObject* pChild)
{
	bool bHasChildren = m_Children.size() > 0 ? true : false;
    m_pCatalog->ObjectDeleted(pChild);
	if(!IGxObjectContainer::DeleteChild(pChild))
		return false;
	if(bHasChildren != m_Children.size() > 0 ? true : false)
		m_pCatalog->ObjectChanged(this);
	return true;
}

wxGISDatasetSPtr wxGxKMLDataset::GetDataset(bool bReadOnly)
{
    LoadChildren();
	if(m_pwxGISDataset == NULL)
        return wxGISDatasetSPtr();

	//m_pwxGISDataset->Reference();
	return m_pwxGISDataset;
}

//--------------------------------------------------------------
//class wxGxKMLSubDataset
//--------------------------------------------------------------

wxGxKMLSubDataset::wxGxKMLSubDataset(wxString sName, wxGISDatasetSPtr pwxGISDataset, wxGISEnumVectorDatasetType nType)
{
	m_type = nType;

	m_sName = sName;

	m_pwxGISDataset = pwxGISDataset;

    m_Encoding = wxFONTENCODING_UTF8;
    m_pPathEncoding = wxConvCurrent;
}

wxGxKMLSubDataset::~wxGxKMLSubDataset(void)
{
    //wsDELETE(m_pwxGISDataset);
}

wxString wxGxKMLSubDataset::GetCategory(void)
{
	return wxString(_("KML Feature class"));
}

wxGISDatasetSPtr wxGxKMLSubDataset::GetDataset(bool bReadOnly)
{
	if(m_pwxGISDataset == NULL)
        return wxGISDatasetSPtr();

  //  if(!m_pwxGISDataset->Open())
  //  {
	 //   const char* err = CPLGetLastErrorMsg();
	 //   wxString sErr = wxString::Format(_("Open failed! OGR error: %s"), wgMB2WX(err));
	 //   wxMessageBox(sErr, _("Error"), wxOK | wxICON_ERROR);
		//return NULL;
  //  }

  //  m_pwxGISDataset->SetSubType(m_type);
	//for storing internal pointer
	//m_pwxGISDataset->Reference();

    //for outer pointer
	//m_pwxGISDataset->Reference();
	return m_pwxGISDataset;
}

void wxGxKMLSubDataset::SetEncoding(wxFontEncoding Encoding)
{
    m_Encoding = Encoding;
}

/******************************************************************************
 * Project:  wxGIS (GIS Catalog)
 * Purpose:  wxGxDataset class.
 * Author:   Bishop (aka Barishnikov Dmitriy), polimax@mail.ru
 ******************************************************************************
*   Copyright (C) 2009  Bishop
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

#include "wxgis/catalog/gxdataset.h"
#include "wxgis/datasource/featuredataset.h"
#include "wxgis/datasource/rasterdataset.h"
#include "wxgis/framework/application.h"
#include "wxgis/datasource/sysop.h"

#include <wx/busyinfo.h>
#include <wx/utils.h>

//--------------------------------------------------------------
//class wxGxTableDataset
//--------------------------------------------------------------

wxGxTableDataset::wxGxTableDataset(wxString Path, wxString Name, wxGISEnumTableDatasetType nType, wxIcon LargeIcon, wxIcon SmallIcon)
{
	m_type = nType;

	m_sName = Name;
	m_sPath = Path;

	m_pwxGISDataset = NULL;
    m_pPathEncoding = wxConvCurrent;

    m_LargeIcon = LargeIcon;
    m_SmallIcon = SmallIcon;
}

wxGxTableDataset::~wxGxTableDataset(void)
{
    wsDELETE(m_pwxGISDataset);
}

wxIcon wxGxTableDataset::GetLargeImage(void)
{
	return m_LargeIcon;
}

wxIcon wxGxTableDataset::GetSmallImage(void)
{
	return m_SmallIcon;
}

bool wxGxTableDataset::Delete(void)
{
    wxGISFeatureDataset* pDSet = dynamic_cast<wxGISFeatureDataset*>(GetDataset());
    if(!pDSet)
        return false;

    //pDSet->Reference();
    bool bRet = pDSet->Delete();
    if(bRet)
	{
        wsDELETE(pDSet);

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
}

	
void wxGxTableDataset::Detach(void)
{
	IGxObject::Detach();
    wsDELETE(m_pwxGISDataset);
}

bool wxGxTableDataset::Rename(wxString NewName)
{
    wxGISFeatureDataset* pDSet = dynamic_cast<wxGISFeatureDataset*>(GetDataset());
    if(!pDSet)
        return false;
	if(pDSet->Rename(NewName))
	{
		NewName = ClearExt(NewName);
		wxFileName PathName(m_sPath);
		PathName.SetName(NewName);

		m_sPath = PathName.GetFullPath();
		m_sName = NewName;
		m_pCatalog->ObjectChanged(this);
		return true;
	}
	else
	{
		const char* err = CPLGetLastErrorMsg();
		wxLogError(_("Delete failed! GDAL error: %s, file '%s'"), wgMB2WX(err), m_sPath.c_str());
		return false;
	}	
}

void wxGxTableDataset::EditProperties(wxWindow *parent)
{
}

wxGISDataset* wxGxTableDataset::GetDataset(void)
{
	if(m_pwxGISDataset == NULL)
	{		
		m_pwxGISDataset = new wxGISFeatureDataset(m_sPath, enumVecUnknown);
		//for storing internal pointer
		m_pwxGISDataset->Reference();
	}
	//for outer pointer
	m_pwxGISDataset->Reference();
	return m_pwxGISDataset;
}


//--------------------------------------------------------------
//class wxGxFeatureDataset
//--------------------------------------------------------------

wxGxFeatureDataset::wxGxFeatureDataset(wxString Path, wxString Name, wxGISEnumVectorDatasetType Type, wxIcon LargeIcon, wxIcon SmallIcon)
{
	m_type = Type;

	m_sName = Name;
	m_sPath = Path;

	m_pwxGISDataset = NULL;

    m_pPathEncoding = wxConvCurrent;

    m_LargeIcon = LargeIcon;
    m_SmallIcon = SmallIcon;
}

wxGxFeatureDataset::~wxGxFeatureDataset(void)
{
    wsDELETE(m_pwxGISDataset);
}

wxString wxGxFeatureDataset::GetCategory(void)
{
	return wxString(_("Feature class"));
}
	
void wxGxFeatureDataset::Detach(void)
{
	IGxObject::Detach();
	wxGISFeatureDataset* pDSet = dynamic_cast<wxGISFeatureDataset*>(m_pwxGISDataset);
	if(pDSet)
		pDSet->Close();
}

wxIcon wxGxFeatureDataset::GetLargeImage(void)
{
    return m_LargeIcon;
}

wxIcon wxGxFeatureDataset::GetSmallImage(void)
{
    return m_SmallIcon;
}

bool wxGxFeatureDataset::Delete(void)
{
    wxGISFeatureDataset* pDSet;
 	if(m_pwxGISDataset == NULL)
    {
        pDSet = new wxGISFeatureDataset(m_sPath, m_type);
        m_pwxGISDataset = static_cast<wxGISDataset*>(pDSet);
    }
    else
    {
        pDSet = dynamic_cast<wxGISFeatureDataset*>(m_pwxGISDataset);
    }
    
    if(!pDSet)
        return false;

    bool bRet = pDSet->Delete();
    if(bRet)
	{
        //wsDELETE(pDSet);
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
}

bool wxGxFeatureDataset::Rename(wxString NewName)
{
    wxGISFeatureDataset* pDSet = dynamic_cast<wxGISFeatureDataset*>(GetDataset());
    if(!pDSet)
        return false;
	if(pDSet->Rename(NewName))
	{
		NewName = ClearExt(NewName);
		wxFileName PathName(m_sPath);
		PathName.SetName(NewName);

		m_sPath = PathName.GetFullPath();
		m_sName = NewName;
		m_pCatalog->ObjectChanged(this);
		return true;
	}
	else
	{
		const char* err = CPLGetLastErrorMsg();
		wxLogError(_("Delete failed! GDAL error: %s, file '%s'"), wgMB2WX(err), m_sPath.c_str());
		return false;
	}	
}

void wxGxFeatureDataset::EditProperties(wxWindow *parent)
{
}

wxGISDataset* wxGxFeatureDataset::GetDataset(void)
{
	if(m_pwxGISDataset == NULL)
	{		
        wxGISFeatureDataset* pwxGISFeatureDataset = new wxGISFeatureDataset(m_sPath, m_type);

        if(!pwxGISFeatureDataset->Open())
        {
		    const char* err = CPLGetLastErrorMsg();
		    wxString sErr = wxString::Format(_("Open failed! GDAL error: %s"), wgMB2WX(err));
		    wxMessageBox(sErr, _("Error"), wxOK | wxICON_ERROR);

            wxDELETE(pwxGISFeatureDataset);
			return NULL;
        }

       // //Spatial Index
       // if(!pwxGISFeatureDataset->HasSpatialIndex())
       // {
       // 	bool bAskSpaInd = true;
       //     wxString name, ext;
       //     wxFileName::SplitPath(m_sPath, NULL, NULL, &name, &ext);
       //     wxString sFileName = name + wxT(".") + ext;
       //     IGISConfig*  pConfig = m_pCatalog->GetConfig();
       //     bool bCreateSpaInd = true;
       //     if(pConfig)
       //     {
       //         wxXmlNode* pNode = pConfig->GetConfigNode(enumGISHKCU, wxString(wxT("catalog/vector")));
       //         if(pNode)
       //             bAskSpaInd = wxAtoi(pNode->GetPropVal(wxT("create_tree"), wxT("1")));
       //         else
       //         {
       //             pNode = pConfig->CreateConfigNode(enumGISHKCU, wxString(wxT("catalog/vector")), true);
       //             pNode->AddProperty(wxT("create_tree"), wxT("1"));
       //         }
       //         if(bAskSpaInd)
       //         {
       //             //show ask dialog
       //             wxGISMessageDlg dlg(NULL, wxID_ANY, wxString::Format(_("Create spatial index for %s"), sFileName.c_str()), wxString(_("This vector datasource does not have spatial index. Spatial index allows rapid display at different resolutions.")), wxString(_("Spatial index building may take few moments.\nWould you like to create spatial index?")), wxDefaultPosition, wxSize( 400,160 ));
       //             if(dlg.ShowModal() == wxID_NO)
       //             {
       //                 bCreateSpaInd = false;
       //             }
       //             if(!dlg.GetShowInFuture())
       //             {
       //                 pNode->DeleteProperty(wxT("create_tree"));
       //                 pNode->AddProperty(wxT("create_tree"), wxT("0"));
       //             }
       //         }
       //     }

	      //  if(bCreateSpaInd)
	      //  {
       //         wxWindowDisabler disableAll;
       //         wxBusyInfo wait(_("Please wait, working..."));

       //         OGRErr err = pwxGISFeatureDataset->CreateSpatialIndex();
			    //if(err != OGRERR_NONE)
			    //{
       //             const char* pszerr = CPLGetLastErrorMsg();
       //             wxLogError(_("Build Spatial Index failed! OGR error: %s"), wgMB2WX(pszerr));
       //             int nRes = wxMessageBox(_("Build Spatial Index! Continue?"), _("Question"), wxICON_QUESTION | wxYES_NO);
       //             if(nRes == wxNO)
       //                 return NULL;
			    //}
	      //  }
       // }

		m_pwxGISDataset = static_cast<wxGISDataset*>(pwxGISFeatureDataset);
        m_pwxGISDataset->SetSubType(m_type);
		//for storing internal pointer
		m_pwxGISDataset->Reference();
	}
	//for outer pointer
	m_pwxGISDataset->Reference();
	return m_pwxGISDataset;
}

//--------------------------------------------------------------
//class wxGxRasterDataset
//--------------------------------------------------------------

wxGxRasterDataset::wxGxRasterDataset(wxString Path, wxString Name, wxGISEnumRasterDatasetType Type, wxIcon LargeIcon, wxIcon SmallIcon)
{

	m_type = Type;

	m_sName = Name;
	m_sPath = Path;

	m_pwxGISDataset = NULL;

    m_LargeIcon = LargeIcon;
    m_SmallIcon = SmallIcon;
}

wxGxRasterDataset::~wxGxRasterDataset(void)
{
	wsDELETE(m_pwxGISDataset);
}

wxIcon wxGxRasterDataset::GetLargeImage(void)
{
	return m_LargeIcon;
}

wxIcon wxGxRasterDataset::GetSmallImage(void)
{
	return m_SmallIcon;
}

bool wxGxRasterDataset::Delete(void)
{
    wxGISRasterDataset* pDSet;
 	if(m_pwxGISDataset == NULL)
    {
        pDSet = new wxGISRasterDataset(m_sPath, m_type);
        m_pwxGISDataset = static_cast<wxGISDataset*>(pDSet);
    }
    else
    {
        pDSet = dynamic_cast<wxGISRasterDataset*>(m_pwxGISDataset);
    }
    
    if(!pDSet)
        return false;

    bool bRet = pDSet->Delete();
    if(bRet)
	{
        //wsDELETE(pDSet);
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
}

bool wxGxRasterDataset::Rename(wxString NewName)
{
    wxGISRasterDataset* pDSet = dynamic_cast<wxGISRasterDataset*>(GetDataset());
    if(!pDSet)
        return false;
	if(pDSet->Rename(NewName))
	{
		NewName = ClearExt(NewName);
		wxFileName PathName(m_sPath);
		PathName.SetName(NewName);

		m_sPath = PathName.GetFullPath();
		m_sName = NewName;
		m_pCatalog->ObjectChanged(this);
		return true;
	}
	else
	{
		const char* err = CPLGetLastErrorMsg();
		wxLogError(_("Delete failed! GDAL error: %s, file '%s'"), wgMB2WX(err), m_sPath.c_str());
		return false;
	}	
}

void wxGxRasterDataset::EditProperties(wxWindow *parent)
{
}

wxString wxGxRasterDataset::GetCategory(void)
{
	switch(m_type)
	{
	case enumRasterUnknown:
	default:
		return wxString(_("Raster"));
	}
}

	
void wxGxRasterDataset::Detach(void)
{
	IGxObject::Detach();
    wsDELETE(m_pwxGISDataset);
}

wxGISDataset* wxGxRasterDataset::GetDataset(void)
{
	if(m_pwxGISDataset == NULL)
	{	
        wxGISRasterDataset* pwxGISRasterDataset = new wxGISRasterDataset(m_sPath, m_type);

        //open (ask for overviews)
        if(!pwxGISRasterDataset->Open())
        {
		    const char* err = CPLGetLastErrorMsg();
		    wxString sErr = wxString::Format(_("Open failed! GDAL error: %s"), wgMB2WX(err));
		    wxMessageBox(sErr, _("Error"), wxOK | wxICON_ERROR);

            wxDELETE(pwxGISRasterDataset);
			return NULL;
        }
        
        m_pwxGISDataset = static_cast<wxGISDataset*>(pwxGISRasterDataset);
		//for storing internal pointer
		m_pwxGISDataset->Reference();
	}
	//for outer pointer
	m_pwxGISDataset->Reference();
	return m_pwxGISDataset;
}
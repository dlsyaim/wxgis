/******************************************************************************
 * Project:  wxGIS (GIS Catalog)
 * Purpose:  wxGxDataset class.
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

#include "wxgis/catalog/gxdataset.h"
#include "wxgis/datasource/featuredataset.h"
#include "wxgis/datasource/rasterdataset.h"
#include "wxgis/datasource/sysop.h"

//--------------------------------------------------------------
//class wxGxTableDataset
//--------------------------------------------------------------

wxGxTableDataset::wxGxTableDataset(wxString Path, wxString Name, wxGISEnumTableDatasetType nType)
{
	m_type = nType;

	m_sName = Name;
	m_sPath = Path;

	//m_pwxGISDataset = NULL;
    m_pPathEncoding = wxConvCurrent;
}

wxGxTableDataset::~wxGxTableDataset(void)
{
    //wsDELETE(m_pwxGISDataset);
}

bool wxGxTableDataset::Delete(void)
{
    wxGISFeatureDatasetSPtr pDSet = boost::dynamic_pointer_cast<wxGISFeatureDataset>(GetDataset(true));
    if(!pDSet)
        return false;

    //pDSet->Reference();
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

	
void wxGxTableDataset::Detach(void)
{
	IGxObject::Detach();
    //if(m_pwxGISDataset)
    //    if(m_pwxGISDataset->Release() == 0)
    //        m_pwxGISDataset = NULL;

}

wxString wxGxTableDataset::GetBaseName(void)
{
    wxFileName FileName(m_sName);
    FileName.SetEmptyExt();
    return FileName.GetName();
}

bool wxGxTableDataset::Rename(wxString NewName)
{
    wxGISFeatureDatasetSPtr pDSet = boost::dynamic_pointer_cast<wxGISFeatureDataset>(GetDataset(true));
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

wxGISDatasetSPtr wxGxTableDataset::GetDataset(bool bReadOnly)
{
	if(m_pwxGISDataset == NULL)
	{		
        m_pwxGISDataset = boost::make_shared<wxGISFeatureDataset>(m_sPath, enumVecUnknown);
		//for storing internal pointer
		//m_pwxGISDataset->Reference();
	}
	//for outer pointer
	//m_pwxGISDataset->Reference();
	return m_pwxGISDataset;
}


//--------------------------------------------------------------
//class wxGxFeatureDataset
//--------------------------------------------------------------

wxGxFeatureDataset::wxGxFeatureDataset(wxString Path, wxString Name, wxGISEnumVectorDatasetType nType)
{
	m_type = nType;

	m_sName = Name;
	m_sPath = Path;

	//m_pwxGISDataset = NULL;

    m_pPathEncoding = wxConvCurrent;
}

wxGxFeatureDataset::~wxGxFeatureDataset(void)
{
    //wsDELETE(m_pwxGISDataset);
}

wxString wxGxFeatureDataset::GetCategory(void)
{
	return wxString(_("Feature class"));
}
	
void wxGxFeatureDataset::Detach(void)
{
	IGxObject::Detach();
    wxGISFeatureDatasetSPtr pDSet = boost::dynamic_pointer_cast<wxGISFeatureDataset>(m_pwxGISDataset);
	if(pDSet)
		pDSet->Close();
}

wxString wxGxFeatureDataset::GetBaseName(void)
{
    wxFileName FileName(m_sName);
    FileName.SetEmptyExt();
    return FileName.GetName();
}

bool wxGxFeatureDataset::Delete(void)
{
    wxGISFeatureDatasetSPtr pDSet;
 	if(m_pwxGISDataset == NULL)
    {
        pDSet = boost::make_shared<wxGISFeatureDataset>(m_sPath, m_type);
        m_pwxGISDataset = boost::static_pointer_cast<wxGISDataset>(pDSet);
    }
    else
    {
        pDSet = boost::dynamic_pointer_cast<wxGISFeatureDataset>(m_pwxGISDataset);
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
    wxGISFeatureDatasetSPtr pDSet = boost::dynamic_pointer_cast<wxGISFeatureDataset>(GetDataset(true));
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

wxGISDatasetSPtr wxGxFeatureDataset::GetDataset(bool bReadOnly)
{
	if(m_pwxGISDataset == NULL)
	{		
        wxGISFeatureDatasetSPtr pwxGISFeatureDataset = boost::make_shared<wxGISFeatureDataset>(m_sPath, m_type);

        if(!pwxGISFeatureDataset->Open())
        {
		    const char* err = CPLGetLastErrorMsg();
		    wxString sErr = wxString::Format(_("Open failed! GDAL error: %s"), wgMB2WX(err));
            wxLogError(sErr);

            //wxDELETE(pwxGISFeatureDataset);
			return wxGISDatasetSPtr();
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

        m_pwxGISDataset = boost::static_pointer_cast<wxGISDataset>(pwxGISFeatureDataset);
        m_pwxGISDataset->SetSubType(m_type);
		//for storing internal pointer
		//m_pwxGISDataset->Reference();
	}
	//for outer pointer
	//m_pwxGISDataset->Reference();
	return m_pwxGISDataset;
}

//--------------------------------------------------------------
//class wxGxRasterDataset
//--------------------------------------------------------------

wxGxRasterDataset::wxGxRasterDataset(wxString Path, wxString Name, wxGISEnumRasterDatasetType nType)
{

	m_type = nType;

	m_sName = Name;
	m_sPath = Path;

	//m_pwxGISDataset = NULL;
}

wxGxRasterDataset::~wxGxRasterDataset(void)
{
	//wsDELETE(m_pwxGISDataset);
}

bool wxGxRasterDataset::Delete(void)
{
    wxGISRasterDatasetSPtr pDSet;
 	if(m_pwxGISDataset == NULL)
    {
        pDSet = boost::make_shared<wxGISRasterDataset>(m_sPath, m_type);
        m_pwxGISDataset = boost::static_pointer_cast<wxGISDataset>(pDSet);
    }
    else
    {
        pDSet = boost::dynamic_pointer_cast<wxGISRasterDataset>(m_pwxGISDataset);
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
wxString wxGxRasterDataset::GetBaseName(void)
{
    wxFileName FileName(m_sName);
    FileName.SetEmptyExt();
    return FileName.GetName();
}

bool wxGxRasterDataset::Rename(wxString NewName)
{
    wxGISRasterDatasetSPtr pDSet = boost::dynamic_pointer_cast<wxGISRasterDataset>(GetDataset(true));
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
    //if(m_pwxGISDataset)
    //    if(m_pwxGISDataset->Release() == 0)
    //        m_pwxGISDataset = NULL;

}

wxGISDatasetSPtr wxGxRasterDataset::GetDataset(bool bReadOnly)
{
	if(m_pwxGISDataset == NULL)
	{	
        wxGISRasterDatasetSPtr pwxGISRasterDataset = boost::make_shared<wxGISRasterDataset>(m_sPath, m_type);

        if(!pwxGISRasterDataset->Open(bReadOnly))
        {
		    const char* err = CPLGetLastErrorMsg();
		    wxString sErr = wxString::Format(_("Open failed! GDAL error: %s"), wgMB2WX(err));
            wxLogError(sErr);

			return wxGISDatasetSPtr();
        }
        
        m_pwxGISDataset = boost::static_pointer_cast<wxGISDataset>(pwxGISRasterDataset);
		//for storing internal pointer
		//m_pwxGISDataset->Reference();
	}
	//for outer pointer
	//m_pwxGISDataset->Reference();
	return m_pwxGISDataset;
}

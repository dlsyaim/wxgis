/******************************************************************************
 * Project:  wxGIS
 * Purpose:  format clases (for format coordinates etc.).
 * Author:   Baryshnikov Dmitriy (aka Bishop), polimax@mail.ru
 ******************************************************************************
*   Copyright (C) 2011 Bishop
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

#include "wxgis/core/core.h"

#include <wx/xml/xml.h>

/** \fn wxString DoubleToString(double Val, bool IsLon) format.h
    \brief Format double value as string coordinate
*/
wxString WXDLLIMPEXP_GIS_CORE DoubleToString(double dVal, bool bIsLon);
/** \fn static double StringToDouble(wxString Val, wxString asterisk) format.h
    \brief Format string coordinate as double value 
*/
double WXDLLIMPEXP_GIS_CORE StringToDouble(const wxString &sVal, const wxString &sAsterisk);

void WXDLLIMPEXP_GIS_CORE SetFloatValue(wxXmlNode* pNode, const wxString &sAttrName, double dfVal);
void WXDLLIMPEXP_GIS_CORE SetFloatValue(wxXmlNode* pNode, const wxString &sAttrName, float dfVal);
double WXDLLIMPEXP_GIS_CORE GetFloatValue(const wxXmlNode* pNode, const wxString &sAttrName, double dfDefVal);
float WXDLLIMPEXP_GIS_CORE GetFloatValue(const wxXmlNode* pNode, const wxString &sAttrName, float dfDefVal);
void WXDLLIMPEXP_GIS_CORE SetDateValue(wxXmlNode* pNode, const wxString &sAttrName, const wxDateTime &dtVal);
wxDateTime WXDLLIMPEXP_GIS_CORE GetDateValue(const wxXmlNode* pNode, const wxString &sAttrName, const wxDateTime &dtDefVal);
void WXDLLIMPEXP_GIS_CORE SetBoolValue(wxXmlNode* pNode, const wxString &sAttrName, bool bVal);
bool WXDLLIMPEXP_GIS_CORE GetBoolValue(const wxXmlNode* pNode, const wxString &sAttrName, bool bDefVal);
void WXDLLIMPEXP_GIS_CORE SetDecimalValue(wxXmlNode* pNode, const wxString &sAttrName, long nVal);
void WXDLLIMPEXP_GIS_CORE SetDecimalValue(wxXmlNode* pNode, const wxString &sAttrName, int nVal);
long WXDLLIMPEXP_GIS_CORE GetDecimalValue(const wxXmlNode* pNode, const wxString &sAttrName, long nDefVal);
int WXDLLIMPEXP_GIS_CORE GetDecimalValue(const wxXmlNode* pNode, const wxString &sAttrName, int nDefVal);

void WXDLLIMPEXP_GIS_CORE FloatStringToCLoc(wxString & str);
void WXDLLIMPEXP_GIS_CORE FloatStringFromCLoc(wxString & str);

/** \fn static wxString NumberScale(double fScaleRatio) format.h
    \brief Format map scale text
*/
WXDLLIMPEXP_GIS_CORE wxString NumberScale(double dScaleRatio);

/** \class wxGISCoordinatesFormat format.h
    \brief The class to format coordinates according different masks
*/
class WXDLLIMPEXP_GIS_CORE wxGISCoordinatesFormat
{
public:
    wxGISCoordinatesFormat(void);
    wxGISCoordinatesFormat(const wxString &sMask, bool bSwap = false);
    virtual ~wxGISCoordinatesFormat(void);
	virtual wxString Format(double dX, double dY);
	virtual bool IsOk(void){return m_bIsOk;};
	virtual void Create(const wxString &sMask, bool bSwap = false);
    virtual wxString GetMask() const {return m_sMask;};
public:
	typedef enum coord_format {  
            D,
            DM,
            DMS,
            M,
            MS,
            S
    } COORD_FORMAT;
protected:
	wxString FormatToken(int nValHigh, int nValLow);
	wxString FormatString(double dCoord, const wxString &sFormat, COORD_FORMAT nCoordFormat, bool bLat);
	wxString ParseString(const wxString &sMask, COORD_FORMAT* pCoordFormat);
protected:
	bool m_bIsOk;
    bool m_bSwaped;
    wxString m_sMask;
    wxString m_sFormatX, m_sFormatY, m_sDivider;
    COORD_FORMAT m_CoordFormatX, m_CoordFormatY;
    bool m_bSign;
};


/******************************************************************************
 * Project:  wxGIS
 * Purpose:  wxGISDisplay class.
 * Author:   Baryshnikov Dmitriy (aka Bishop), polimax@mail.ru
 ******************************************************************************
*   Copyright (C) 2011-2013 Bishop
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
#include "wxgis/display/gisdisplay.h"
#include "wxgis/datasource/vectorop.h"
#include "wxgis/display/displayop.h"

#include <wx/graphics.h>
#include <wx/dcgraph.h>

wxGISDisplay::wxGISDisplay(void)
{
	//default background color
	m_stBackGroudnColour.dRed = 0.9;
	m_stBackGroudnColour.dGreen = 1.0;
	m_stBackGroudnColour.dBlue = 1.0;
	m_stBackGroudnColour.dAlpha = 1.0;
	//m_stBackGroudnColour = {0.9,1.0,1.0,1.0}; //230, 255, 255, 255

	//default frame size
	m_oDeviceFrameRect = wxRect(0,0,800,600);
	m_dFrameCenterX = 400;
	m_dFrameCenterY = 300;
	m_dFrameRatio = 1.3333333333333333333333333333333;

	//create first cached layer
	m_nMax_X = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
	m_nMax_Y = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
	LAYERCACHEDATA layercachedata;
	layercachedata.bIsDerty = true;
	layercachedata.pCairoSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, m_nMax_X, m_nMax_Y);
	layercachedata.pCairoContext = cairo_create (layercachedata.pCairoSurface);
	m_saLayerCaches.push_back(layercachedata);

    //add flash cache
    layercachedata.bIsDerty = false;
	layercachedata.pCairoSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, m_nMax_X, m_nMax_Y);
	layercachedata.pCairoContext = cairo_create (layercachedata.pCairoSurface);
	m_saLayerCaches.push_back(layercachedata);

	m_dCacheCenterX = m_nMax_X / 2;
	m_dCacheCenterY = m_nMax_Y / 2;

	m_pMatrix = new cairo_matrix_t;
	m_pDisplayMatrix = new cairo_matrix_t;
	m_pDisplayMatrixNoRotate = new cairo_matrix_t;

	m_surface_tmp = cairo_image_surface_create (CAIRO_FORMAT_RGB24, m_oDeviceFrameRect.GetWidth(), m_oDeviceFrameRect.GetHeight());
	m_cr_tmp = cairo_create (m_surface_tmp);

	Clear();
}

wxGISDisplay::~wxGISDisplay(void)
{
    for(size_t i = 0; i < m_saLayerCaches.size(); ++i)
	{
		cairo_destroy(m_saLayerCaches[i].pCairoContext);
		cairo_surface_destroy(m_saLayerCaches[i].pCairoSurface);
	}
	wxDELETE(m_pMatrix);
	wxDELETE(m_pDisplayMatrix);
	wxDELETE(m_pDisplayMatrixNoRotate);
    cairo_destroy (m_cr_tmp);
    cairo_surface_destroy (m_surface_tmp);
}

void wxGISDisplay::Clear()
{
    if(m_saLayerCaches.size() > 2)
    {
	    for(size_t i = 1; i < m_nLastCacheID; ++i)
	    {
		    cairo_destroy(m_saLayerCaches[i].pCairoContext);
		    cairo_surface_destroy(m_saLayerCaches[i].pCairoSurface);
	    }
        m_saLayerCaches.erase(m_saLayerCaches.begin() + 1, m_saLayerCaches.begin() + m_nLastCacheID);
    }

	//default map bounds
	m_RealBounds.MinX = ENVMIN_X;
	m_RealBounds.MaxX = ENVMAX_X;
	m_RealBounds.MinY = ENVMIN_Y;
	m_RealBounds.MaxY = ENVMAX_Y;
	m_CurrentBounds = m_RealBounds;

	SetEnvelopeRatio(m_CurrentBounds, m_dFrameRatio);
	m_CurrentBoundsRotated = m_CurrentBounds;
	m_dRotatedBoundsCenterX = m_CurrentBoundsRotated.MinX + (m_CurrentBoundsRotated.MaxX - m_CurrentBoundsRotated.MinX) / 2;
	m_dRotatedBoundsCenterY = m_CurrentBoundsRotated.MinY + (m_CurrentBoundsRotated.MaxY - m_CurrentBoundsRotated.MinY) / 2;

	m_CurrentBoundsX8 = m_CurrentBounds;
	IncreaseEnvelope(m_CurrentBoundsX8, 8);

	m_nLastCacheID = 0;
	m_nCurrentLayer = 0;
	m_dAngleRad = 0;
	cairo_matrix_init(m_pMatrix, 1, 0, 0, 1, 0, 0);
	cairo_matrix_init(m_pDisplayMatrix, 1, 0, 0, 1, 0, 0);
	cairo_matrix_init(m_pDisplayMatrixNoRotate, 1, 0, 0, 1, 0, 0);

	m_stFillColour.dRed = m_stFillColour.dGreen = m_stFillColour.dBlue = 1;
	m_stFillColour.dAlpha = 1;
	m_stLineColour.dRed = m_stLineColour.dGreen = m_stLineColour.dBlue = 0;
	m_stLineColour.dAlpha = 1;
	m_stPointColour = m_stLineColour;

	m_dLineWidth = m_dPointRadius = 0.5;

	m_bZeroCacheSet = false;

    InitTransformMatrix();
	OnEraseBackground();
}

cairo_t* wxGISDisplay::CreateContext(wxDC* dc)
{
    cairo_t *cr(NULL);
#ifdef __WXMSW__
//#if CAIRO_HAS_WIN32_SURFACE
     HDC hdc = (HDC)dc->GetHDC();
     cr = cairo_create(cairo_win32_surface_create( hdc ));
     return cr;
#endif

#ifdef __WXGTK__
     wxGraphicsRenderer * const renderer = wxGraphicsRenderer::GetCairoRenderer();
     wxWindowDC* pwdc =  wxDynamicCast(dc, wxWindowDC);
     wxGraphicsContext * gc = renderer->CreateContext(*pwdc);
     if(gc)
          cr =  (cairo_t*)gc->GetNativeContext();
#endif
     return cr;
}

void wxGISDisplay::OnEraseBackground(void)
{
	wxCriticalSectionLocker locker(m_CritSect);
	cairo_set_source_rgb(m_saLayerCaches[0].pCairoContext, m_stBackGroudnColour.dRed, m_stBackGroudnColour.dGreen, m_stBackGroudnColour.dBlue);
	cairo_paint(m_saLayerCaches[0].pCairoContext);
}

void wxGISDisplay::Output(wxDC* pDC, wxGISPointsArray ClipGeometry)
{
	wxCriticalSectionLocker locker(m_CritSect);
	//cairo_surface_t *surface;
    cairo_t *cr;

	cr = CreateContext(pDC);
	//surface = cairo_get_target(cr);

	//cairo_save (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
	//cairo_surface_flush (m_saLayerCaches[m_nCurrentLayer].pCairoSurface);

	cairo_set_source_surface (cr, m_saLayerCaches[m_nCurrentLayer].pCairoSurface, -m_dOrigin_X, -m_dOrigin_Y);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	if(ClipGeometry.GetCount() > 0)//x1 && y1 && x2 && y2)
	{
		cairo_move_to(cr, ClipGeometry[0]->x, ClipGeometry[0]->y);
		for(size_t i = 1; i < ClipGeometry.GetCount(); ++i)
			cairo_line_to(cr, ClipGeometry[i]->x, ClipGeometry[i]->y);
		//close path
		cairo_line_to(cr, ClipGeometry[0]->x, ClipGeometry[0]->y);
		cairo_clip(cr);
		cairo_new_path (cr);
	}
	cairo_paint (cr);

	//cairo_restore (m_saLayerCaches[m_nCurrentLayer].pCairoContext);

#ifdef __WXMSW__
    //cairo_surface_destroy (surface);
    cairo_destroy (cr);
#endif
}

void wxGISDisplay::ZoomingDraw(const wxRect& rc, wxDC* pDC)
{
	wxCriticalSectionLocker locker(m_CritSect);
	cairo_surface_t *surface;
    cairo_t *cr;

	cr = CreateContext(pDC);
	surface = cairo_get_target(cr);

	//compute scale
	double dScaleX = double(rc.GetWidth()) / m_oDeviceFrameRect.GetWidth();
	double dScaleY = double(rc.GetHeight()) / m_oDeviceFrameRect.GetHeight();
	cairo_scale(cr, dScaleX, dScaleY);
	cairo_set_source_surface (cr, m_saLayerCaches[m_nLastCacheID].pCairoSurface, -m_dOrigin_X, -m_dOrigin_Y);
	cairo_paint (cr);

#ifdef __WXMSW__
    cairo_surface_destroy (surface);
    cairo_destroy (cr);
#endif
}

void wxGISDisplay::WheelingDraw(double dZoom, wxDC* pDC)
{
	wxCriticalSectionLocker locker(m_CritSect);
	cairo_surface_t *surface;
    cairo_t *cr;

	cr = CreateContext(pDC);
	surface = cairo_get_target(cr);

	if(IsDoubleEquil(dZoom, 1)) // no zoom
	{
		cairo_set_source_surface (cr, m_saLayerCaches[m_nCurrentLayer].pCairoSurface, -m_dOrigin_X, -m_dOrigin_Y);
		cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	}
	else if(dZoom > 1) // zoom in
	{
		double dDCXDelta = m_dFrameCenterX / dZoom;
		double dDCYDelta = m_dFrameCenterY / dZoom;
		double dOrigin_X = m_dCacheCenterX - dDCXDelta;
		double dOrigin_Y = m_dCacheCenterY - dDCYDelta;

		cairo_scale(cr, dZoom, dZoom);

		cairo_set_source_surface (cr, m_saLayerCaches[m_nLastCacheID].pCairoSurface, -dOrigin_X, -dOrigin_Y);
	}
	else //zoom out
	{
		double dDCXDelta = m_dFrameCenterX * dZoom;
		double dDCYDelta = m_dFrameCenterY * dZoom;
		double dOrigin_X = m_dFrameCenterX - dDCXDelta;
		double dOrigin_Y = m_dFrameCenterY - dDCYDelta;

        cairo_set_source_rgb(m_cr_tmp, m_stBackGroudnColour.dRed, m_stBackGroudnColour.dGreen, m_stBackGroudnColour.dBlue);
	    cairo_paint(m_cr_tmp);

        cairo_translate (m_cr_tmp, dOrigin_X, dOrigin_Y);
		cairo_scale(m_cr_tmp, dZoom, dZoom);

		cairo_set_source_surface (m_cr_tmp, m_saLayerCaches[m_nLastCacheID].pCairoSurface, -m_dOrigin_X, -m_dOrigin_Y);


		//cairo_set_source_rgb(cr, m_stBackGroudnColour.dRed, m_stBackGroudnColour.dGreen, m_stBackGroudnColour.dBlue);
		//cairo_paint(cr); //comment to prevent flickering
 
		//cairo_translate (cr, dOrigin_X, dOrigin_Y);
		//cairo_scale(cr, dZoom, dZoom);

		//cairo_set_source_surface (cr, m_saLayerCaches[m_nLastCacheID].pCairoSurface, -m_dOrigin_X, -m_dOrigin_Y);

        cairo_paint (m_cr_tmp);

    	cairo_set_source_surface (cr, m_surface_tmp, 0, 0);
	}
	cairo_paint (cr);

#ifdef __WXMSW__
	cairo_surface_destroy (surface);
    cairo_destroy (cr);
#endif
    cairo_matrix_t mat = {1, 0, 0, 1, 0, 0};
	cairo_set_matrix (m_cr_tmp, &mat);
}

void wxGISDisplay::PanningDraw(wxCoord x, wxCoord y, wxDC* pDC)
{
	wxCriticalSectionLocker locker(m_CritSect);

	cairo_set_source_rgb(m_cr_tmp, m_stBackGroudnColour.dRed, m_stBackGroudnColour.dGreen, m_stBackGroudnColour.dBlue);
	cairo_paint(m_cr_tmp);

	double dNewX = m_dOrigin_X + double(x);
	double dNewY = m_dOrigin_Y + double(y);
	cairo_set_source_surface (m_cr_tmp, m_saLayerCaches[m_nLastCacheID].pCairoSurface, -dNewX, -dNewY);

	cairo_paint (m_cr_tmp);

	cairo_surface_t *surface;
    cairo_t *cr;

	cr = CreateContext(pDC);
	surface = cairo_get_target(cr);

	cairo_set_source_surface (cr, m_surface_tmp, 0, 0);
	cairo_paint (cr);

#ifdef __WXMSW__
    cairo_surface_destroy (surface);
    cairo_destroy (cr);
#endif
}

void wxGISDisplay::RotatingDraw(double dAngle, wxDC* pDC)
{
	wxCriticalSectionLocker locker(m_CritSect);

	cairo_set_source_rgb(m_cr_tmp, m_stBackGroudnColour.dRed, m_stBackGroudnColour.dGreen, m_stBackGroudnColour.dBlue);
	cairo_paint(m_cr_tmp);

	int w = cairo_image_surface_get_width (cairo_get_target(m_cr_tmp));
	int h = cairo_image_surface_get_height (cairo_get_target(m_cr_tmp));
	//cairo_translate (m_cr_tmp, 0.5 * m_oDeviceFrameRect.GetWidth(), 0.5 * m_oDeviceFrameRect.GetHeight());

	//double dWorldCenterX = m_CurrentBounds.MinX + double(m_CurrentBounds.MaxX - m_CurrentBounds.MinX) / 2;
	//double dWorldCenterY = m_CurrentBounds.MinY + double(m_CurrentBounds.MaxY - m_CurrentBounds.MinY) / 2;
	//World2DC(&dWorldCenterX, &dWorldCenterY);

	//cairo_translate (m_cr_tmp, dWorldCenterX, dWorldCenterY);
	cairo_translate (m_cr_tmp, m_dFrameCenterX, m_dFrameCenterY);
	cairo_rotate (m_cr_tmp, dAngle);
	//cairo_translate (m_cr_tmp, -0.5 * m_oDeviceFrameRect.GetWidth(), -0.5 * m_oDeviceFrameRect.GetHeight());
	//cairo_translate (m_cr_tmp, -dWorldCenterX, -dWorldCenterY);
	cairo_translate (m_cr_tmp, -m_dFrameCenterX, -m_dFrameCenterY);
	cairo_set_source_surface (m_cr_tmp, m_saLayerCaches[m_nLastCacheID].pCairoSurface, -m_dOrigin_X, -m_dOrigin_Y);

	cairo_paint (m_cr_tmp);

	cairo_surface_t *surface;
    cairo_t *cr;

	cr = CreateContext(pDC);
	surface = cairo_get_target(cr);

	cairo_set_source_surface (cr, m_surface_tmp, 0, 0);
	cairo_paint (cr);

#ifdef __WXMSW__
    cairo_surface_destroy (surface);
    cairo_destroy (cr);
#endif
	cairo_matrix_t mat = {1, 0, 0, 1, 0, 0};
	cairo_set_matrix (m_cr_tmp, &mat);
}

size_t wxGISDisplay::AddCache(void)
{
	wxCriticalSectionLocker locker(m_CritSect);
	if(m_bZeroCacheSet)
	{
		LAYERCACHEDATA layercachedata;
		layercachedata.bIsDerty = true;
		layercachedata.pCairoSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, m_nMax_X, m_nMax_Y);
		layercachedata.pCairoContext = cairo_create (layercachedata.pCairoSurface);
        m_nLastCacheID++;
        m_saLayerCaches.insert(m_saLayerCaches.begin() + m_nLastCacheID, layercachedata);
	}
	else
		m_bZeroCacheSet = true;
	return m_nLastCacheID;
}

size_t wxGISDisplay::GetLastCacheID(void) const
{
	return m_nLastCacheID;
}

size_t wxGISDisplay::GetFlashCacheID(void) const
{
	return m_saLayerCaches.size() - 1;
}

bool wxGISDisplay::IsCacheDerty(size_t nCacheID) const
{
	return m_saLayerCaches[nCacheID].bIsDerty;
}

void wxGISDisplay::SetCacheDerty(size_t nCacheID, bool bIsDerty)
{
	wxCriticalSectionLocker locker(m_CritSect);
	m_saLayerCaches[nCacheID].bIsDerty = bIsDerty;
}

void wxGISDisplay::SetDrawCache(size_t nCacheID, bool bNoDerty)
{
	wxCriticalSectionLocker locker(m_CritSect);
    if(bNoDerty || nCacheID == 0)
        m_nCurrentLayer = nCacheID;
    else
    {
		//merge previous cache
		//TODO: clip by frame size cairo_clip()?
		cairo_save(m_saLayerCaches[nCacheID].pCairoContext);

		cairo_matrix_t mat = {1, 0, 0, 1, 0, 0};
		cairo_set_matrix (m_saLayerCaches[nCacheID].pCairoContext, &mat);

		cairo_set_source_surface (m_saLayerCaches[nCacheID].pCairoContext, m_saLayerCaches[m_nCurrentLayer].pCairoSurface, 0, 0);
		cairo_set_operator (m_saLayerCaches[nCacheID].pCairoContext, CAIRO_OPERATOR_SOURCE);
		cairo_paint (m_saLayerCaches[nCacheID].pCairoContext);

		cairo_restore(m_saLayerCaches[nCacheID].pCairoContext);

		m_nCurrentLayer = nCacheID;
    }
}

void wxGISDisplay::SetDerty(bool bIsDerty)
{
	wxCriticalSectionLocker locker(m_CritSect);
	for(size_t i = 0; i < m_nLastCacheID + 1; ++i)
		m_saLayerCaches[i].bIsDerty = bIsDerty;
}

void wxGISDisplay::SetDertyUpperCache(size_t nFromCacheNo, bool bIsDerty)
{
	wxCriticalSectionLocker locker(m_CritSect);
    if(nFromCacheNo >= m_nLastCacheID + 1)
        return;
	for(size_t i = nFromCacheNo; i < m_nLastCacheID + 1; ++i)
        m_saLayerCaches[i].bIsDerty = bIsDerty;
}

bool wxGISDisplay::IsDerty(void) const
{
	for(size_t i = 0; i < m_nLastCacheID + 1; ++i)
		if(m_saLayerCaches[i].bIsDerty)
			return true;
	return false;
}

void wxGISDisplay::SetDeviceFrame(wxRect &rc)
{
	m_oDeviceFrameRect = rc;
	if(m_oDeviceFrameRect.GetWidth() % 2 != 0)
		m_oDeviceFrameRect.SetWidth(m_oDeviceFrameRect.GetWidth() + 1);
	if(m_oDeviceFrameRect.GetHeight() % 2 != 0)
		m_oDeviceFrameRect.SetHeight(m_oDeviceFrameRect.GetHeight() + 1);

	m_dFrameRatio = double(m_oDeviceFrameRect.GetWidth()) / m_oDeviceFrameRect.GetHeight();

	m_CurrentBounds = m_RealBounds;

	SetEnvelopeRatio(m_CurrentBounds, m_dFrameRatio);
	m_dRotatedBoundsCenterX = m_CurrentBounds.MinX + (m_CurrentBounds.MaxX - m_CurrentBounds.MinX) / 2;
	m_dRotatedBoundsCenterY = m_CurrentBounds.MinY + (m_CurrentBounds.MaxY - m_CurrentBounds.MinY) / 2;
	m_CurrentBoundsRotated = m_CurrentBounds;
	if(!IsDoubleEquil(m_dAngleRad, 0.0))
	{
		RotateEnvelope(m_CurrentBoundsRotated, m_dAngleRad, m_dRotatedBoundsCenterX, m_dRotatedBoundsCenterY);
		SetEnvelopeRatio(m_CurrentBoundsRotated, m_dFrameRatio);//test
	}
	m_CurrentBoundsX8 = m_CurrentBoundsRotated;
	IncreaseEnvelope(m_CurrentBoundsX8, 8);

    cairo_destroy (m_cr_tmp);
    cairo_surface_destroy (m_surface_tmp);

	m_surface_tmp = cairo_image_surface_create (CAIRO_FORMAT_RGB24, m_oDeviceFrameRect.GetWidth(), m_oDeviceFrameRect.GetHeight());
	m_cr_tmp = cairo_create (m_surface_tmp);


	//compute current transform matrix
	InitTransformMatrix();
}

wxRect wxGISDisplay::GetDeviceFrame(void) const
{
	return m_oDeviceFrameRect;
}

void wxGISDisplay::SetBounds(const OGREnvelope& Bounds)
{
	//update bounds to frame ratio
	m_RealBounds = Bounds;
	m_CurrentBounds = m_RealBounds;

	SetEnvelopeRatio(m_CurrentBounds, m_dFrameRatio);
	m_dRotatedBoundsCenterX = m_CurrentBounds.MinX + (m_CurrentBounds.MaxX - m_CurrentBounds.MinX) / 2;
	m_dRotatedBoundsCenterY = m_CurrentBounds.MinY + (m_CurrentBounds.MaxY - m_CurrentBounds.MinY) / 2;
	m_CurrentBoundsRotated = m_CurrentBounds;
	if(!IsDoubleEquil(m_dAngleRad, 0.0))
	{
		RotateEnvelope(m_CurrentBoundsRotated, m_dAngleRad, m_dRotatedBoundsCenterX, m_dRotatedBoundsCenterY);
		SetEnvelopeRatio(m_CurrentBoundsRotated, m_dFrameRatio);//test
	}
	m_CurrentBoundsX8 = m_CurrentBoundsRotated;
	IncreaseEnvelope(m_CurrentBoundsX8, 8);

	SetDerty(true);
	//compute current transform matrix
	InitTransformMatrix();
}

OGREnvelope wxGISDisplay::GetBounds(bool bRotated) const
{
	if(bRotated)
		return m_CurrentBoundsRotated;
	else
		return m_CurrentBounds;
}

void wxGISDisplay::InitTransformMatrix(void)
{
	m_dFrameCenterX = m_oDeviceFrameRect.GetWidth() / 2;
	m_dFrameCenterY = m_oDeviceFrameRect.GetHeight() / 2;
	double dWorldCenterX = (m_CurrentBounds.MaxX - m_CurrentBounds.MinX) / 2;
	double dWorldCenterY = (m_CurrentBounds.MaxY - m_CurrentBounds.MinY) / 2;

	//origin (UL corner)
	m_dOrigin_X = m_dCacheCenterX - m_dFrameCenterX;
	m_dOrigin_Y = m_dCacheCenterY - m_dFrameCenterY;
	//get scale
	double dScaleX = fabs(m_dFrameCenterX / dWorldCenterX);
	double dScaleY = fabs(m_dFrameCenterY / dWorldCenterY);
	m_dScale = std::min(dScaleX, dScaleY);

	double dWorldDeltaX = dWorldCenterX + m_CurrentBounds.MinX;
	double dWorldDeltaY = dWorldCenterY + m_CurrentBounds.MinY;

	double dWorldDeltaXSt = m_dScale * dWorldDeltaX;// + m_dAngleRad * dWorldDeltaY;
	double dWorldDeltaYSt = m_dScale * dWorldDeltaY;//m_dAngleRad * dWorldDeltaX +

	//double dCenterX = m_dCacheCenterX - dWorldDeltaXSt;//(dWorldCenterX + m_CurrentBounds.MinX) * dScale;//
	//double dCenterY = m_dCacheCenterY + dWorldDeltaYSt;//(dWorldCenterY + m_CurrentBounds.MinY) * dScale;//
	m_dFrameXShift = m_dFrameCenterX - dWorldDeltaXSt;//(dWorldCenterX + m_CurrentBounds.MinX) * dScale;//
	m_dFrameYShift = m_dFrameCenterY + dWorldDeltaYSt;//(dWorldCenterY + m_CurrentBounds.MinY) * dScale;//

//	cairo_matrix_init (m_pMatrix, 1, 0, 0, 1, m_dCacheCenterX, m_dCacheCenterY);
	//cairo_matrix_init (m_pMatrix, dScale, 0.0, 0.0, -dScale, dCenterX, dCenterY);
	cairo_matrix_init_translate (m_pMatrix, m_dCacheCenterX, m_dCacheCenterY);
	//rotate
	//cairo_matrix_rotate(m_pMatrix, 45.0 * M_PI / 180.0);
	if(!IsDoubleEquil(m_dAngleRad, 0.0))
	//{
		//cairo_matrix_translate(m_pMatrix, dWorldDeltaXSt, dWorldDeltaYSt);
		cairo_matrix_rotate(m_pMatrix, m_dAngleRad);
		//cairo_matrix_translate(m_pMatrix, -dWorldDeltaXSt, dWorldDeltaYSt);
	//}
	//else
	cairo_matrix_translate(m_pMatrix, -dWorldDeltaXSt, dWorldDeltaYSt);

	cairo_matrix_scale(m_pMatrix, m_dScale, -m_dScale);

	//init matrix for Wld2DC & DC2Wld
	cairo_matrix_init_translate (m_pDisplayMatrix, m_dFrameCenterX, m_dFrameCenterY);
	if(!IsDoubleEquil(m_dAngleRad, 0.0))
		cairo_matrix_rotate(m_pDisplayMatrix, m_dAngleRad);
	cairo_matrix_translate(m_pDisplayMatrix, -dWorldDeltaXSt, dWorldDeltaYSt);
	cairo_matrix_scale(m_pDisplayMatrix, m_dScale, -m_dScale);

	//init matrix for TransformRect
	cairo_matrix_init_translate (m_pDisplayMatrixNoRotate, m_dFrameCenterX, m_dFrameCenterY);
	cairo_matrix_translate(m_pDisplayMatrixNoRotate, -dWorldDeltaXSt, dWorldDeltaYSt);
	cairo_matrix_scale(m_pDisplayMatrixNoRotate, m_dScale, -m_dScale);

	//set matrix to all caches
	for(size_t i = 0; i < m_saLayerCaches.size(); ++i)
		cairo_set_matrix (m_saLayerCaches[i].pCairoContext, m_pMatrix);
}

void wxGISDisplay::DC2World(double* pdX, double* pdY)
{
    //set center of real window not cache
	cairo_matrix_t InvertMatrix = {m_pDisplayMatrix->xx, m_pDisplayMatrix->yx, m_pDisplayMatrix->xy, m_pDisplayMatrix->yy, m_pDisplayMatrix->x0, m_pDisplayMatrix->y0};
	cairo_matrix_invert(&InvertMatrix);
	cairo_matrix_transform_point(&InvertMatrix, pdX, pdY);
}

void wxGISDisplay::World2DC(double* pdX, double* pdY)
{
    //set center of real window not cache
	cairo_matrix_t Matrix = {m_pDisplayMatrix->xx, m_pDisplayMatrix->yx, m_pDisplayMatrix->xy, m_pDisplayMatrix->yy, m_pDisplayMatrix->x0, m_pDisplayMatrix->y0};
	cairo_matrix_transform_point(&Matrix, pdX, pdY);
}

void wxGISDisplay::DC2WorldDist(double* pdX, double* pdY, bool bRotated)
{
	cairo_matrix_t InvertMatrix;
	if(bRotated)//set center of real window not cache
		InvertMatrix = *m_pDisplayMatrix;
	else
		InvertMatrix = *m_pDisplayMatrixNoRotate;
	cairo_matrix_invert(&InvertMatrix);
	cairo_matrix_transform_distance(&InvertMatrix, pdX, pdY);
}

void wxGISDisplay::World2DCDist(double* pdX, double* pdY, bool bRotated)
{
	cairo_matrix_t Matrix;
	if(bRotated)
		Matrix = *m_pDisplayMatrix;
	else
		Matrix = *m_pDisplayMatrixNoRotate;
	cairo_matrix_transform_distance(&Matrix, pdX, pdY);
}

void wxGISDisplay::TestDraw(void)
{
	srand ( time(NULL) );
	SetDrawCache(0);

	m_stLineColour.dRed = 0.0;
	m_stLineColour.dGreen = 0.0;
	m_stLineColour.dBlue = 0.0;
	m_stLineColour.dAlpha = 1.0;
	m_stPointColour = m_stLineColour;

	int random_number1 = (rand() % 50);
	int random_number2 = (rand() % 50);
	int random_number3 = (rand() % 50);
	m_stFillColour.dRed = double(205 + random_number1) / 255;
	m_stFillColour.dGreen = double(205 + random_number2) / 255;
	m_stFillColour.dBlue = double(205 + random_number3) / 255;
	m_stFillColour.dAlpha = 1.0;

	SetColor(m_stLineColour);
	cairo_move_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, -178.0, 79.0);
	cairo_line_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, 178.0, -79.0);
	cairo_move_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, 178.0, 79.0);
	cairo_line_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, -178.0, -79.0);
	cairo_set_line_width (m_saLayerCaches[m_nCurrentLayer].pCairoContext, 0.2);
	cairo_stroke (m_saLayerCaches[m_nCurrentLayer].pCairoContext);

	cairo_move_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, -6, 6);
	cairo_line_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, 6, 6);
//	cairo_move_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, 6, 6);
	cairo_line_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, 6, -6);
//	cairo_move_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, 6, -6);
	cairo_line_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, -6, -6);
//	cairo_move_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, -6, -6);
	cairo_line_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, -6, 6);

	//cairo_move_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, 64.0, 25.6);
	//cairo_rel_line_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, 51.2, 51.2);
	//cairo_rel_line_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, -51.2, 51.2);
	//cairo_rel_line_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, -51.2, -51.2);
	//cairo_close_path (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
	//cairo_rectangle (m_saLayerCaches[m_nCurrentLayer].pCairoContext, -6, 6, 12, -12);
	SetColor(m_stFillColour);
	cairo_fill_preserve (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
	SetColor(m_stLineColour);
	cairo_stroke (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
}

size_t wxGISDisplay::GetDrawCache(void) const
{
	return m_nCurrentLayer;
}

void wxGISDisplay::SetColor(double dRed, double dGreen, double dBlue, double dAlpha)
{
	cairo_set_source_rgba(m_saLayerCaches[m_nCurrentLayer].pCairoContext, dRed, dGreen, dBlue, dAlpha);
}

void wxGISDisplay::SetColor(WXGISRGBA Color)
{
	SetColor(Color.dRed, Color.dGreen, Color.dBlue, Color.dAlpha);
}

bool wxGISDisplay::DrawPoint(double dX, double dY)
{
	cairo_arc (m_saLayerCaches[m_nCurrentLayer].pCairoContext, dX, dY, m_dPointRadius, 0, 2*M_PI);
	return true;
}

bool wxGISDisplay::DrawPointFast(double dX, double dY)
{
	cairo_move_to (m_saLayerCaches[m_nCurrentLayer].pCairoContext, dX, dY);
	cairo_close_path (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
	return true;
}

bool wxGISDisplay::DrawPointFast(const OGRPoint* pPoint)
{
	if(NULL == pPoint)
		return false;
	return DrawPointFast(pPoint->getX(), pPoint->getY());
}


void wxGISDisplay::SetLineCap(cairo_line_cap_t line_cap)
{
	cairo_set_line_cap(m_saLayerCaches[m_nCurrentLayer].pCairoContext, line_cap);
}

void wxGISDisplay::SetLineWidth(double dWidth)
{
	m_dLineWidth = GetScaledWidth(dWidth);
}

void wxGISDisplay::SetPointRadius(double dRadius)
{
	m_dPointRadius = GetScaledWidth(dRadius);
}

void wxGISDisplay::SetFillRule(cairo_fill_rule_t fill_rule)
{
	cairo_set_fill_rule(m_saLayerCaches[m_nCurrentLayer].pCairoContext, fill_rule);
}

bool wxGISDisplay::DrawPoint(const OGRPoint* pPoint)
{
	if(NULL == pPoint)
		return false;
	return DrawPoint(pPoint->getX(), pPoint->getY());
}

bool wxGISDisplay::DrawLine(const OGRLineString* pLine, bool bIsRing)
{
	if(NULL == pLine)
		return false;
	int nPointCount = pLine->getNumPoints();
	if(nPointCount < 2)
		return false;
    OGRRawPoint* pOGRRawPoints = new OGRRawPoint[nPointCount * 3];
	pLine->getPoints(pOGRRawPoints);

	ClipGeometryByEnvelope(pOGRRawPoints, &nPointCount, m_CurrentBoundsX8, !bIsRing);

	cairo_move_to(m_saLayerCaches[m_nCurrentLayer].pCairoContext, pOGRRawPoints[0].x, pOGRRawPoints[0].y);

	for(int i = 1; i < nPointCount; ++i)
	{
        cairo_line_to(m_saLayerCaches[m_nCurrentLayer].pCairoContext, pOGRRawPoints[i].x, pOGRRawPoints[i].y);
	}
	wxDELETEA(pOGRRawPoints);
	return true;
}

bool wxGISDisplay::DrawRing(const OGRLinearRing* pRing)
{
	if(NULL == pRing)
		return false;
	DrawLine((OGRLineString*)pRing, true);
	return true;
}

bool wxGISDisplay::DrawPolygon(const OGRPolygon* pPolygon)
{
	if(NULL == pPolygon)
		return false;
	//check ext ring in extent
	const OGRLinearRing *pRing = pPolygon->getExteriorRing();
	OGREnvelope Env;
	pRing->getEnvelope(&Env);

	if(!IsDoubleEquil(m_dAngleRad, 0.0))
	{
		RotateEnvelope(Env, -m_dAngleRad, m_dRotatedBoundsCenterX, m_dRotatedBoundsCenterY);
	}

	bool bDraw = Env.Contains(m_CurrentBoundsRotated) || m_CurrentBoundsRotated.Contains(Env) || Env.Intersects(m_CurrentBoundsRotated);
	if( !bDraw )
		return false;

	DrawRing( pRing );
	int nNumInteriorRings = pPolygon->getNumInteriorRings();
	for(int nPart = 0; nPart < nNumInteriorRings; ++nPart)
	{
		const OGRLinearRing* pRing = pPolygon->getInteriorRing(nPart);
		if(!CheckDrawAsPoint((OGRGeometry*)pRing, true))
			DrawRing(pRing);
	}

    if(!IsDoubleEquil(m_stFillColour.dAlpha, 0.0))
    {
	    SetFillRule( CAIRO_FILL_RULE_EVEN_ODD );
	    SetColor(m_stFillColour);
	    cairo_fill_preserve (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
    }
	return true;
}

bool wxGISDisplay::CheckDrawAsPoint(const OGRGeometry* pGeometry, bool bCheckEnvelope)
{
	OGREnvelope Envelope, TestEnv;
	pGeometry->getEnvelope(&Envelope);
	TestEnv = Envelope;
	if(!IsDoubleEquil(m_dAngleRad, 0.0))
	{
		RotateEnvelope(TestEnv, -m_dAngleRad, m_dRotatedBoundsCenterX, m_dRotatedBoundsCenterY);
	}

	if(bCheckEnvelope && !m_CurrentBoundsRotated.Intersects(TestEnv))
		return true;

	double EnvWidth = Envelope.MaxX - Envelope.MinX;
	double EnvHeight = Envelope.MaxY - Envelope.MinY;

	World2DCDist(&EnvWidth, &EnvHeight);
	EnvWidth = fabs(EnvWidth);
	EnvHeight = fabs(EnvHeight);
	if(	EnvWidth <= MINPOLYDRAWAREA && EnvHeight <= MINPOLYDRAWAREA )
	{
		if(	EnvWidth >= MINPOLYAREA && EnvHeight >= MINPOLYAREA )
		{
			cairo_move_to(m_saLayerCaches[m_nCurrentLayer].pCairoContext, Envelope.MinX, Envelope.MinY);
			cairo_line_to(m_saLayerCaches[m_nCurrentLayer].pCairoContext, Envelope.MaxX, Envelope.MaxY);
		}
        else
        {
//			cairo_move_to(m_saLayerCaches[m_nCurrentLayer].pCairoContext, Envelope.MinX, Envelope.MinY);
//			cairo_line_to(m_saLayerCaches[m_nCurrentLayer].pCairoContext, Envelope.MaxX, Envelope.MaxY);
        }
		return true;
	}
	return false;
}

void wxGISDisplay::DrawRaster(cairo_surface_t *surface, OGREnvelope& Envelope, bool bDrawEnvelope)
{
	wxCriticalSectionLocker locker(m_CritSect);

    cairo_pattern_t *pattern = cairo_pattern_create_for_surface (surface);
	cairo_matrix_t   matrix;
	cairo_matrix_init_scale (&matrix, m_dScale, -m_dScale);
	cairo_matrix_translate(&matrix, -Envelope.MinX, -Envelope.MaxY);
	cairo_pattern_set_matrix (pattern, &matrix);
	cairo_set_source (m_saLayerCaches[m_nCurrentLayer].pCairoContext, pattern);
	cairo_paint (m_saLayerCaches[m_nCurrentLayer].pCairoContext);

	if(bDrawEnvelope)
	{
		cairo_set_line_width(m_saLayerCaches[m_nCurrentLayer].pCairoContext, m_dLineWidth);

		SetColor(m_stFillColour);
		cairo_move_to(m_saLayerCaches[m_nCurrentLayer].pCairoContext, Envelope.MinX, Envelope.MinY);
		cairo_line_to(m_saLayerCaches[m_nCurrentLayer].pCairoContext, Envelope.MaxX, Envelope.MinY);
		cairo_line_to(m_saLayerCaches[m_nCurrentLayer].pCairoContext, Envelope.MaxX, Envelope.MaxY);
		cairo_line_to(m_saLayerCaches[m_nCurrentLayer].pCairoContext, Envelope.MinX, Envelope.MaxY);
		cairo_line_to(m_saLayerCaches[m_nCurrentLayer].pCairoContext, Envelope.MinX, Envelope.MinY);
		cairo_stroke (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
	}

	cairo_pattern_destroy (pattern);
}

void wxGISDisplay::DrawGeometry(OGRGeometry* poGeometry)
{
    if(!poGeometry)
        return;
	wxCriticalSectionLocker locker(m_CritSect);

	OGRwkbGeometryType type = wkbFlatten(poGeometry->getGeometryType());
	switch(type)
	{
	case wkbPoint:
		if(m_dLineWidth)
		{
			SetColor(m_stPointColour);
			cairo_set_line_width(m_saLayerCaches[m_nCurrentLayer].pCairoContext, m_dPointRadius + m_dPointRadius + m_dLineWidth);
			if(!DrawPointFast((OGRPoint*)poGeometry))
				break;
			cairo_stroke (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
			cairo_set_line_width(m_saLayerCaches[m_nCurrentLayer].pCairoContext, m_dPointRadius +m_dPointRadius);
		}
		SetColor(m_stFillColour);
		if(!DrawPointFast((OGRPoint*)poGeometry))
			return;
		cairo_stroke (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
		break;
	case wkbLineString:
		cairo_set_line_width(m_saLayerCaches[m_nCurrentLayer].pCairoContext, m_dLineWidth);
		//check if line is too small
		if(!CheckDrawAsPoint(poGeometry))
			if(!DrawLine((OGRLineString*)poGeometry))
				break;
		SetColor(m_stLineColour);
		cairo_stroke (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
		break;
	case wkbLinearRing:
		cairo_set_line_width(m_saLayerCaches[m_nCurrentLayer].pCairoContext, m_dLineWidth);
		//check if line is too small
		if(!CheckDrawAsPoint(poGeometry))
			if(!DrawRing((OGRLinearRing*)poGeometry))
				break;
		SetColor(m_stLineColour);
		cairo_stroke (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
		break;
	case wkbPolygon:
		cairo_set_line_width(m_saLayerCaches[m_nCurrentLayer].pCairoContext, m_dLineWidth);
		//check if line is too small
		if(!CheckDrawAsPoint(poGeometry))
			if(!DrawPolygon((OGRPolygon*)poGeometry))
				break;
		SetColor(m_stLineColour);
		cairo_stroke (m_saLayerCaches[m_nCurrentLayer].pCairoContext);
		break;
	case wkbMultiPolygon:
	case wkbMultiPoint:
	case wkbMultiLineString:
	case wkbGeometryCollection:
		{
		OGRGeometryCollection* pOGRGeometryCollection = (OGRGeometryCollection*)poGeometry;
		for(int i = 0; i < pOGRGeometryCollection->getNumGeometries(); ++i)
			DrawGeometry(pOGRGeometryCollection->getGeometryRef(i));
		}
		break;
	default:
	case wkbUnknown:
	case wkbNone:
		break;
	}
}

OGREnvelope wxGISDisplay::TransformRect(wxRect &rect)
{
	OGREnvelope out;
	double dX1, dX2, dY2, dY1;
	double dWHalf = double(rect.width) / 2;
	double dHHalf = double(rect.height) / 2;
	double dXCenter = rect.x + dWHalf, dYCenter = rect.y + dHHalf;
	DC2World(&dXCenter, &dYCenter);

	cairo_matrix_t InvertMatrix = {m_pDisplayMatrixNoRotate->xx, m_pDisplayMatrixNoRotate->yx, m_pDisplayMatrixNoRotate->xy, m_pDisplayMatrixNoRotate->yy, m_pDisplayMatrixNoRotate->x0, m_pDisplayMatrixNoRotate->y0};
	cairo_matrix_invert(&InvertMatrix);

	cairo_matrix_transform_distance(&InvertMatrix, &dWHalf, &dHHalf);

	dX1 = dXCenter - dWHalf;
	dX2 = dXCenter + dWHalf;
	dY1 = dYCenter - dHHalf;
	dY2 = dYCenter + dHHalf;

    out.MinX = std::min(dX1, dX2);
	out.MinY = std::min(dY1, dY2);
	out.MaxX = std::max(dX1, dX2);
	out.MaxY = std::max(dY1, dY2);
	return out;
}

void wxGISDisplay::SetRotate(double dAngleRad)
{
	m_dAngleRad = dAngleRad;
    //for rotate panning & zooming
	m_CurrentBoundsRotated = m_CurrentBounds;
	m_dRotatedBoundsCenterX = m_CurrentBoundsRotated.MinX + (m_CurrentBoundsRotated.MaxX - m_CurrentBoundsRotated.MinX) / 2;
	m_dRotatedBoundsCenterY = m_CurrentBoundsRotated.MinY + (m_CurrentBoundsRotated.MaxY - m_CurrentBoundsRotated.MinY) / 2;
	if(!IsDoubleEquil(m_dAngleRad, 0.0))
	{
		RotateEnvelope(m_CurrentBoundsRotated, m_dAngleRad, m_dRotatedBoundsCenterX, m_dRotatedBoundsCenterY);
		SetEnvelopeRatio(m_CurrentBoundsRotated, m_dFrameRatio);//test
	}
	m_CurrentBoundsX8 = m_CurrentBoundsRotated;
	IncreaseEnvelope(m_CurrentBoundsX8, 8);

	//compute current transform matrix
	InitTransformMatrix();

	SetDerty(true);
}

void wxGISDisplay::SetColor(wxGISEnumDrawStyle eStyle, const WXGISRGBA &Color)
{
    switch(eStyle)
    {
    case enumGISDrawStyleFill:
        m_stFillColour = Color;
        break;
    case enumGISDrawStyleOutline:
        m_stLineColour = Color;
        break;
    case enumGISDrawStylePoint:
        m_stPointColour = Color;
        break;
    default:
    case enumGISDrawStyleNone:
        break;
    };
}


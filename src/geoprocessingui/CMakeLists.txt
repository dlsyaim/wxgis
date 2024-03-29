# **************************************************************************** 
# * Project:  wxGIS
# * Purpose:  cmake script
# * Author:   Baryshnikov Dmitriy (aka Bishop), polimax@mail.ru
# ****************************************************************************
# *   Copyright (C) 2013 Bishop
# *
# *    This program is free software: you can redistribute it and/or modify
# *    it under the terms of the GNU General Public License as published by
# *    the Free Software Foundation, either version 3 of the License, or
# *    (at your option) any later version.
# *
# *    This program is distributed in the hope that it will be useful,
# *    but WITHOUT ANY WARRANTY; without even the implied warranty of
# *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *    GNU General Public License for more details.
# *
# *    You should have received a copy of the GNU General Public License
# *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
# ****************************************************************************
cmake_minimum_required (VERSION 2.8)
set(PROJECT_NAME geoprocessingui)

include(lib)
include(common)

set(LIB_HEADERS ${WXGIS_CURRENT_SOURCE_DIR}/include/wxgis/geoprocessingui)
set(LIB_SOURCES ${WXGIS_CURRENT_SOURCE_DIR}/src/geoprocessingui)

if(WIN32)
    set(wxWidgets_EXCLUDE_COMMON_LIBRARIES TRUE)
endif(WIN32)

find_package(wxWidgets 2.9 REQUIRED core base)
# wxWidgets include (this will do all the magic to configure everything)
if(wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
endif(wxWidgets_FOUND)

if(UNIX)
    find_package(GTK2 REQUIRED)
    if(GTK2_FOUND)
        include_directories(${GTK2_INCLUDE_DIRS})
        add_definitions(-DHAVE_GTK2)
    endif(GTK2_FOUND)
    message(STATUS "GTK2 libs=[${GTK2_LIBRARIES}] headers=[${GTK2_INCLUDE_DIRS}]")
endif()

find_package(CAIRO REQUIRED)
if(CAIRO_FOUND)
    include_directories(${CAIRO_INCLUDE_DIR})
    add_definitions(-DHAVE_CAIRO)
    set(wxGIS_USE_CAIRO ON)
endif(CAIRO_FOUND)

include_directories(${LIB_HEADERS})

set(PROJECT_HHEADERS ${PROJECT_HHEADERS} 
    ${LIB_HEADERS}/geoprocessingcmd.h
    ${LIB_HEADERS}/geoprocessingui.h
    ${LIB_HEADERS}/gpcontrols.h
    ${LIB_HEADERS}/gptaskexecdlg.h
    ${LIB_HEADERS}/gptoolbox.h
    ${LIB_HEADERS}/gptoolboxview.h
    ${LIB_HEADERS}/gptooldlg.h
    ${LIB_HEADERS}/gxtoolexecview.h
    ${LIB_HEADERS}/taskcmd.h
    ${LIB_HEADERS}/droptarget.h
)

set(PROJECT_CSOURCES ${PROJECT_CSOURCES}   
    ${LIB_SOURCES}/geoprocessingcmd.cpp
    ${LIB_SOURCES}/gpcontrols.cpp
    ${LIB_SOURCES}/gptaskexecdlg.cpp
    ${LIB_SOURCES}/gptoolbox.cpp
    ${LIB_SOURCES}/gptoolboxview.cpp
    ${LIB_SOURCES}/gptooldlg.cpp
    ${LIB_SOURCES}/gxtoolexecview.cpp
    ${LIB_SOURCES}/taskcmd.cpp 
    ${LIB_SOURCES}/droptarget.cpp 
)

add_definitions(-DWXMAKINGDLL_GIS_GPU)
set( WXGISGEOPROCESSINGUI_LIB_NAME ${LIB_NAME} PARENT_SCOPE )

add_library(${LIB_NAME} SHARED ${PROJECT_HHEADERS} ${PROJECT_CSOURCES})

if(wxWidgets_FOUND)
   target_link_libraries(${LIB_NAME} ${wxWidgets_LIBRARIES})
endif(wxWidgets_FOUND)

if(GDAL_FOUND)
    target_link_libraries(${LIB_NAME} ${GDAL_LIBRARIES})
endif(GDAL_FOUND) 

if(CAIRO_FOUND)
    target_link_libraries(${LIB_NAME} ${CAIRO_LIBRARIES})
endif(CAIRO_FOUND)

if(UNIX)
    if(GTK2_FOUND)
        target_link_libraries(${LIB_NAME} ${GTK2_LIBRARIES})
    endif(GTK2_FOUND)
endif(UNIX)

target_link_libraries(${LIB_NAME} ${WXGISCORE_LIB_NAME} ${WXGISDATASOURCE_LIB_NAME} ${WXGISFRAMEWORK_LIB_NAME} ${WXGISCATALOG_LIB_NAME} ${WXGISCATALOGUI_LIB_NAME} ${WXGISGEOPROCESSING_LIB_NAME}) #${WXGISDISPLAY_LIB_NAME}

include(installlib)

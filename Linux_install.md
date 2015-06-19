# Introduction #

This topic describes Linux install. There is the Ubuntu package on PPA now (https://launchpad.net/~bishop-gis/+archive/wxgis)

# Details #

**To instal wxGIS on Ubuntu your need:**
  * Add new source in your sources list (deb http://ppa.launchpad.net/bishop-gis/wxgis/ubuntu lucid main )
  * Update package manager
  * Install package wxgis0

**To instal wxGIS from sources your need:**
  * Download and unpack wxgis-version.tar.gz
  * cd to unpacked directory
  * run:
```
make lib
sudo make install_lib
make app
sudo make install_app
```

Before your run:
  * Install wxGISGDAL (http://code.google.com/p/wxgisgdal/downloads/list)
  * Look carefully install paths in Makefile, ./build/make/makefie ./build/make/`*`.mak and fix them if needed
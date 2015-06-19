# Introduction #

The 0.3 version of wxGIS introduce new and more simple mechanism of changing application localization (see [Localization](Localization.md) page). This page describe the locale files create process.


# Details #

From the beginning to create locale files you need:
  1. Download and install poEdit (http://www.poedit.net/download.php)
  1. Download sources (http://code.google.com/p/wxgis/source/checkout), there are several tags for different versions of wxGIS (see http://wxgis.googlecode.com/svn/tags/). For Microsoft Windows I recommend TortoiseSVN, for Ubuntu - RabbitSVN.

The next step is creating catalog for each part of wxGIS (recommended) but you can combine all translations in one big file.

To create a translation you have to run poEdit and click File->New catalog...
<p><img src='http://wxgis.googlecode.com/svn/wiki/WikiPageName.attach/create_cat.png' /></p>

Set your name and email
<p><img src='http://wxgis.googlecode.com/svn/wiki/WikiPageName.attach/settings_dlg.png' /></p>

Set paths. At least you have to add one path to directory in src folder and one to directory in include/wxgis folder (see picture).
<p><img src='http://wxgis.googlecode.com/svn/wiki/WikiPageName.attach/settings1_dlg.png' /></p>

Save catalog in loc/your\_locale (eg. English - loc/en, Russian - /loc/ru etc.) and with name ended _**cat.po (see picture). The wxGISCatalog search it's locale files using this mask.**<p><img src='http://wxgis.googlecode.com/svn/wiki/WikiPageName.attach/cat_save_dlg.png' /></p>_

The main window should be shown and filled with text string to translate at next step. **IMPORTANT! During translation you should check the number of formating characters (eg. %s %d %.2f etc.). You shouldn't remove this characters! But you can put them at any place of string (example of changing position of formating characters: "error no %d in %s, value %f" -> "%d error of %f in %s")**
<p><img src='http://wxgis.googlecode.com/svn/wiki/WikiPageName.attach/main_wnd.png' /></p>

To control translation you can create a project in catalogs manager. In translation project should be shown all translations and theirs state. If you update sources you’ll see changes in catalog manager.
<p><img src='http://wxgis.googlecode.com/svn/wiki/WikiPageName.attach/manager.png' /></p>

To create translation project push first button in catalog manager and fill dialog.
<p><img src='http://wxgis.googlecode.com/svn/wiki/WikiPageName.attach/edit_project.png' /></p>

After the translation you can test the result on your wxGISCatalog installation. To do it:
- create in folder there wxGISCatalog.exe is situated sub-folder named "locale" (if it's not present)
- create sub-folder with your locale in locale folder (eg. locale/en).
- copy all **.mo files to this folder.
- follow instructions of [Localization](Localization.md) page.**

**If you don't want to create numerous catalogs – also you can get locale files from other language and copy them to your language folder** (for example, from ...wxGIS\loc\ru to ...wxGIS\loc\en).
Than you can create translation project as it was written upper and set new paths to that files in each catalog configures and rewrite locale strings to your language.

**Please send me your translations to put them into the distributive of wxGIS!**
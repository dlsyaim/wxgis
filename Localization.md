# Introduction #

wxGIS have standard localization capabilities. The locale files (`*`.mo) install in target system dependent of OS.

# Details #

In MS Windows the files put in target directory in subdirectrory named "locale".
For standalone version the "locale" situated in root of program folder.
In Linux the locale files put on standart path.

But, there are some special things.
The locale subdirectory include several direcories of national language files.
For example, for russian language the path will be **locale->ru**, for english - **locale->en** and etc.
The current locale, sets in xml config file in section "loc" like this:
```
<loc locale="en" path="d:\work\Projects\wxGIS\build\debug\locale"/>
```
You can manually change it.
If you delete all contents of locale directory you get the pure english application. The locale directory should not been deleted!

P.S. The configs structure describes in [Configs](Configs.md) page

The version 0.3 and above will have options dialog to set the config values fron UI.

![http://wxgis.googlecode.com/svn/wiki/WikiPageName.attach/options(misc).jpg](http://wxgis.googlecode.com/svn/wiki/WikiPageName.attach/options(misc).jpg)
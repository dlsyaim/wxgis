# Introduction #

The wxGIS have several config like the MS Windows registry (the HKLM and HKCU).


# Details #

The HKLM is read onle common config for all users of target PC. The HKLM fills from the config subdirectory which places in target directory there wxGIS installed.
The config subdirectory includes several xml config files.

The HKCU includes configs for curretn user. The config files created in user home.


**The config files structure**

---

root->config<p>
<p>wxCatalog.xml<br>
<p>wxGISCatalog.xml<br>
<p>wxGISContDialog.xml<br>
<p>wxGISObjDialog.xml<br>
<hr />
user_home->wxGIS<p>
<p>\wxCatalog\hkcu_config.xml<br>
<p>\wxGISCatalog\hkcu_config.xml<br>
<p>\wxGISContDialog\hkcu_config.xml<br>
<p>\wxGISObjDialog\hkcu_config.xml<br>
<hr />
app_data->wxGIS<p>
<p>\wxCatalog\hklm_config.xml<br>
<p>\wxGISCatalog\hklm_config.xml<br>
<p>\wxGISContDialog\hklm_config.xml<br>
<p>\wxGISObjDialog\hklm_config.xml<br>
<hr />
<b>The config file structure</b>

TODO
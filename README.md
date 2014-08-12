zbrowse
=======

Zillow browser (Qt app for Windows/Mac/Linux)

This is a work in progress. It is still in early development although it offers some useable functionality.

This native GUI application uses the Qt web browser widget to browse the zillow.com site and save real estate
listings (US only, currently) to a local database. Listings can be ranked using different user-defined criteria.

Current limitations
The Qt Webkit browser may sometimes crash due to a known issue with CSS3 animations
fixed after Qt 5.2.1

The Webkit browser also does not handle all asynchronous updates as expected. This affects
some of the Zillow listing information and also means the image browser does not work.

The distance to a point currently only supports signed decimal latitude,longitude as in
the example coordinates provided.

As of 0.9.2 there is still a known bug with fetch if any column sort modifiers are in effect.
Data will be written to the wrong row.

Requires QtXlsxWriter (available on Github). Clone as a sibling of the project directory for zillowbrowse

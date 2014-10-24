// parsefns.cpp - HTML parsing and processing functions

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "sqldefs.h"
#include "columnids.h"

#include <QWebFrame>
#include <QWebView>
#include <QtWebKit>

#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QDebug>

#include "math.h"

void MainWindow::onIdleTimer()
{
    if (m_loadBusy)
    {
        QTimer::singleShot(1500, this, SLOT(onIdleTimer()));
        qDebug() << "Still busy, rescheduled";
        return;
    }
    // Monitored urls are loaded via fetch
    // Unmonitored urls may be a list we need to parse
    QString sUrl(ui->webView->url().toString());
    if (!m_urlToRow.contains(sUrl))
    {
        qDebug() << "idle: start non-indexed page";
        processWebView();
        m_pageProcessingPending = 0;
        return;
    }
    int rowIndex = m_urlToRow[sUrl];
    if (safeItemText(rowIndex, COL_URL) != sUrl)
    {
        logMsg( "Warning: mismatch, expected " + sUrl + " got " + safeItemText(rowIndex, COL_URL), 0 );
        m_pageProcessingPending = 0;
        return;
    }
    QWebPage *webWindow = ui->webView->page();
    if (webWindow == NULL)
    {
        qDebug() << "idle: null window";
        m_pageProcessingPending = 0;
        return;
    }
    QWebFrame *mainFrame = webWindow->mainFrame();
    if (mainFrame == NULL)
    {
        qDebug() << "idle: null main frame";
        m_pageProcessingPending = 0;
        return;
    }
    qDebug() << "idle: start page, row" << rowIndex;
    startPlay(m_pcmBeep1);
    QString s( mainFrame->toHtml() );
    // Depending on entry we may not have URL set in table
    if (safeItemText(rowIndex, COL_URL) != sUrl)
    {
        qDebug() << "Asserting url:" << sUrl;
        ui->tblData->setItem(rowIndex, COL_URL, new QTableWidgetItem(sUrl));
    }
    //qDebug() << "end html";
    /*** example of raw data we're interested in ***
     * Snippet 1:

                var _gaqBackup = _gaqBackup || [];
                </script><link href="http://www.zillowstatic.com/vstatic/df5c99b/static/css/master.css" type="text/css" rel="stylesheet" media="all"><link rel="stylesheet" type="text/css" media="all" href="http://www.zillowstatic.com/c/df5c99b/lib?css/z-themes/tengage-upsells.css&amp;css/z-modules/signature-refactor.css&amp;css/z-modules/contact-form-template-refactor.css&amp;css/z-modules/breadcrumb.css&amp;css/z-modules/hdp-locked-upsell.css&amp;css/z-modules/ui-modal.css&amp;css/z-modules/gallery-upsell.css&amp;css/z-modules/map-button.css&amp;css/z-modules/schools-nearby-schools.css&amp;css/z-modules/schools-great-school-badges.css"><link href="http://www.zillowstatic.com/vstatic/df5c99b/static/css/z-pages/rental-hdp.css" type="text/css" rel="stylesheet" media="all"><link href="http://www.zillowstatic.com/static-hi/360d391/static-hi/css/z-modules/digs-gallery-save.css" type="text/css" rel="stylesheet" media="all"><link href="http://www.zillowstatic.com/vstatic/df5c99b/static/css/z-modules/footer-seo-2.css" type="text/css" rel="stylesheet" media="all"><!--[if (IE 7)]><link href="http://www.zillowstatic.com/vstatic/df5c99b/static/css/ie7.css" type="text/css" rel="stylesheet" media="screen"/><![endif]--><!--[if (lte IE 8)]><link href="http://www.zillowstatic.com/vstatic/df5c99b/static/css/z-modules/ie-messaging.css" type="text/css" rel="stylesheet" media="screen"/><![endif]--><script src="http://www.zillowstatic.com/static/js/modernizr.custom.zillow.js" type="text/javascript"></script><link rel="start" title="Zillow home" href="/"><meta content="width=device-width, initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0, user-scalable=no" name="viewport"><meta name="apple-mobile-web-app-capable" content="yes"><meta name="apple-mobile-web-app-status-bar-style" content="black-translucent"><link rel="apple-touch-icon" href="http://www.zillowstatic.com/static/images/m/apple-touch-icon.png"><!--[if gte IE 9]><link rel="shortcut icon" href="/static/images/ie9_favicon.ico" type="image/x-icon"/><![endif]--><meta name="ROBOTS" content="ALL"><meta name="ROBOTS" content="NOYDIR"><meta name="ROBOTS" content="NOODP"><meta name="author" content="Zillow, Inc."><meta name="Copyright" content="Copyright (c) 2006-2013 Zillow, Inc."><meta content="691f1bfccade71b5-c065751219a379dd-g64cedb67f5ea020a-a" name="google-translate-customization"><meta property="fb:admins" content="202692,878610170,662000799,100001769907023,10716009,769244502,10716649,503322863"><meta property="fb:app_id" content="172285552816089"><meta property="og:type" content="zillow_fb:home"><meta property="og:zillow_fb:address" content="911 Mallard Pointe Dr, Cedar Hill, TX 75104"><meta property="zillow_fb:beds" content="4"><meta property="zillow_fb:baths" content="3"><meta property="zillow_fb:description" content="For sale: $323,000. BREATHTAKING VIEWS-One Acre Treed Homesite!! Backyard Oasis-Amazing Pool with View of Community Pond...Only Few Homes Have This! Upstairs Loft Pond-Lake View! Slate Floors Accent Wide Open Kitchen-Amazing Pantry Space*Private Porch off Nook! Master has Sitting Space*Fireplace*Private Porch*Sunken Tub*Oversized Shower*2 Closets"><meta property="og:url" content="http://www.zillow.com/homedetails/911-Mallard-Pointe-Dr-Cedar-Hill-TX-75104/27062400_zpid/"><meta property="og:title" content="Cedar Hill Home For Sale"><meta property="og:image" content="http://photos2.zillowstatic.com/p_d/IShjyqwv5uw8c21000000000.jpg"><meta property="og:description" content="For sale: $323,000. BREATHTAKING VIEWS-One Acre Treed Homesite!! Backyard Oasis-Amazing Pool with View of Community Pond...Only Few Homes Have This! Upstairs Loft Pond-Lake View! Slate Floors Accent Wide Open Kitchen-Amazing Pantry Space*Private Porch off Nook! Master has Sitting Space*Fireplace*Private Porch*Sunken Tub*Oversized Shower*2 Closets"><link rel="alternate" href="android-app://com.zillow.android.zillowmap/http/www.zillow.com/homedetails/911-Mallard-Pointe-Dr-Cedar-Hill-TX-75104/27062400_zpid/"><script charset="utf-8" id="yui_3_15_0_1_1407415446921_2" src="http://www.zillowstatic.com/c/yui?3.15.0/attribute-core/attribute-core-min.js&amp;3.15.0/attribute-observable/attribute-observable-min.js&amp;3.15.0/attribute-extras/attribute-extras-min.js&amp;3.15.0/attribute-base/attribute-base-min.js&amp;3.15.0/base-core/base-core-min.js&amp;3.15.0/base-observable/base-observable-min.js&amp;3.15.0/base-base/base-base-min.js&amp;3.15.0/base-build/base-build-min.js&amp;3.15.0/node-style/node-style-min.js&amp;3.15.0/selector-css2/selector-css2-min.js&amp;3.15.0/selector-css3/selector-css3-min.js&amp;3.15.0/plugin/plugin-min.js&amp;3.15.0/node-pluginhost/node-pluginhost-min.js&amp;3.15.0/dom-screen/dom-screen-min.js&amp;3.15.0/event-custom-complex/event-custom-complex-min.js&amp;3.15.0/event-synthetic/event-synthetic-min.js&amp;3.15.0/event-resize/event-resize-min.js&amp;3.15.0/yui-throttle/yui-throttle-min.js&amp;3.15.0/event-delegate/event-delegate-min.js&amp;3.15.0/node-event-delegate/node-event-delegate-min.js&amp;3.15.0/arraylist/arraylist-min.js&amp;3.15.0/attribute-complex/attribute-complex-min.js&amp;3.15.0/base-pluginhost/base-pluginhost-min.js&amp;3.15.0/classnamemanager/classnamemanager-min.js&amp;3.15.0/event-focus/event-focus-min.js&amp;3.15.0/widget-base/widget-base-min.js&amp;3.15.0/widget-htmlparser/widget-htmlparser-min.js&amp;3.15.0/widget-skin/widget-skin-min.js&amp;3.15.0/widget-uievents/widget-uievents-min.js&amp;3.15.0/widget-parent/widget-parent-min.js&amp;3.15.0/widget-child/widget-child-min.js&amp;3.15.0/tabview-base/tabview-base-min.js&amp;3.15.0/node-screen/node-screen-min.js&amp;3.15.0/event-simulate/event-simulate-min.js&amp;3.15.0/async-queue/async-queue-min.js&amp;3.15.0/gesture-simulate/gesture-simulate-min.js&amp;3.15.0/node-event-simulate/node-event-simulate-min.js&amp;3.15.0/event-key/event-key-min.js&amp;3.15.0/node-focusmanager/node-focusmanager-min.js&amp;3.15.0/tabview/tabview-min.js&amp;3.15.0/pluginhost-base/pluginhost-base-min.js&amp;3.15.0/pluginhost-config/pluginhost-config-min.js" async=""></script><script charset="utf-8" id="yui_3_15_0_1_1407415446921_3" src="http://www.zillowstatic.com/c/df5c99b/lib?js/src/ux/html5/attr-placeholder.js&amp;js/z-get/z-get-min.js&amp;js/z-analytics/z-analytics-min.js&amp;js/src/zillow/linktrack.js&amp;js/z-engagement-analytics/z-engagement-analytics-min.js&amp;js/z-managed-base/z-managed-base-min.js&amp;js/z-event-handle-manager/z-event-handle-manager-min.js&amp;js/z-tab-content-mgr/z-tab-content-mgr-min.js&amp;js/src/zillow/user/plugins/AuthRequiredPlugin.js&amp;js/src/dom/toggle.js&amp;js/zmm/zmm-analytics.js" async=""></script><script charset="utf-8" id="yui_3_15_0_1_1407415446921_272" src="http://www.zillowstatic.com/c/yui?3.15.0/transition/transition-min.js&amp;3.15.0/event-touch/event-touch-min.js&amp;3.15.0/event-tap/event-tap-min.js&amp;3.15.0/event-outside/event-outside-min.js&amp;3.15.0/event-mouseenter/event-mouseenter-min.js&amp;3.15.0/event-hover/event-hover-min.js" async=""></script><script charset="utf-8" id="yui_3_15_0_1_1407415446921_273" src="http://www.zillowstatic.com/c/df5c99b/lib?js/z-account-popup-message/z-account-popup-message-min.js&amp;js/z-node-dataset/z-node-dataset-min.js&amp;js/z-plugin-nav/z-plugin-nav-min.js&amp;js/z-event-mouse-touch/z-event-mouse-touch-min.js" async=""></script><link charset="utf-8" rel="stylesheet" id="yui_3_15_0_1_1407415446921_320" href="http://www.zillowstatic.com/c/df5c99b/lib?css/z-modules/hdp-auto-quotes.css"><script charset="utf-8" id="yui_3_15_0_1_1407415446921_321" src="http://www.zillowstatic.com/c/yui?3.15.0/array-extras/array-extras-min.js&amp;3.15.0/querystring-stringify-simple/querystring-stringify-simple-min.js&amp;3.15.0/io-base/io-base-min.js&amp;3.15.0/io-form/io-form-min.js&amp;3.15.0/json-parse/json-parse-min.js&amp;3.15.0/jsonp/jsonp-min.js&amp;3.15.0/escape/escape-min.js&amp;3.15.0/model/model-min.js&amp;3.15.0/view/view-min.js&amp;3.15.0/jsonp-url/jsonp-url-min.js&amp;3.15.0/datatype-date-parse/datatype-date-parse-min.js&amp;3.15.0/datatype-date-format/datatype-date-format-min.js&amp;3.15.0/datatype-number-format/datatype-number-format-min.js&amp;3.15.0/event-move/event-move-min.js&amp;3.15.0/querystring-parse-simple/querystring-parse-simple-min.js&amp;3.15.0/querystring-parse/querystring-parse-min.js&amp;3.15.0/widget-position/widget-position-min.js&amp;3.15.0/widget-stack/widget-stack-min.js&amp;3.15.0/node-event-html5/node-event-html5-min.js&amp;3.15.0/cookie/cookie-min.js&amp;3.15.0/json-stringify/json-stringify-min.js&amp;gallery/gallery-2013.08.22-21-03/gallery-storage-lite/gallery-storage-lite-min.js&amp;3.15.0/event-flick/event-flick-min.js&amp;3.15.0/array-invoke/array-invoke-min.js&amp;3.15.0/querystring-stringify/querystring-stringify-min.js&amp;3.15.0/timers/timers-min.js&amp;3.15.0/promise/promise-min.js&amp;3.15.0/tree-node/tree-node-min.js&amp;3.15.0/tree/tree-min.js&amp;3.15.0/tree-sortable/tree-sortable-min.js&amp;3.15.0/intl/intl-min.js&amp;3.15.0/datatype-date-format/lang/datatype-date-format.js" async=""></script><script charset="utf-8" id="yui_3_15_0_1_1407415446921_322" src="http://www.zillowstatic.com/c/df5c99b/lib?js/src/util/zutils.js&amp;js/z-jsonp/z-jsonp-min.js&amp;js/src/ux/AjaxFormExt.js&amp;js/src/ux/AjaxFormBase.js&amp;js/src/ux/AjaxFormWrapper.js&amp;js/src/ux/plugins/MenuNavPlugin.js&amp;js/src/zillow/geobreadcrumb.js&amp;js/src/zillow/hdp/ActionBar.js&amp;js/z-paparazzi-chart-model/z-paparazzi-chart-model-min.js&amp;js/src/ux/chart/ChartFormatters.js&amp;js/z-paparazzi-legend/z-paparazzi-legend-min.js&amp;js/z-paparazzi-chart-interaction/z-paparazzi-chart-interaction-min.js&amp;js/z-paparazzi-chart-view/z-paparazzi-chart-view-min.js&amp;js/z-paparazzi-chart-upsell/z-paparazzi-chart-upsell-min.js&amp;js/src/ux/plugins/MetricStateControllerPlugin.js&amp;js/src/ux/MetricState.js&amp;js/z-plugin-chart-nav/z-plugin-chart-nav-min.js&amp;js/z-paparazzi-app/z-paparazzi-app-min.js&amp;js/src/zillow/hdp/HDPChartLoader.js&amp;js/z-iframe-refresh/z-iframe-refresh-min.js&amp;js/src/zillow/hdp/HDPIFrameRefresh.js&amp;js/src/ux/Tooltip.js&amp;js/z-abstract-toggle-content/z-abstract-toggle-content-min.js&amp;js/z-app-history-shim/z-app-history-shim-min.js&amp;js/z-base-component-mgr/z-base-component-mgr-min.js&amp;js/z-async-content-mgr/z-async-content-mgr-min.js&amp;js/src/ux/plugins/BlockIOPlugin.js&amp;js/src/util/WidgetAsyncBlock.js&amp;js/src/ux/AsyncBlock.js&amp;js/src/zillow/complaints.js&amp;js/z-complaint-manager-async-block/z-complaint-manager-async-block-min.js&amp;js/z-toggle-content/z-toggle-content-min.js&amp;js/z-expando-table/z-expando-table-min.js&amp;js/z-hdp-auto-quotes-trigger/z-hdp-auto-quotes-trigger-min.js&amp;js/src/util/Formatters.js&amp;js/zmm/zmm-loan-request.js&amp;js/zmm/zmm-form-base.js&amp;js/zmm/zmm-form-extras.js&amp;js/zmm/zmm-form-persistence-plugin.js&amp;js/src/dom/HiddenBlock.js&amp;js/zmm/zmm-api.js&amp;js/z-hdp-auto-quotes/z-hdp-auto-quotes-min.js&amp;js/z-hdp-google-street-view/z-hdp-google-street-view-min.js&amp;js/z-bing-map-sdk/z-bing-map-sdk-min.js&amp;js/z-map-marker/z-map-marker-min.js&amp;js/z-bing-map-marker/z-bing-map-marker-min.js&amp;js/z-hdp-map-view/z-hdp-map-view-min.js&amp;js/z-hdp-other-costs/z-hdp-other-costs-min.js&amp;js/z-plugin-carousel-nav/z-plugin-carousel-nav-min.js" async=""></script><script charset="utf-8" id="yui_3_15_0_1_1407415446921_323" src="http://www.zillowstatic.com/c/df5c99b/lib?js/z-plugin-carousel-sync/z-plugin-carousel-sync-min.js&amp;js/src/ux/LightboxManager.js&amp;js/z-photo-carousel/z-photo-carousel-min.js&amp;js/z-photo-carousel-peek/z-photo-carousel-peek-min.js&amp;js/z-hdp-photo-carousel/z-hdp-photo-carousel-min.js&amp;js/z-hdp-price-history/z-hdp-price-history-min.js&amp;js/z-plugin-collapse-contact-info/z-plugin-collapse-contact-info-min.js&amp;js/z-plugin-hdp-facts-suite/z-plugin-hdp-facts-suite-min.js&amp;js/z-plugin-message/z-plugin-message-min.js&amp;js/z-plugin-nav-overflow/z-plugin-nav-overflow-min.js&amp;js/z-plugin-sticky-action-bar/z-plugin-sticky-action-bar-min.js&amp;js/z-scroll-event-tracker/z-scroll-event-tracker-min.js&amp;js/src/util/AsyncLoader.js&amp;js/z-jsonp-page-attrs/z-jsonp-page-attrs-min.js&amp;js/z-main-nav-async-model/z-main-nav-async-model-min.js&amp;js/z-server-log/z-server-log-min.js&amp;js/z-base-jsonp-view/z-base-jsonp-view-min.js&amp;js/z-view-base/z-view-base-min.js&amp;js/z-main-nav-async-view/z-main-nav-async-view-min.js&amp;js/src/zillow/contact/ContactPluginExt.js&amp;js/z-plugin-email-format/z-plugin-email-format-min.js&amp;js/src/zillow/contact/HDPContactPlugin.js&amp;js/z-map-lat-lon/z-map-lat-lon-min.js&amp;js/resurrection/map/tiling/SpriteMapUtil.js&amp;js/src/zillow/hdp/map/HDPMapLoader.js" async=""></script><link charset="utf-8" rel="stylesheet" id="yui_3_15_0_1_1407415446921_324" href="http://www.zillowstatic.com/c/df5c99b/lib?css/z-modules/hdp-auto-quotes.css"></head>
<body id="hdp" class="hc-header-merge-14q1 header-fresh zss-no-touch is-responsive zss-legacy-layout tengage wide">
<a name="top" id="top"></a><div id="wrapper" class="homedetails-wrap"><header class=" zss-header"><nav class="nav-advice-ab nav-wrapper"><div id="header" class="page-width-wrapper"><a class="skiplink" href="#contentstart">Skip to content</a><a href="http://www.zillow.com/" title="Zillow Real Estate" class="zss-logo zss-logo-link" accesskey="1"><span class="zss-logo-text">Zillow Real Estate &amp;amp; Homes for Sale</span></a><div class="zss-mobile-header"><div class="responsive-page-title">911 Mallard Pointe Dr, Cedar Hill, TX 75104 is For Sale</div></div><div class="nav-main"><div class="nav-primary"><ul class="l-inline zss-nav dropdown-hoverable no-ancillary-nav"><li data-nav-level="1" class="dropdown homes-tab top-nav-tab rollable"><a href="/homes/for_sale/" data-za-category="TopNav" class="is-selected nav-item nav-primary-item nav-item-button dropdown-trigger responsive-nav-item" data-za-label="None" id="top-nav-homes" data-overflow-item-class="zss-responsive-nav-item" title="Homes" data-za-action="Homes">Homes</a><div class="dropdown-bd"><ul class="dropdown-content nav l-stacked"><li><a href="/grand-prairie-tx/" data-za-category="TopNav" class="nav-item za-track-event" data-za-label="Homes For Sale" title="Grand Prairie Homes For Sale" data-za-action="Homes"><span class="region-name">Grand Prairie</span> Homes For Sale</a></li><li><a href="/grand-prairie-tx/fsbo/" data-za-category="TopNav" class="nav-item za-track-event" data-za-label="For Sale By Owner" title="Grand Prairie For Sale By Owner" data-za-action="Homes"><span class="region-name">Grand Prairie</span> For Sale By Owner</a></li><li><a href="/grand-prairie-tx/foreclosures/" data-za-category="TopNav" class="nav-item za-track-event" data-za-label="Foreclosures" title="Grand Prairie Foreclosures" data-za-action="Homes"><span class="region-name">Grand Prairie</span> Foreclosures</a></li><li><a href="/grand-prairie-tx/open-house/" data-za-category="TopNav" class="nav-item za-track-event" data-za-label="Open Houses" title="Grand Prairie Open Houses" data-za-action="Homes"><span class="region-name">Grand Prairie</span> Open Houses</a></li><li><a href="/grand-prairie-tx/new/" data-za-category="TopNav" class="nav-item za-track-event" data-za-label="New Homes" title="Grand Prairie New Homes" data-za-action="Homes"><span class="region-name">Grand Prairie</span> New Homes</a></li><li><a href="/grand-prairie-tx/coming-soon/" data-za-category="TopNav" class="nav-item za-track-event" data-za-label="Coming Soon Homes" title="Grand Prairie Coming Soon Homes" data-za-action="Homes"><span class="region-name">Grand Prairie</span> Coming Soon Homes</a></li><li><a href="/grand-prairie-tx/sold/" data-za-category="TopNav" class="nav-item za-track-event" data-za-label="Recent Home Sales" title="Grand Prairie Recent Home Sales" data-za-action="Homes"><span class="region-name">Grand Prairie</span> Recent Home Sales</a></li><li><a href="/grand-prairie-tx/expensive-homes/" title="Grand Prairie Most Expensive Homes" data-za-category="TopNav" data-za-action="Homes" class="nav-item za-track-event" data-za-label="Most Expensive Homes"><span class="region-name">Grand Prairie</span> Most Expensive Homes</a></li><li><a href="/homes/browse/Grand Prairie-TX/" title="Browse Grand Prairie  Homes" data-za-category="TopNav" data-za-action="Homes" class="nav-item za-track-event" data-za-label="Browse All Homes">Browse <span class="region-name">Grand Prairie</span> Homes</a></li></ul><dl class="dropdown-content nav"><dt class="nav-subtitle">Post a Home For Free</dt><dd><a href="/for-sale-by-owner/" title="For Sale by Owner" data-za-category="Posting" data-za-action="FSBO" class="nav-item za-track-event" data-za-label="Top Nav">For Sale by Owner</a></dd><dd><a href="/post-real-estate-listings/" title="For Sale by Agent" data-za-category="Posting" data-za-action="FSBA" class="nav-item za-track-event" data-za-label="Top Nav">For Sale by Agent</a></dd><dd><a href="/coming-soon/" title="Coming Soon" data-za-category="Posting" data-za-action="ComingSoon" class="nav-item za-track-event" data-za-label="Top Nav">Coming Soon <span class="new">NEW!</span></a></dd><dd><a href="/make-me-move/" title="Make Me Move" data-za-category="Posting" data-za-action="MMM" class="nav-item za-track-event" data-za-label="Top Nav">Make Me Move</a></dd></dl></div></li><li data-nav-level="1" class="dropdown rentals-tab top-nav-tab rollable"><a href="/homes/for_rent/" data-za-category="TopNav" class="nav-item nav-primary-item nav-item-button dropdown-trigger responsive-nav-item" data-za-label="None" id="top-nav-rentals" data-overflow-item-class="zss-responsive-nav-item" title="Rentals" data-za-action="Rentals">Rentals</a><div class="dropdown-bd"><ul class="dropdown-content nav l-stacked"><li><a href="/grand-prairie-tx-75104/rent/" data-za-category="TopNav" class="nav-item za-track-event" data-za-label="Apartments for Rent" title="75104 Apartments For Rent" data-za-action="Rentals"><span class="region-name">75104</span> Apartments For Rent</a></li><li class="menu-nav-seperator"><a href="/grand-prairie-tx-75104/rent-houses/" data-za-category="TopNav" class="nav-item za-track-event" data-za-label="Homes for Rent" title="75104 Houses For Rent" data-za-action="Rentals"><span class="region-name">75104</span> Houses For Rent</a></li><li><a href="/grand-prairie-tx/rent/" data-za-category="TopNav" class="nav-item za-track-event" data-za-label="Apartments for Rent" title="Grand Prairie Apartments For Rent" data-za-action="Rentals"><span class="region-name">Grand Prairie</span> Apartments For Rent</a></li><li><a href="/grand-prairie-tx/rent-houses/" data-za-category="TopNav" class="nav-item za-track-event" data-za-label="Homes for Rent" title="Grand Prairie Houses For Rent" data-za-action="Rentals"><span class="region-name">Grand Prairie</span> Houses For Rent</a></li><li><a href="/grand-prairie-tx/expensive-rentals/" title="Grand Prairie Most Expensive Rentals" data-za-category="TopNav" data-za-action="Homes" class="nav-item za-track-event" data-za-label="Most Expensive Rentals"><span class="region-name">Grand Prairie</span> Most Expensive Rentals</a></li></ul><dl class="dropdown-content nav"><dd><a href="/rent/guide/" title="Renter Guide" class="nav-item za-track-event" data-za-category="TopNav" data-za-action="Renter Guide" data-za-label="Renter Guide">Renter's Guide</a></dd></dl><dl class="dropdown-content nav"><dd class="dropdown-btn-ian"><a href="/post-rental-listings/?source=topnav" title="Post a Rental For Free" data-za-category="Posting" data-za-action="Rental" class="inline-button " data-za-label="Top Nav">Post a Rental For Free</a></dd></dl></div></li><li data-nav-level="2" class="dropdown mortgage-tab top-nav-tab rollable"><a href="/mortgage-rates/" id="top-nav-Mortgage" data-overflow-item-class="zss-responsive-nav-item" title="Mortgages" data-za-category="TopNav" data-za-action="Mortgage Rates" data-za-label="None" class="nav-item nav-primary-item nav-item-button dropdown-trigger responsive-nav-item">Mortgages</a><div class="dropdown-bd"><ul class="dropdown-content nav l-stacked"><li><a href="/mortgage-rates/tx/grand-prairie/" title="Grand Prairie Mortgage Rates" data-za-category="TopNav" data-za-action="Mortgage Rates" class="nav-item za-track-event" data-za-label="City Mortgage Rates"><span class="region-name">Grand Prairie</span> Mortgage Rates</a></li><li><a href="/mortgage-rates/tx/" title="Texas Mortgage Rates" data-za-category="TopNav" data-za-action="Mortgage Rates" class="nav-item za-track-event" data-za-label="Mortgage Rates"><span class="region-name">Texas</span> Mortgage Rates</a></li><li><a href="/refinance/tx/grand-prairie/" title="Grand Prairie Refinance" data-za-category="TopNav" data-za-action="Mortgage Rates" data-za-label="City Refinance" class="nav-item za-track-event"><span class="region-name">Grand Prairie</span> Refinance</a></li><li><a href="/refinance/tx/" title="Texas Refinance" data-za-category="TopNav" data-za-action="Mortgage Rates" data-za-label="Refinance" class="nav-item za-track-event"><span class="region-name">Texas</span> Refinance</a></li><li><a href="/pre-approval/" title="Get Pre-Approved" data-za-category="TopNav" data-za-action="Mortgage Rates" class="nav-item za-track-event" data-za-label="Get Pre-Approved">Get Pre-Approved</a></li></ul><dl class="dropdown-content nav"><dt class="nav-subtitle">Calculators</dt><dd><a href="/mortgage-calculator/" title="Mortgage Calculator" data-za-category="TopNav" data-za-action="Mortgage Rates" class="nav-item za-track-event" data-za-label="Mortgage Calculator">Mortgage Calculator</a></dd><dd><a href="/mortgage-calculator/refinance-calculator/" title="Refinance Calculator" data-za-category="TopNav" data-za-action="Mortgage Rates" class="nav-item za-track-event" data-za-label="Refinance Calculator">Refinance Calculator</a></dd><dd><a href="/mortgage-calculator/house-affordability/" title="Affordability Calculator" data-za-category="TopNav" data-za-action="Mortgage Rates" class="nav-item za-track-event" data-za-label="Affordability Calculator">Affordability Calculator</a></dd><dd><a href="/mortgage-rates/calc-list/" class="nav-item" title="See All">See All</a></dd></dl><dl class="dropdown-content nav"><dt class="nav-subtitle">Resources</dt><dd><a href="/mortgage-rates/learn/" title="Mortgage Education Center" data-za-category="TopNav" data-za-action="Mortgage Rates" class="nav-item za-track-event" data-za-label="Mortgage Education Center">Mortgage Education Center</a></dd><dd><a href="/mortgage-calculator/harp-eligibility/" title="HARP Program" data-za-category="TopNav" data-za-action="HARP Program" class="nav-item za-track-event" data-za-label="HARP Program">HARP Program</a></dd><dd><a href="/mortgage-rates/finding-the-right-loan/fha-loan/" title="FHA Loan" data-za-category="TopNav" data-za-action="Mortgage Rates" class="nav-item za-track-event" data-za-label="FHA Loan">FHA Loan</a></dd><dd><a href="/mortgage-rates/finding-the-right-loan/home-equity-mortgage/" title="Home Equity Loan" data-za-category="TopNav" data-za-action="Mortgage Rates" class="nav-item za-track-event" data-za-label="Home Equity Loan">Home Equity Loan</a></dd></dl></div></li><li data-nav-level="4" class="dropdown directory-tab top-nav-tab rollable"><a href="http://www.zillow.com/directory/Grand-Prairie-TX/real-estate-agents/" title="Find a Pro" data-za-category="TopNav" data-za-action="Professionals" class="nav-item nav-primary-item nav-item-button dropdown-trigger responsive-nav-item" data-za-label="None">Agents</a><div class="dropdown-bd"><dl class="dropdown-content nav"><dt class="nav-subtitle">Find Trusted Pros</dt><dd><a href="http://www.zillow.com/directory/Grand-Prairie-TX/real-estate-agents/" title="Grand Prairie Real Estate Agents" data-ga-label="Realtors" class="nav-item"><span class="region-name">Grand Prairie</span> Real Estate Agents </a></dd><dd><a href="http://www.zillow.com/directory/Grand-Prairie-TX/mortgage-lenders/" title="Grand Prairie Mortgage Lenders" data-ga-category="TopNav" data-ga-label="Mortgage Lenders" class="nav-item" data-ga-action="Professionals"><span class="region-name">Grand Prairie</span> Mortgage Lenders </a></dd><dd><a href="http://www.zillow.com/directory/Grand-Prairie-TX/property-management/" title="Grand Prairie Property Managers" data-ga-category="TopNav" data-ga-label="Property Management Pros" class="nav-item" data-ga-action="Professionals"><span class="region-name">Grand Prairie</span> Property Managers </a></dd><dd><a href="http://www.zillow.com/directory/Grand-Prairie-TX/home-improvement/" title="Grand Prairie Home Improvement Pros" data-ga-category="TopNav" data-ga-label="Home Improvement Pros" class="nav-item" data-ga-action="Professionals"><span class="region-name">Grand Prairie</span> Home Improvement Pros </a></dd><dd><a href="/homedetail/HomeDetail,$PageWrapper.topNav.$nav$FindRealEstateProfessionals_1.$links$review$AnonymousReviewDirectLink.sdirect" class="nav-item" data-ga-label="Review an Agent, Lender or Pro" title="Review an Agent, Lender or Pro" data-ga-action="Professionals" data-ga-category="TopNav"> Review an Agent, Lender or Pro </a></dd></dl><dl class="dropdown-content nav"><dd></dd><dt class="nav-subtitle">For Agents</dt><dd><a href="/feedback/AdvertiserSignup.htm?source=TopNav&amp;action=Find%20A%20Pro&amp;label=Real%20Estate%20Advertising" class="nav-item za-track-event" title="Real Estate Agent Advertising" data-za-category="TopNav" data-za-action="Find a Pro" data-za-label="Real Estate Advertising"> Real Estate Agent Advertising </a></dd><dd><a href="/homedetail/HomeDetail,$PageWrapper.topNav.$nav$MarketingOnZillow_0.$links$directory$JoinDirectoryLink.sdirect?form:homedetail/HomeDetail=ZH4sIAAAAAAAAALWTv0sDMRTH3xUFfxXUzoLD4RgVVARdLA5Walt0EFwk3j2v0WuSJrle20F00cFVtw4Ojv4rju7irrgJTt5ZW7F0qEUzBPLey%2Fu%2B93nJ3TMMhggAYxbk7QL1cFdRKVERI2SOVojNacXeouoYDeNenu8x3xfh%2FhyxfcaPte0yhY4RqmZvCsbXW6ds5IRklEczbdDdEcrIcNGCmaLrEFsjVU7RLigRRZjaNsrIvxMcaEcxaZjgMCK%2FfBlXK5g4ohVKfMo9kuEGPVSTTze3b2cXywmwMjBYoX6AVQXj33G5oHSA6vzuemr06vEyAVCVVuP1NPT%2Boc9Uu884cVNZhjkLlroq9ZswCR1Y5h1RIvXPQokrSpRx0vITHcgYKklTjYX2JWguK8KWhYG6ZK6BVDamNhtTm%2F2iu1KVugwnUWCMbKH3sQ03Q2Lbj6mlhfCR8vtpdfrQeH%2BJ5PdaU5NWuNy7QLIoSti2xzUO%2Faq%2BQ4XlALlTa%2FYXswhXe78%2F0eK75ke7bpew0f%2BsOz%2FJvgVbvT%2FQfhTwzxW6vtcPcSSTk1sEAAA%3D#joinDirectory" title="Join the Professional Directory" data-za-category="TopNav" data-za-action="Agents" class="nav-item za-track-event" data-za-label="Join Agent Directory">Join the Agent Directory</a></dd><dd><a href="/agents/" title="Access Agent Hub" data-za-category="TopNav" data-za-action="For Agents" class="nav-item za-track-event" data-za-label="Agent Hub">Access Agent Hub</a></dd></dl><dl class="dropdown-content nav"><dd></dd><dt class="nav-subtitle">For Rental Pros</dt><dd><a href="http://rentalpro.zillow.com/" class="nav-item za-track-event" title="Real Estate Advertising" rel="nofollow" data-za-category="TopNav" data-za-action="Agents" data-za-label="Rental Advertising"> Real Estate Advertising </a></dd></dl><dl class="dropdown-content nav"><dd></dd><dt class="nav-subtitle">For Pros</dt><dd><a href="/homedetail/HomeDetail,$PageWrapper.topNav.$links$directory$JoinDirectoryLink.sdirect?form:homedetail/HomeDetail=ZH4sIAAAAAAAAALWTv0sDMRTH3xUFfxXUzoLD4RgVVARdLA5Walt0EFwk3j2v0WuSJrle20F00cFVtw4Ojv4rju7irrgJTt5ZW7F0qEUzBPLey%2Fu%2B93nJ3TMMhggAYxbk7QL1cFdRKVERI2SOVojNacXeouoYDeNenu8x3xfh%2FhyxfcaPte0yhY4RqmZvCsbXW6ds5IRklEczbdDdEcrIcNGCmaLrEFsjVU7RLigRRZjaNsrIvxMcaEcxaZjgMCK%2FfBlXK5g4ohVKfMo9kuEGPVSTTze3b2cXywmwMjBYoX6AVQXj33G5oHSA6vzuemr06vEyAVCVVuP1NPT%2Boc9Uu884cVNZhjkLlroq9ZswCR1Y5h1RIvXPQokrSpRx0vITHcgYKklTjYX2JWguK8KWhYG6ZK6BVDamNhtTm%2F2iu1KVugwnUWCMbKH3sQ03Q2Lbj6mlhfCR8vtpdfrQeH%2BJ5PdaU5NWuNy7QLIoSti2xzUO%2Faq%2BQ4XlALlTa%2FYXswhXe78%2F0eK75ke7bpew0f%2BsOz%2FJvgVbvT%2FQfhTwzxW6vtcPcSSTk1sEAAA%3D#joinDirectory" title="Join the Professional Directory" data-za-category="TopNav" data-za-action="Professionals" class="nav-item" data-za-label="Join the Professional Directory"> Join the Professional Directory </a></dd></dl></div></li><li data-nav-level="3" class="dropdown advice-tab top-nav-tab rollable"><a href="http://www.zillow.com/advice/" data-overflow-item-class="more-advice" title="Advice" data-za-category="TopNav" data-za-action="Advice" data-za-label="None" class="nav-item nav-primary-item nav-item-button dropdown-trigger responsive-nav-item">Advice</a><div class="dropdown-bd"><dl class="dropdown-content nav"><dt class="nav-subtitle">Join Our Discussions</dt><dd><a href="http://www.zillow.com/advice/" title="Real Estate Forum" data-za-category="TopNav" data-za-action="Advice" class="nav-item za-track-event" data-za-label="Real Estate Forum">Real Estate Forum</a></dd><dd><a href="http://www.zillow.com/advice/US/mortgage/question-discussion-guide/" title="Mortgage Forum" data-za-category="TopNav" data-za-action="Advice" data-za-label="Mortgage Forum" class="nav-item za-track-event">Mortgage Forum</a></dd><dd><a href="/advice/United-States%253B%253B%253B%253B%253B%253B%253B%253B/zillow-questions/discussion/" title="Using Zillow Forum" data-za-category="TopNav" data-za-action="Advice" class="nav-item za-track-event" data-za-label="Using Zillow Forum">Using Zillow Forum</a></dd><dd><a href="/advicepages/AskAdvice.htm" title="Ask a Question" data-za-category="TopNav" data-za-action="Advice" class="nav-item za-track-event" data-za-label="Ask a Question">Ask a Question</a></dd></dl><dl class="dropdown-content nav"><dt class="nav-subtitle">Education Guides</dt><dd><a href="/foreclosures/" title="Foreclosure Center" data-za-category="TopNav" data-za-action="Advice" class="nav-item za-track-event" data-za-label="Foreclosure Center">Foreclosure Center</a></dd><dd><a href="/mortgage-rates/learn/" title="Mortgage Education Center" data-za-category="TopNav" data-za-action="Advice" class="nav-item za-track-event" data-za-label="Mortgage Education Center">Mortgage Education Center</a></dd><dd><a href="/home-buying-guide/" title="Buyers Guide" data-za-category="TopNav" data-za-action="Advice" class="nav-item za-track-event" data-za-label="Buyers Guide">Buyer's Guide</a></dd><dd><a href="/help/" title="Zillow Help Center" data-za-category="TopNav" data-za-action="Advice" class="nav-item za-track-event" data-za-label="Zillow Help Center">Zillow Help Center</a></dd></dl></div></li><li data-nav-level="1" class="dropdown local-tab top-nav-tab rollable"><a href="/grand-prairie-tx/home-values/" data-overflow-item-class="local-link" title="Local Info" data-ga-category="TopNav" data-ga-label="None" class="responsive-nav-item nav-primary-item nav-item-button nav-item dropdown-trigger" data-ga-action="Local Info">Local</a><div class="dropdown-bd"><ul class="dropdown-content nav l-stacked"><li><a href="http://www.zillow.com/local-info/TX-Grand-Prairie/r_31821/" title="Grand Prairie Overview" data-za-category="TopNav" data-za-action="Local Info" class="nav-item za-track-event" data-za-label="Overview"><span class="region-name">Grand Prairie</span>&nbsp;Overview </a></li><li><a href="/grand-prairie-tx/schools/" title="Grand Prairie Schools" data-za-category="TopNav" data-za-action="Local Info" class="nav-item za-track-event" data-za-label="Schools"><span class="region-name">Grand Prairie</span>&nbsp;Schools </a></li><li><a href="/grand-prairie-tx/home-values/" title="Grand Prairie Home Values" data-za-category="TopNav" data-za-action="Local Info" class="nav-item za-track-event" data-za-label="City Home Values"><span class="region-name">Grand Prairie</span>&nbsp;Home Values </a></li></ul><dl class="dropdown-content nav"><dt class="nav-subtitle">Popular</dt><dd><a href="http://www.zillow.com/local-info/" title="Real Estate Market Reports" data-za-category="TopNav" data-za-action="Local Info" class="nav-item za-track-event" data-za-label="Real Estate Market Reports">Real Estate Market Reports</a></dd><dd><a href="/compare/" title="Compare Places" data-za-category="TopNav" data-za-action="Local Info" class="nav-item za-track-event" data-za-label="Compare Places">Compare Places</a></dd></dl></div></li><li data-nav-level="6" class="dropdown digs-tab top-nav-tab rollable"><a href="/digs/" data-overflow-item-class="zss-mobile-nav-item" title="Home Design" data-za-category="TopNav" data-za-action="Digs" class="nav-item nav-primary-item nav-item-button dropdown-trigger responsive-nav-item" data-za-label="None">Home Design</a><div class="dropdown-bd"><ul class="dropdown-content nav l-stacked"><li><a href="/digs/bathrooms/" title="Bathroom Ideas" data-za-category="TopNav" data-za-action="Digs" class="nav-item za-track-event" data-za-label="Bathrooms">Bathrooms</a></li><li><a href="/digs/bedrooms/" title="Bedroom Ideas" data-za-category="TopNav" data-za-action="Digs" class="nav-item za-track-event" data-za-label="Bedrooms">Bedrooms</a></li><li><a href="/digs/gardens/" title="Garden Ideas" data-za-category="TopNav" data-za-action="Digs" class="nav-item za-track-event" data-za-label="Gardens">Gardens</a></li><li><a href="/digs/kitchens/" title="Kitchen Ideas" data-za-category="TopNav" data-za-action="Digs" class="nav-item za-track-event" data-za-label="Kitchens">Kitchens</a></li><li><a href="/digs/living-rooms/" title="Living Room Ideas" data-za-category="TopNav" data-za-action="Digs" class="nav-item za-track-event" data-za-label="Living Rooms">Living Rooms</a></li></ul></div></li><li data-nav-level="0" class="dropdown more-tab top-nav-tab rollable"><span class="nav-item nav-primary-item nav-item-button dropdown-trigger dropdown-trigger-ico" title="More" href="#"> More <span class="dropdown-arrow"></span></span><div class="dropdown-bd"><dl class="dropdown-content nav more-dropdown"><dt class="nav-subtitle">Guides</dt><dd><a href="/foreclosures/" title="Foreclosure Center" data-za-category="TopNav" data-za-action="Advice" class="nav-item za-track-event" data-za-label="Foreclosure Center">Foreclosure Center</a></dd><dd><a href="/mortgage-rates/learn/" title="Mortgage Education Center" data-za-category="TopNav" data-za-action="Advice" class="nav-item za-track-event" data-za-label="Mortgage Education Center">Mortgage Education Center</a></dd><dd><a href="/home-buying-guide/" title="Buyers Guide" data-za-category="TopNav" data-za-action="Advice" class="nav-item za-track-event" data-za-label="Buyers Guide">Buyer's Guide</a></dd><dd><a href="/rent/guide/" title="Renter Guide" class="nav-item za-track-event" data-za-category="TopNav" data-za-action="Renter Guide" data-za-label="Renter Guide">Renter's Guide</a></dd></dl><dl class="dropdown-content nav more-dropdown"><dt class="nav-subtitle"><strong>Blogs</strong></dt><dd><a href="http://www.zillow.com/blog/" class="nav-item za-track-event" title="Zillow Blog" data-za-category="TopNav" data-za-action="Blog" data-za-label="Zillow Blog">Zillow Blog</a></dd><dd><a href="http://www.zillow.com/blog/pro/" class="nav-item za-track-event" title="Zillow Pros Blog" data-za-category="TopNav" data-za-action="Blog" data-za-label="Zillow Pros Blog">Zillow Pros Blog</a></dd><dd><a href="http://engineering.zillow.com/" class="nav-item za-track-event" title="Zillow Engineering" data-za-category="TopNav" data-za-action="Blog" data-za-label="Zillow Engineering">Zillow Engineering</a></dd></dl><dl class="dropdown-content nav more-dropdown"><dt class="nav-subtitle"><strong>More</strong></dt><dd><a href="/advertising/" title="Real Estate Advertising" data-za-category="TopNav" data-za-action="More" class="nav-item za-track-event" data-za-label="Real Estate Advertising">Real Estate Advertising</a></dd><dd><a href="/advertising/solutions/" title="Real Estate Advertising" data-za-category="TopNav" data-za-action="More" class="nav-item za-track-event" data-za-label="Real Estate Advertising">Advertising Solutions</a></dd><dd><a href="/visuals/" class="nav-item za-track-event" title="Zillow Visuals">Zillow Visuals</a></dd><dd><a href="http://www.zillow.com/research/" class="nav-item za-track-event" title="Zillow Research" data-za-category="TopNav" data-za-action="Blog" data-za-label="Zillow Research">Zillow Research</a></dd><dd><a href="/tv/" class="nav-item za-track-event" title="Zillow TV Spots">Zillow TV Spots</a></dd><dd><a href="/webtools/" title="Widgets, Badges and Data" data-za-category="TopNav" data-za-action="More" class="nav-item za-track-event" data-za-label="Widgets, Badges-Data">Widgets, Badges &amp; Data</a></dd><dd><a href="/mobile/" title="Mobile" data-za-category="TopNav" data-za-action="More" class="nav-item za-track-event" data-za-label="What Zillow Offers">Zillow Mobile Apps</a></dd><dd><a href="/help/" title="Help" data-za-category="TopNav" data-za-action="More" class="nav-item za-track-event" data-za-label="Widgets, Badges-Data">Help</a></dd></dl></div></li></ul></div><div id="login-block" class=" login-block nav-animation-text"><ul class="l-inline zss-nav dropdown-hoverable"><li data-nav-level="7" class=" top-nav-tab dropdown myzillow-tab agenthub-tab"><span class="nav-item nav-primary-item nav-item-button dropdown-trigger dropdown-trigger-ico"><a href="/agents/" title="For Agents" data-za-category="TopNav" data-za-action="For Agents" class="top-nav-advice za-track-event" data-za-label="Agents">Advertise</a></span></li><!-- not logged in --><li class="auth-menu"><div class="nav-long-text"><a href="/homedetail/HomeDetail,$PageWrapper.$links$LoginLink.sdirect?form:homedetail/HomeDetail=ZH4sIAAAAAAAAALWTv0sDMRTH3xUFfxXUzoLD4RgVVARdLA5Walt0EFwk3j2v0WuSJrle20F00cFVtw4Ojv4rju7irrgJTt5ZW7F0qEUzBPLey%2Fu%2B93nJ3TMMhggAYxbk7QL1cFdRKVERI2SOVojNacXeouoYDeNenu8x3xfh%2FhyxfcaPte0yhY4RqmZvCsbXW6ds5IRklEczbdDdEcrIcNGCmaLrEFsjVU7RLigRRZjaNsrIvxMcaEcxaZjgMCK%2FfBlXK5g4ohVKfMo9kuEGPVSTTze3b2cXywmwMjBYoX6AVQXj33G5oHSA6vzuemr06vEyAVCVVuP1NPT%2Boc9Uu884cVNZhjkLlroq9ZswCR1Y5h1RIvXPQokrSpRx0vITHcgYKklTjYX2JWguK8KWhYG6ZK6BVDamNhtTm%2F2iu1KVugwnUWCMbKH3sQ03Q2Lbj6mlhfCR8vtpdfrQeH%2BJ5PdaU5NWuNy7QLIoSti2xzUO%2Faq%2BQ4XlALlTa%2FYXswhXe78%2F0eK75ke7bpew0f%2BsOz%2FJvgVbvT%2FQfhTwzxW6vtcPcSSTk1sEAAA%3D" class="nav-item nav-primary-item nav-item-button tengage-reg-nav nav-text nav-animation show-lightbox auth-required login" id="login_opener" title="Sign In" data-ga-label="TopNav" rel="nofollow">Sign In</a><a id="nav-text-conjecture" class="nav-item nav-primary-item nav-item-button tengage-reg-nav nav-text">or</a><a href="/homedetail/HomeDetail,$PageWrapper.$links$RegisterLink.sdirect?form:homedetail/HomeDetail=ZH4sIAAAAAAAAALWTv0sDMRTH3xUFfxXUzoLD4RgVVARdLA5Walt0EFwk3j2v0WuSJrle20F00cFVtw4Ojv4rju7irrgJTt5ZW7F0qEUzBPLey%2Fu%2B93nJ3TMMhggAYxbk7QL1cFdRKVERI2SOVojNacXeouoYDeNenu8x3xfh%2FhyxfcaPte0yhY4RqmZvCsbXW6ds5IRklEczbdDdEcrIcNGCmaLrEFsjVU7RLigRRZjaNsrIvxMcaEcxaZjgMCK%2FfBlXK5g4ohVKfMo9kuEGPVSTTze3b2cXywmwMjBYoX6AVQXj33G5oHSA6vzuemr06vEyAVCVVuP1NPT%2Boc9Uu884cVNZhjkLlroq9ZswCR1Y5h1RIvXPQokrSpRx0vITHcgYKklTjYX2JWguK8KWhYG6ZK6BVDamNhtTm%2F2iu1KVugwnUWCMbKH3sQ03Q2Lbj6mlhfCR8vtpdfrQeH%2BJ5PdaU5NWuNy7QLIoSti2xzUO%2Faq%2BQ4XlALlTa%2FYXswhXe78%2F0eK75ke7bpew0f%2BsOz%2FJvgVbvT%2FQfhTwzxW6vtcPcSSTk1sEAAA%3D" class="nav-item nav-primary-item nav-item-button tengage-reg-nav nav-text nav-animation show-lightbox auth-required register" id="register_opener" title="Join" data-ga-label="TopNav" rel="nofollow">Join</a></div><div class="auth-nav-animation-block" style="opacity: 1; transition: opacity 500ms ease-in 0ms; -webkit-transition: opacity 500ms ease-in 0ms;"><div class="auth-nav-animation-beak">&nbsp;</div><div class="auth-nav-animation"><span><a tabindex="-1" title="Close" class="zbt icon close">x</a></span><span class="auth-button"><a href="/homedetail/HomeDetail,$PageWrapper.$links$LoginLink_0.sdirect?form:homedetail/HomeDetail=ZH4sIAAAAAAAAALWTv0sDMRTH3xUFfxXUzoLD4RgVVARdLA5Walt0EFwk3j2v0WuSJrle20F00cFVtw4Ojv4rju7irrgJTt5ZW7F0qEUzBPLey%2Fu%2B93nJ3TMMhggAYxbk7QL1cFdRKVERI2SOVojNacXeouoYDeNenu8x3xfh%2FhyxfcaPte0yhY4RqmZvCsbXW6ds5IRklEczbdDdEcrIcNGCmaLrEFsjVU7RLigRRZjaNsrIvxMcaEcxaZjgMCK%2FfBlXK5g4ohVKfMo9kuEGPVSTTze3b2cXywmwMjBYoX6AVQXj33G5oHSA6vzuemr06vEyAVCVVuP1NPT%2Boc9Uu884cVNZhjkLlroq9ZswCR1Y5h1RIvXPQokrSpRx0vITHcgYKklTjYX2JWguK8KWhYG6ZK6BVDamNhtTm%2F2iu1KVugwnUWCMbKH3sQ03Q2Lbj6mlhfCR8vtpdfrQeH%2BJ5PdaU5NWuNy7QLIoSti2xzUO%2Faq%2BQ4XlALlTa%2FYXswhXe78%2F0eK75ke7bpew0f%2BsOz%2FJvgVbvT%2FQfhTwzxW6vtcPcSSTk1sEAAA%3D" class="nav-item nav-primary-item nav-item-button tengage-reg-nav nav-text nav-animation show-lightbox auth-required login" id="login_opener_0" title="Sign In" data-ga-label="Top nav pop up" rel="nofollow">Sign In</a></span><p class="animation-block-text"><span>Need an account?&nbsp;</span><span><a href="/homedetail/HomeDetail,$PageWrapper.$links$RegisterLink_0.sdirect?form:homedetail/HomeDetail=ZH4sIAAAAAAAAALWTv0sDMRTH3xUFfxXUzoLD4RgVVARdLA5Walt0EFwk3j2v0WuSJrle20F00cFVtw4Ojv4rju7irrgJTt5ZW7F0qEUzBPLey%2Fu%2B93nJ3TMMhggAYxbk7QL1cFdRKVERI2SOVojNacXeouoYDeNenu8x3xfh%2FhyxfcaPte0yhY4RqmZvCsbXW6ds5IRklEczbdDdEcrIcNGCmaLrEFsjVU7RLigRRZjaNsrIvxMcaEcxaZjgMCK%2FfBlXK5g4ohVKfMo9kuEGPVSTTze3b2cXywmwMjBYoX6AVQXj33G5oHSA6vzuemr06vEyAVCVVuP1NPT%2Boc9Uu884cVNZhjkLlroq9ZswCR1Y5h1RIvXPQokrSpRx0vITHcgYKklTjYX2JWguK8KWhYG6ZK6BVDamNhtTm%2F2iu1KVugwnUWCMbKH3sQ03Q2Lbj6mlhfCR8vtpdfrQeH%2BJ5PdaU5NWuNy7QLIoSti2xzUO%2Faq%2BQ4XlALlTa%2FYXswhXe78%2F0eK75ke7bpew0f%2BsOz%2FJvgVbvT%2FQfhTwzxW6vtcPcSSTk1sEAAA%3D" class="nav-item nav-primary-item tengage-reg-nav nav-text nav-animation show-lightbox auth-required register" id="register_opener_0" title="Sign up" data-ga-label="Top nav pop up" rel="nofollow">Sign up here</a></span></p></div></div></li><li class="help-menu"><div class="nav-long-text zillow-help-icon"><a href="/help/" title="Zillow Help Center" data-ga-category="TopNav" data-ga-label="Help Icon" class="top-nav-advice za-track-event" data-ga-action="Help">?</a></div></li></ul></div></div></div></nav></header><div id="search-anchor" class=" header-search search search-wrapper"><div class="hdp-sticky-bar clearfix"><div id="actionBar" class="prop-mod prop-action-bar action-bar no-js hdp click-open"><ul class="l-inline"><li id="contact-menu" class="menu-item contact"><div class="menu-label noicon contact-label"><div id="contact-lightbox_src" class="template hide">&nbsp;<!--<div id="contact\-lightbox_content" class="ajax\-form dialog clearfix"><a class="close lightbox\-close" title="Close" tabindex="\-1">&times;</a><div class="yui3\-widget\-bd lightbox\-body clearfix"><div id="contact\-lightbox\-confirm" class="lightbox\-block current"><div class="generic\-box yui3\-widget\-stdmod module"><div class="module\-body yui3\-widget\-bd clearfix"><div class="bal\-refactor   featured">
    <h2 class="prop\-mod\-hd">Get more information</h2>

    * Snippet 2 - images:

<div id="container" class="b-c-layout has-breadcrumb"><div class="active-view" id="yui_3_15_0_1_1407415446921_422"><div id="breadcrumb-wrap" class="clearfix"><div id="gbc-area" class="clearfix"><ul id="gbc" class="no-js notranslate"><li class="page-views"> Views: 7,179</li><li id="region-state" class="gbc-parent state bca shallow" typeof="v:Breadcrumb"><a href="/tx/" data-ga-category="Homes" data-ga-label="State" class="gbc-top track-ga-event" property="v:title" rel="v:url" data-ga-action="Breadcrumb click">Texas</a></li><li id="region-city" class="gbc-parent city bca shallow" typeof="v:Breadcrumb"><a href="/grand-prairie-tx/" data-ga-category="Homes" data-ga-label="City" class="gbc-top track-ga-event" property="v:title" rel="v:url" data-ga-action="Breadcrumb click">Grand Prairie</a></li><li id="region-zipcode" class="gbc-parent zipcode bca shallow" typeof="v:Breadcrumb"><a href="/grand-prairie-tx-75104/" data-ga-category="Homes" data-ga-label="Zipcode" class="gbc-top track-ga-event" property="v:title" rel="v:url" data-ga-action="Breadcrumb click">75104 </a></li><li class="shallow current"><span class="gbc-top">911 Mallard Pointe Dr</span></li></ul></div></div><a name="contentstart" accesskey="2" href="#contentstart"></a><div id="content" class="zss-content yui3-u"><div data-zmm-page="HDP" id="yui_3_15_0_1_1407415446921_421"><div id="hdp-content" class="prop photos-swap"><div id="map-tabs" class="hdp-tabbed-header prop-map-tabs has-photos  tabs-z yui3-widget yui3-tabview yui3-tabcontentmgr yui3-tabcontentmgr-content"><div class="yui3-tabview-panel" id="yui_3_15_0_1_1407415446921_420"><div id="googlestreet" class="yui3-tab-panel" role="tabpanel" aria-labelledby="yui_3_15_0_1_1407415446921_83"><div id="googlestreet-view-container"><div id="google-street-view"></div></div></div><div id="birdseye" class="yui3-tab-panel" role="tabpanel" aria-labelledby="yui_3_15_0_1_1407415446921_102"><div id="hdp-map-view-birdseye" class="hdp-map-view"><div class="hdp-map-view-map"></div><div class="rotate-container-large hide"><a class="rotate-left-large rotate-major-large" href="#"></a><a class="rotate-right-large rotate-major-large" href="#"></a></div><div class="zoom-container-large"><a class="zoom-in-large zoom-major-large" href="#"></a><a class="zoom-out-large zoom-major-large" href="#"></a></div></div></div><div id="map" class="yui3-tab-panel" role="tabpanel" aria-labelledby="yui_3_15_0_1_1407415446921_121"><div id="hdp-map-view" class="hdp-map-view"><div class="hdp-map-view-map"></div><div class="rotate-container-large hide"><a class="rotate-left-large rotate-major-large" href="#"></a><a class="rotate-right-large rotate-major-large" href="#"></a></div><div class="zoom-container-large"><a class="zoom-in-large zoom-major-large" href="#"></a><a class="zoom-out-large zoom-major-large" href="#"></a></div><a href="/homes/27062400_zpid/" class="view-larger map-overlay za-track-event" data-za-category="Homes" data-za-action="View larger map">View larger map</a></div></div><div id="photos" class="yui3-tab-panel yui3-tab-panel-selected" role="tabpanel" aria-labelledby="hip-content-thumbs"><div id="photo-view-container"><div id="home-image-preview" class="clearfix carousel"><div id="hip-content" class="carousel thumb_target left-aligned"><span id="save-and-pin-block" class="save-and-pin middle hide"><div class="save-and-pin-wrapper"><a href="#hdp-photo-lightbox" class="show-lightbox za-track-event" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" data-za-href="!ignore"><span class="pintest-item larger">View Larger </span></a></div></span><ol class="photos"><li class="current"><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAz3ecvv5uw8c21000000000_hli8s" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" src="http://photos2.zillowstatic.com/p_h/IShjyqwv5uw8c21000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAbt5wgyg514d01000000000_lhz40" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" src="http://photos2.zillowstatic.com/p_h/ISt8qaiyg514d01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IA3n3kyrcxs6d01000000000_ljdoh" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos3.zillowstatic.com/p_h/ISl2oyzrcxs6d01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IA3bap5mscveq21000000000_oryvo" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos3.zillowstatic.com/p_h/ISlqu37mscveq21000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAvg18gl8pk9d01000000000_lks8y" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos1.zillowstatic.com/p_h/ISdwlmhl8pk9d01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAnazvxe4hccd01000000000_lm6tf" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos2.zillowstatic.com/p_h/IS5qjaze4hccd01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAv48dnfo4nhq21000000000_otdg5" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos1.zillowstatic.com/p_h/ISdksrofo4nhq21000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAf4xjf8094fd01000000000_lnldw" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos3.zillowstatic.com/p_h/ISxjhyg8094fd01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IA7yu7x1w0whd01000000000_lozyd" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos1.zillowstatic.com/p_h/ISpdfmy1w0whd01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAny5159kwekq21000000000_ous0m" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos2.zillowstatic.com/p_h/IS5eqf69kwekq21000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAzrsvevrsnkd01000000000_lqeiu" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos2.zillowstatic.com/p_h/ISh7dagvrsnkd01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAfs3pm2go6nq21000000000_ow6l3" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos3.zillowstatic.com/p_h/ISx7o3o2go6nq21000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IArlqjwonkfnd01000000000_lrt3b" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos3.zillowstatic.com/p_h/IS91byxonkfnd01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAjfo7eijc7qd01000000000_lt7ns" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos1.zillowstatic.com/p_h/IS1v8mfijc7qd01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IA7m1d4wbgypq21000000000_oxl5k" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos1.zillowstatic.com/p_h/ISp1mr5wbgypq21000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAb9mvvbf4zsd01000000000_lum89" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos2.zillowstatic.com/p_h/ISto6axbf4zsd01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAzfz0mp78qsq21000000000_oyzq1" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos2.zillowstatic.com/p_h/IShvjfnp78qsq21000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IA33kjd5bwqvd01000000000_lw0sq" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos3.zillowstatic.com/p_h/ISli4ye5bwqvd01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAvwh7vy6oiyd01000000000_lxfd7" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos1.zillowstatic.com/p_h/ISdc2mwy6oiyd01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAr9xo3j30ivq21000000000_p0eai" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos3.zillowstatic.com/p_h/IS9ph35j30ivq21000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAnqfvcs2ga1e01000000000_lytxo" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos2.zillowstatic.com/p_h/IS560aes2ga1e01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAfkdjuly724e01000000000_m08i5" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos3.zillowstatic.com/p_h/ISxzxxvly724e01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAj3vclczr9yq21000000000_p1suz" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos1.zillowstatic.com/p_h/IS1jfrmczr9yq21000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IA7eb7cfuzt6e01000000000_m1n2m" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos1.zillowstatic.com/p_h/ISptvldfuzt6e01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li><li><a title="View larger photos" data-za-category="Homes" data-za-action="Photo Lighbtox - Open" class="show-lightbox za-track-event" href="#hdp-photo-lightbox" data-za-href="!ignore"><img id="X1-IAz79vt8qrl9e01000000000_m31n3" alt="911 Mallard Pointe Dr, Cedar Hill, TX 75104" class="hip-photo" href="http://photos2.zillowstatic.com/p_h/IShnt9v8qrl9e01000000000.jpg"><span class="carousel-open-button view-larger-btn map-overlay">View larger</span></a></li></ol><div class="pagination prev hide"><span class="arrow">‹</span></div><div class="pagination next" id="yui_3_15_0_1_1407415446921_419"><span class="arrow" id="yui_3_15_0_1_1407415446921_418">›</span></div></div><div id="hdp-photo-lightbox_src" class="template hide">&nbsp;<!--<div id="hdp\-photo\-lightbox_content" class="dialog clearfix"><a class="close lightbox\-close" title="Close" tabindex="\-1">&times;</a><div class="yui3\-widget\-bd lightbox\-body clearfix"><div id="hdp\-photo\-lightbox\-photoscom" class="lightbox\-block current"></div></div></div>--></div></div><!-- end home-image-preview --></div></div></div><ul class="tab-container yui3-tabview-list tab-count-4" role="tablist"><li id="googlestreet-tab-li" class="yui3-tab yui3-widget yui3-customtab" data-tab-select="none" role="presentation"><div class="yui3-tab-label yui3-customtab-content" id="yui_3_15_0_1_1407415446921_83" role="tab" tabindex="0"><span class="tab-hit">Street View<span class="t-outer-border"><span class="t-inner-border"></span></span></span></div></li><li id="birdseye-tab-li" class="yui3-tab yui3-widget yui3-customtab" data-tab-select="none" role="presentation"><div class="yui3-tab-label yui3-customtab-content" id="yui_3_15_0_1_1407415446921_102" role="tab" tabindex="-1"><span class="tab-hit">Bird's Eye<span class="t-outer-border"><span class="t-inner-border"></span></span></span></div></li><li id="map-tab-li" class="yui3-tab yui3-widget yui3-customtab" data-tab-select="none" role="presentation"><div class="yui3-tab-label yui3-customtab-content" id="yui_3_15_0_1_1407415446921_121" role="tab" tabindex="-1"><span class="tab-hit">Map View<span class="t-outer-border"><span class="t-inner-border"></span></span></span></div></li><li id="photos-tab-li" class="yui3-tab yui3-widget yui3-customtab yui3-customtab-selected" data-tab-select="selected" role="presentation"><div id="hip-content-thumbs" class="carousel thumb_source yui3-tab-label yui3-customtab-content" role="tab" tabindex="-1"><div class="thumb-nav-container clearfix"><div class="thumb-nav"><ol class="photos clearfix"><li class="current" data-thumbindex="0"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="1"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="2"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="3"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="4"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="5"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="6"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="7"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="8"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="9"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="10"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="11"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="12"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="13"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="14"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="15"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="16"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="17"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="18"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="19"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="20"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="21"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="22"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="23"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li><li class="" data-thumbindex="24"><span class="thumb-select"><span class="thumb-select-inner"></span></span></li></ol></div></div><span class="curtain-left hide"></span><span class="curtain-right"></span></div></li></ul></div>

    * Snippet 3 - financing info:

        <div class="finance">
        <div class="secondary-val prop-value-zestimate"><span class="label">Est. Mortgage:</span><div data-fixed-30-rate="4.033" data-property-tax-assessed-value="229030" class=" value clearfix loan-calculator-loading" data-fixed-15-rate="3.104" data-property-zipcode="75104" data-property-tax-rate="0.0245" id="loan-calculator-container" data-property-city="Cedar Hill" data-arm-5-rate="2.804" data-property-value="323000" data-property-state="TX"><div id="monthly-payment-options" class="tooltip hide"><h3>Estimated Monthly Payment</h3><div class="hlc-inputs zss-form"><div class="zss-form-field"><label for="hlc-input-price">Price</label><input id="hlc-input-price" class="zss-form-size-s" type="text"><p class="zss-form-error-text">Please enter a dollar amount between $0 and $200,000,000</p></div><div class="zss-form-field"><label for="hlc-input-down-payment-percent">Down payment</label><div class="hlc-input-down-payment-percent-container"><input id="hlc-input-down-payment-percent" class="zss-form-size-s" type="text"></div><span class="de-emph">(<span class="hlc-output-down-payment"></span>)</span><p class="zss-form-error-text">Please enter a percent between 0% and 100%</p></div></div><div class="hlc-outputs"><div class="hlc-output"><div class="hlc-output-label">30 Year Fixed</div><div class="hlc-output-rate-container"><span class="hlc-output-rate hlc-output-fixed30"></span>/mo</div></div><div class="hlc-output"><div class="hlc-output-label">15 Year Fixed</div><div class="hlc-output-rate-container"><span class="hlc-output-rate hlc-output-fixed15"></span>/mo</div></div><div class="hlc-output"><div class="hlc-output-label">5/1 ARM</div><div class="hlc-output-rate-container"><span class="hlc-output-rate hlc-output-arm5"></span>/mo</div></div><div class="hlc-output-info"><span>Estimated taxes &amp; insurance of <span class="hlc-output-taxes-and-insurance"></span> are not included.</span></div></div><div class="ab-control"><a class="zmm-shopping-link button za-track-event" data-za-category="Mortgages" data-za-action="Upsell click" data-za-label="HDP:HeaderLoanCalculator">See current rates</a><span>On </span><a class="zmm-shopping-link za-track-event" data-za-category="Mortgages" data-za-action="Upsell click" data-za-label="HDP:HeaderLoanCalculator"><img src="http://www.zillowstatic.com/static/images/zmmupsells/2x/zmm-h2-logo.png" alt="Zillow Mortgage Marketplace" width="133"></a></div><div class="hlc-footer ab-variant-1"><a href="/pre-approval/" class="button zmm-preapproval-link za-track-event" data-za-category="Mortgages" data-za-action="Header Loan Calculator" data-za-label="Preapproval Upsell">Next: Get Pre-Approved</a><a class="zmm-shopping-link"><img src="http://www.zillowstatic.com/static/images/zmmupsells/2x/zmm-h2-logo.png" alt="Zillow Mortgage Marketplace" width="130"></a></div><div class="hlc-footer ab-variant-2 ab-variant-3 ab-variant-4"><strong>Are you pre-approved yet?</strong><p>Pre-approval demonstrates to agents and sellers that you're a credible buyer and allows you to act fast when you find the home you want to buy.</p><a href="/pre-approval/" class="button zmm-preapproval-link za-track-event" data-za-category="Mortgages" data-za-action="Header Loan Calculator" data-za-label="Preapproval Upsell">Get Pre-Approved</a><span>On </span><a class="zmm-shopping-link"><img src="http://www.zillowstatic.com/static/images/zmmupsells/2x/zmm-h2-logo.png" alt="Zillow Mortgage Marketplace" width="145"></a></div></div><div class="payment clearfix"><div class="zbti"><span id="monthlyPaymentAmount"><span class="hlc-output-fixed30"></span></span><span class="de-emph per-mo">/mo</span><span class="payment-wrapper"><a class="monthly-payment-arrow z-hdp-ts" id="monthlyPaymentArrow" href="#">&nbsp;</a></span></div></div></div></div><div class="partner-link prop-value-ad"><a data-za-category="Mortgages" data-za-action="Upsell click" class="za-track-event" data-za-label="!zmm" href="/pre-approval/#stateAbbreviation=TX" data-zmm-component="HeaderDetailsLink">Get Pre-Approved on Zillow</a></div><div class="partner-link prop-value-ad"><div id="z_ad_WxxSNQHVR_-CTSwvJGgDSw_0-target" class="deferred-iframe-target" style="height:13px;width:256px"></div></div>
        </div>
</div>

    * Snippet 4 - schools

    * Snippet 5 - geo breadcrumb

    new Y.Z.GeoBreadcrumb();
var asyncLoader = new Y.Z.AsyncLoader();
asyncLoader.load({"boundingBox":"#map-tabs","phaseType":"scroll","jsModule":"zillow-async-block"});
asyncLoader.load({"boundingBox":"#google-street-view","phaseType":"custom","longitude":-97.017695,"latitude":32.575159,"customEvent":"tabContentMgr:googlestreet","jsModule":"z-hdp-google-street-view"});
asyncLoader.load({"zoom":18,"phaseType":"custom","customEvent":"tabContentMgr:birdseye","mapMarkerClass":"hdp-map-view-marker-birdseye","showScalebar":true,"boundingBox":"#hdp-map-view-birdseye","mapMarkerDimension":25,"disableKeyboardInput":true,"longitude":-97.017695,"latitude":32.575159,"disableZooming":true,"jsModule":"z-hdp-map-view","mapType":"birdseye"});
asyncLoader.load({"zoom":14,"phaseType":"custom","customEvent":"tabContentMgr:map","mapMarkerClass":"forSale hdp-home-circle hdp-map-view-marker-home","showScalebar":false,"boundingBox":"#hdp-map-view","mapMarkerDimension":17,"disableKeyboardInput":true,"longitude":-97.017695,"latitude":32.575159,"disableZooming":true,"jsModule":"z-hdp-map-view","mapType":"road"});
asyncLoader.load({"isLeftAligned":true,"enableAdUpsell":false,"phaseType":"load","useCarouselPeek":true,"baseFluid":true,"isForRent":false,"boundingBox":"#home-image-preview","enableStreetEasyUpsell":false,"enableShare":false,"stripConfig":{"imgPerStrip":7,"strips":["http://iss.zillowstatic.com/sheet/ZILLOW/p_a_8/Z01MgGv0QwqDNCDH1KZKBzMwn-4/IShjyqwv5uw8c21000000000/ISt8qaiyg514d01000000000/ISl2oyzrcxs6d01000000000/ISlqu37mscveq21000000000/ISdwlmhl8pk9d01000000000/IS5qjaze4hccd01000000000/ISdksrofo4nhq21000000000/ISxjhyg8094fd01000000000","http://iss.zillowstatic.com/sheet/ZILLOW/p_a_8/7JRu23Tqe-fR66Rvb94yS6b7WsQ/ISxjhyg8094fd01000000000/ISpdfmy1w0whd01000000000/IS5eqf69kwekq21000000000/ISh7dagvrsnkd01000000000/ISx7o3o2go6nq21000000000/IS91byxonkfnd01000000000/IS1v8mfijc7qd01000000000/ISp1mr5wbgypq21000000000","http://iss.zillowstatic.com/sheet/ZILLOW/p_a_8/9cXC2Z5pJpJ0aHaT7aykLvsIFaA/ISp1mr5wbgypq21000000000/ISto6axbf4zsd01000000000/IShvjfnp78qsq21000000000/ISli4ye5bwqvd01000000000/ISdc2mwy6oiyd01000000000/IS9ph35j30ivq21000000000/IS560aes2ga1e01000000000/ISxzxxvly724e01000000000","http://iss.zillowstatic.com/sheet/ZILLOW/p_a_8/-WyxdhjE7drDW0LskBMd5yjoUYY/ISxzxxvly724e01000000000/IS1jfrmczr9yq21000000000/ISptvldfuzt6e01000000000/IShnt9v8qrl9e01000000000"]},"enableDigsUpsell":true,"enableThumbs":true,"jsModule":"z-hdp-photo-carousel"});
ZILLOW.AsyncContentMgr.addComponent({"boundingBox":"#z-digs-upsell","global":false,"name":"z-digs-upsell","phaseType":"scroll","cfg":{"id":"z-digs-upsell","klass":"AsyncBlock","blocks":[{"boundingBox":"#z-digs-upsell-upsell","url":"/AjaxRender.htm?encparams=5~1733496107950239319~4UfU8lTEH7iV2Yy_0Yibk4GrgUpfzW2p9nltkymGT976JJcKlOeK4cNU2iEuiNqqzhINnbESDMvLb9DcnNd13GWMwEKQYMHNwSfACHPYs3aTf5VQGWRmD_VMX7UWRmlXyh6j3-kZkg6B5JXnL0aWXrcNe23o0pvQ9fab8UGBrF3lTgdujOiqh9HJX8GPJVvg&rwebid=7490941&rhost=1","divId":"z-digs-upsell"}]},"customEvent":"digsUpsell:renderUpsell","requires":["zillow-async-block"],"initializer":"_create","jsModule":"zillow-async-block"});
asyncLoader.load({"phaseType":"scroll","ajaxURL":"/AjaxRender.htm?encparams=7~1399157769210804121~ZuV2hkfb-Bj-z3_OsuzjYZ6WbzYAgourRzdsV8u8W2E7GtSpZirMW9Mne1xjyPlQ1D6R7MIQlOaoPi_ZC6QM-LZiO518BIxYF6-aUtas2xogzpkVQbCkNFUkm1Hp-5QCjSHp6mTrSjl47my0iNMv_d7IKl61E9cCxrXxsrAo2kXm81tObl03qQ==&rwebid=7490941&rhost=1","divId":"homedetail_toolbox_PropertyNotes","jsModule":"zillow-async-block"});

    * Snippet 6:
        new Y.Z.HdpOtherCosts();
    asyncLoader.load({"boundingBox":"#maps-and-views","mapInfo":{"premierCommunity":false,"communityName":" ","isPlan":false,"subdivisionId":""},
"phaseType":"scroll","zoomLevel":17,"googleEnabled":true,
"latLong":{"longitude":-97.017695,"latitude":32.575159},"zpid":"27062400","jsModule":"zillow-hdp-map-loader"});

     * Snippet 7:
     *     var data = {};

        data['SHO_MAGIC_CARPET'] = 'CONTROL';

        data['aamgnrc1'] = '1102+Redbud+St';

        data['aamgnrc2'] = 'Celina';

        data['aamgnrc3'] = 'TX';

        data['aamgnrc4'] = '75009+Celina+Collin+TX+Texas';

     *   data['asking'] = '279900';

        data['dma'] = 'dallas';

        data['fsbid'] = '35';

        data['fsbo'] = 'false';

        data['generic'] = '75009';

        data['hoc_buy'] = '10';

        data['mlb'] = 'pr2';

        data['priceReduced'] = 'false';

        data['price_band'] = 'z200';

        data['rt'] = '9';

        data['sqft'] = '3300';

    *    data['sqftx'] = '3305';

        data['yrBlt'] = '1990';

    *    data['yrbltx'] = '1995';

        data['z600'] = 'false';

        data['z_listing_image_url'] = 'http://photos1.zillowstatic.com/p_d/IS18ehdmrcgid2b.jpg';

        data['zask'] = '20';

    *    data['zbath'] = '4';

    *    data['zbed'] = '5';

    *    data['zcity'] = 'Celina';

    *    data['zcounty'] = 'Collin';

        data['zdays'] = '1';

        data['zdeck'] = 'true';

    *    data['zestimate'] = '231878';

        data['zipextension'] = '3976';

        data['zls'] = '11';

    *    data['zmo'] = 'ForSale';

    *    data['zstate'] = 'TX';

    *    data['zzip_code'] = '75009';

    *    data['zzpid'] = '53098340';
    * (zzpid is the Zillow home ID)
    * var bkReporter

    * Snippet 8:
             window._gaqBackup = [
["_setCustomVar", 26, "BrokerId", "3031"],
["_setCustomVar", 14, "HDP Type", "ForSale"],
["_setCustomVar", 25, "Device Type", "Desktop"],
["_setCustomVar", 1, "Guid", "e5b610cdc50d472ebe799f6bfb99f693", 1],
["_setCustomVar", 12, "UserType", "Unrecognized", 2],
["_setCustomVar", 36, "A/B Test 11", "SELMetricsUI.CONTROL", 1],
["_setCustomVar", 50, "A/B Test 10", "RE-MobileWebFormRefresh.ON", 1],
["_setCustomVar", 38, "A/B Test 13", "RE-AnimateContactForm.CONTROL", 1],
["_setCustomVar", 37, "A/B Test 12", "MortgageSponsoredQuotes.FEATURED_QUOTES", 1],
["_setCustomVar", 27, "TrackingContactModule", "featured\\HDP:-C-Column\\11111\\1\\21\\1"],
["_setCustomVar", 40, "A/B Test 15", "SHO_ScrubbingBubbles.SB_BOUNDARIES_AND_MARKERS_ON", 1],
["_setCustomVar", 39, "A/B Test 14", "ApolloContactV1.APOLLO_CONTACT_EDITED", 1],
["_setCustomVar", 49, "A/B Test 9", "ADS_BAL_TALK.ON", 1],
["_setCustomVar", 42, "A/B Test 2", "TENgageSEMRegistrationUI.SEARCH_LIGHTBOX", 1],
["_setCustomVar", 41, "A/B Test 1", "ADS_BALPALS.CONTROL", 1],
["_setCustomVar", 44, "A/B Test 4", "SemNationalPage.SEARCH_FREEPASS_IMAGE", 1],
["_setCustomVar", 43, "A/B Test 3", "SHO_MAGIC_CARPET.CONTROL", 1],
["_setCustomVar", 46, "A/B Test 6", "MortgagePreapprovalFlow.MULTI_STEP_CREDIT_CONTACT_BEFORE_DOWN_NO_LENDER_SELECTION", 1],
["_setCustomVar", 45, "A/B Test 5", "MortgageNewAdPlacements.BOTH_AD_PLACEMENTS", 1],
["_setCustomVar", 48, "A/B Test 8", "HDP_Collapsed_Mobile_Web.ON_W_COLLAPSE_OLD", 1],
["_setCustomVar", 47, "A/B Test 7", "DigsLandingPage.ON", 1],
["_setCustomVar", 7, "Neighborhood", ""],
["_setCustomVar", 24, "ListingFeatureType", "featured"],
["_setCustomVar", 22, "AccountCreationDate", "", 1],
["_setCustomVar", 6, "Zip", "75104"],
["_setCustomVar", 23, "SignedInNow", "false", 2],
["_setCustomVar", 5, "City", "Grand Prairie"],
["_setCustomVar", 9, "PropertyType", "Single Family"],
["_setCustomVar", 4, "County", "Dallas"],
["_setCustomVar", 3, "State", "TX"],
["_setCustomVar", 10, "Price", "$323,000"],
["_setCustomVar", 8, "Address", "911 Mallard Pointe Dr"],
["_setCustomVar", 2, "House ID / Zpid", "27062400"],
["_setCustomVar", 15, "Listing Type", "For Sale (Broker)"],
["_trackPageview", "\/homedetails\/911-Mallard-Pointe-Dr-Cedar-Hill-TX-75104\/27062400_zpid\/#ForSale"]
];

****/
    QRegExp rxData(".*var data = \\{\\};(.+)var bkReporter.*" );
    QRegExp rxZzpid(".*data\\['zzpid'\\]\\s*=\\s*'(.*)';");
    if (rxData.exactMatch(s))
    {
        QStringList a( rxData.cap(1).split('\n', QString::SkipEmptyParts) );
        // Expressions for items we're interested in
        QRegExp rxPrice(".*data\\['asking'\\]\\s*=\\s*'(.*)';");
        QRegExp rxYear(".*data\\['yrbltx'\\]\\s*=\\s*'(.*)';");
        QRegExp rxSqft(".*data\\['sqftx'\\]\\s*=\\s*'(.*)';");
        QRegExp rxBath(".*data\\['zbath'\\]\\s*=\\s*'(.*)';");
        QRegExp rxBed(".*data\\['zbed'\\]\\s*=\\s*'(.*)';");
        QRegExp rxAddress(".*data\\['aamgnrc1'\\]\\s*=\\s*'(.*)';");
        // zcity is sometimes undefined, oddly...
        QRegExp rxCity(".*data\\['zcity'\\]\\s*=\\s*'(.*)';");
        QRegExp rxCity2(".*data\\['aamgnrc2'\\]\\s*=\\s*'(.*)';");
        QRegExp rxCounty(".*data\\['zcounty'\\]\\s*=\\s*'(.*)';");
        QRegExp rxState(".*data\\['zstate'\\]\\s*=\\s*'(.*)';");
        QRegExp rxZip(".*data\\['zzip_code'\\]\\s*=\\s*'(.*)';");
        QRegExp rxZestimate(".*data\\['zestimate'\\]\\s*=\\s*'(.*)';");
        QRegExp rxZmo(".*data\\['zmo'\\]\\s*=\\s*'(.*)';");
        int n;
        QString sCity;
        // Address and city (anything from aamgnrc?) need to be url-unescaped
        // QUrl::fromPercentEncoding is not needed
        for (n = 0; n < a.length(); n++)
        {
            if (rxPrice.exactMatch(a[n])) ui->tblData->setItem(rowIndex, COL_PRICE, new QTableWidgetItem(rxPrice.cap(1)));
            if (rxYear.exactMatch(a[n])) ui->tblData->setItem(rowIndex, COL_YEAR, new QTableWidgetItem(rxYear.cap(1)));
            if (rxSqft.exactMatch(a[n])) ui->tblData->setItem(rowIndex, COL_SQFT, new QTableWidgetItem(rxSqft.cap(1)));
            if (rxBath.exactMatch(a[n])) ui->tblData->setItem(rowIndex, COL_BA, new QTableWidgetItem(rxBath.cap(1)));
            if (rxBed.exactMatch(a[n])) ui->tblData->setItem(rowIndex, COL_BR, new QTableWidgetItem(rxBed.cap(1)));
            if (rxAddress.exactMatch(a[n])) ui->tblData->setItem(rowIndex, COL_ADDRESS, new QTableWidgetItem(rxAddress.cap(1).replace('+',' ')));
            if (rxCity.exactMatch(a[n])) sCity = rxCity.cap(1).replace('+',' ');
            if (rxCity2.exactMatch(a[n])) sCity = rxCity2.cap(1).replace('+',' ');
            if (rxCounty.exactMatch(a[n])) ui->tblData->setItem(rowIndex, COL_COUNTY, new QTableWidgetItem(rxCounty.cap(1)));
            if (rxZip.exactMatch(a[n])) ui->tblData->setItem(rowIndex, COL_ZIP, new QTableWidgetItem(rxZip.cap(1))); //ui->txtDebug->appendPlainText("zip:" + rxZip.cap(1));
            if (rxZestimate.exactMatch(a[n])) ui->tblData->setItem(rowIndex, COL_ZESTIMATE, new QTableWidgetItem(rxZestimate.cap(1)));
            if (rxZmo.exactMatch(a[n])) ui->txtDebug->appendPlainText("zmo: " + rxZmo.cap(1));
            if (rxState.exactMatch(a[n])) ui->tblData->setItem(rowIndex, COL_STATE, new QTableWidgetItem(rxState.cap(1)));
            if (rxZzpid.exactMatch(a[n])) ui->txtDebug->appendPlainText("zzpid: " + rxZzpid.cap(1));
        }
        ui->tblData->setItem(rowIndex, COL_CITY, new QTableWidgetItem(sCity));
        //ui->txtDebug->appendPlainText("parsed\n");
        //ui->txtDebug->appendPlainText(s2);
    }
    // Things we need to scrape from facts and features, etc.
    QRegExp rxLot(".*<li><div class=\"fact-bullet\">Lot: ([^<]+)<\\/div><\\/li>.*", Qt::CaseInsensitive, QRegExp::RegExp2);
    QRegExp rxHOA(".*<li><div class=\"fact-bullet\">HOA Fee: ([^<]+)<\\/div><\\/li>.*", Qt::CaseInsensitive, QRegExp::RegExp2);
    QRegExp rxLatLong(".*new Y.Z.HdpOtherCosts\\(\\);\\s*asyncLoader.load\\(\\{.*\"latLong\":\\{\"longitude\":([^,]+),\"latitude\":([^,]+)\\},.*");
    int dbLat = 0, dbLong = 0;
    if (rxLot.exactMatch(s))
    {
        ui->tblData->setItem(rowIndex, COL_LOT, new QTableWidgetItem(sortableLotSize(rxLot.cap(1))));
    }
    if (rxHOA.exactMatch(s))
    {
        ui->tblData->setItem(rowIndex, COL_HOA, new QTableWidgetItem(rxHOA.cap(1)));
    }
    if (rxLatLong.exactMatch(s))
    {
        ui->tblData->setItem(rowIndex, COL_GEO, new QTableWidgetItem(rxLatLong.cap(2) + ',' + rxLatLong.cap(1)));
        setDistanceColumn(rowIndex);
        dbLat = (int)(rxLatLong.cap(2).toDouble() * 1000000.0);
        dbLong = (int)(rxLatLong.cap(1).toDouble() * 1000000.0);
    }
    else qDebug() << "Failed to get lat/long";
    QRegExp rxSchools(".*<section class=\"nearby-schools no-decor\">(.+)<\\/section>.*");
    double avgSchool = 0.0;
    if (rxSchools.exactMatch(s))
    {
        QString s2( rxSchools.cap(1) );
        // Schools may not be asssigned. We can assume the assigned ones will come first
        QRegExp rxNearbyList(".*<li class=\"nearby-school[^\"]*\">(.+)<\\/li>.*");
        if (rxNearbyList.exactMatch(s2))
        {
            // Split into multiple elements
            QRegExp rxNearbyListSep("<\\/li>\\s*<li class=\"nearby-school[^\"]*\">");
            QStringList aSchools( rxNearbyList.cap(1).split(rxNearbyListSep, QString::SkipEmptyParts ) );
            if (aSchools.length() > 0)
            {
                ui->txtDebug->appendPlainText(QString().sprintf("%d schools:\n", aSchools.length()));
                /*** Example of content within <li class="nearby-school assigned-school">..</li>
                 *             <span class="gs-rating-badge">
                <div class="gs-rating gs-rating-8"><span class="gs-rating-number">8</span><span class="gs-rating-subtext"> out of 10</span></div>
            </span>
            <span class="nearby-schools-name">

                    <a href="/crandall-tx/schools/w-a-martin-elementary-school-93450/" class="ga-tracked-link track-ga-event school-name notranslate" data-ga-action="School details click" data-ga-label="HDP AB Module" data-ga-category="Homes" data-ga-standard-href="true">W A Martin Elementary</a>

                 <span class="assigned-label de-emph">(assigned)</span>
            </span>
            <span class="nearby-schools-grades">PK-6</span>
            <span class="nearby-schools-distance">5.3 mi</span>

                 ***/
                int n;
                // Use non-greedy matching
                QRegExp rxRating(".*<span class=\"gs-rating-number\">([^<]+)<\\/span><span class=\"gs-rating-subtext.*", Qt::CaseInsensitive, QRegExp::RegExp2);
                QRegExp rxSchoolName(".*<span class=\"nearby-schools-name\">\\s*<a href=[^>]*>([^<]+)<\\/a>.*", Qt::CaseInsensitive, QRegExp::RegExp2);
                QRegExp rxSchoolGrades(".*<span class=\"nearby-schools-grades\">([^<]+)<\\/span>.*", Qt::CaseInsensitive, QRegExp::RegExp2);
                QRegExp rxSchoolDistance(".*<span class=\"nearby-schools-distance\">(.+) mi<\\/span>.*", Qt::CaseInsensitive, QRegExp::RegExp2);
                double scoreTotal = 0.0;
                double minDistance = 100.0;
                int scoreCount = 0;
                for (n = 0; n < aSchools.length(); n++)
                {
                    QString sRating;
                    QString sSchoolName;
                    QString sSchoolGrades;
                    QString sSchoolDistance;
                    if (rxRating.exactMatch(aSchools[n]))
                    {
                        sRating = rxRating.cap(1);
                        if (!sRating.isEmpty() && sRating != "NR")
                        {
                            bool ok;
                            scoreTotal += sRating.toDouble(&ok);
                            if (ok) scoreCount++;
                        }
                    }
                    if (rxSchoolName.exactMatch(aSchools[n])) sSchoolName = rxSchoolName.cap(1);
                    if (rxSchoolGrades.exactMatch(aSchools[n])) sSchoolGrades = rxSchoolGrades.cap(1);
                    if (rxSchoolDistance.exactMatch(aSchools[n]))
                    {
                        sSchoolDistance = rxSchoolDistance.cap(1);
                        if (!sSchoolDistance.isEmpty())
                        {
                            bool ok;
                            double dist = sSchoolDistance.toDouble(&ok);
                            if (ok && dist < minDistance) minDistance = dist;
                        }
                    }
                    if (!sSchoolName.isEmpty()) ui->txtDebug->appendPlainText("[" + sRating + "/10] " + sSchoolName + " Grades " + sSchoolGrades + " Distance:" + sSchoolDistance);
                }
                // Aggregate
                QString sSchoolSummary("No info");
                if (scoreCount > 0)
                {
                    avgSchool = scoreTotal / scoreCount;
                    sSchoolSummary = QString().sprintf("%.1f N=%d dst %.1f", avgSchool, scoreCount, minDistance);
                }
                ui->tblData->setItem(rowIndex, COL_SCHOOLS, new QTableWidgetItem(sSchoolSummary));
            }
        }
    }
    updateDbFromTable( rowIndex, rxZzpid.cap(1), avgSchool, dbLat, dbLong );
    m_pageProcessingPending = 0;
    // More to fetch?
    if (m_fetchQueue.isEmpty()) return;
    qDebug() << "Triggering next load...";
    startPlay(m_pcmBeep2);
    QTimer::singleShot(1000, this, SLOT(processQueue()));
}

void MainWindow::processWebView()
{
    // For saved search, map results:
    // We may have <div id="list-container" class=""
    // followed by <div id="search-results" class="pop-bubble">
    // with multiple <article>...</article> containers:
    /******
<article id="zpid_80304051" statustype="1" role="article" class="property-listing property-listing-large rollable plisting notification-unread featured-listing image-loaded"
longitude="-97768785" latitude="32393621">
  <figure class="photo left column property-thumb photoex" role="img" data-photourl="http://photos1.zillowstatic.com/p_g/ISdk80i0806vuz0000000000.jpg">
  <a href="/homedetails/4600-Contrary-Creek-Rd-Granbury-TX-76048/80304051_zpid/" class="routable mask hdp-link">
    <img class="image-loaded" rel="nofollow" src="http://photos1.zillowstatic.com/p_g/ISdk80i0806vuz0000000000.jpg" alt="4600 Contrary Creek Rd, Granbury, TX">
    <figcaption class="photo-count">22 Photos</figcaption>
  </a></figure>
  <div class="property-listing-data" id="yui_3_15_0_1_1407059789911_7881"><div class="property-info" id="yui_3_15_0_1_1407059789911_7880">
  <dl class="property-info-list col-1 column"><dt class="type-forSale type show-icon"><div class="type-icon"></div><strong>House For Sale</strong></dt><dt class="price-large"><strong>$224,000</strong></dt><div class="colabc"><div class="zestimate"><span class="right-align definition yui3-tooltip-trigger" data-tooltip-key="zest-tip-list">Zestimate</span><sup>®</sup>: $283K</div></div><dt><span class="zbt featured-listing ir">Featured Listing</span></dt><dt class="type-uss"><span class="price-reduction"><span class="arrow"></span>$25,000 (08/02)</span></dt></dl><dl class="property-info-list col-2 column" id="yui_3_15_0_1_1407059789911_7879"><dt class="property-address" id="yui_3_15_0_1_1407059789911_7878"><a href="/homedetails/4600-Contrary-Creek-Rd-Granbury-TX-76048/80304051_zpid/" class="hdp-link routable" title="4600 Contrary Creek Rd, Granbury, TX Real Estate" id="yui_3_15_0_1_1407059789911_7877">4600 Contrary Creek Rd, Granbury, TX</a></dt><dt class="property-data" id="yui_3_15_0_1_1407059789911_7883">4 b<span class="hide-when-narrow">e</span>d<span class="hide-when-narrow">s</span>, 2.5 ba<span class="hide-when-narrow">ths</span>, 3,381 sqft</dt><dt class="property-lot">4.14 ac lot </dt><dt class="property-year"> Built in 1994</dt></dl><div class="list-card-actions col-3"><a href="/myzillow/UpdateFavorites.htm?zpid=80304051&amp;operation=add&amp;ajax=false" rel="nofollow" data-fm-zpid="80304051" data-fm-callback="resultListSaveFavoriteSuccessHandler" title="Save this home" data-za-label="Save Map:List" class="fm-save-event" id="auth_SaveFavoriteAuthCheck"><span class="zpi list-save-icon"></span></a><a href="/myzillow/UpdateHiddenProperty.htm?hidezpid=80304051&amp;hideop=hide" rel="nofollow" data-hpm-zpid="80304051" data-hpm-callback="resultListHidePropertySuccessHandler" title="Hide this home" data-za-label="HiddenHomes Map:List" class="hpm-hide-event list-hide-button" id="auth_HidePropertyAuthCheck"><span class="zpi list-hide-icon"></span></a></div></div></div><div class="minibubble template hide">&nbsp;<!--["$224,000","http://photos1.zillowstatic.com/p_a/ISdk80i0806vuz0000000000.jpg",4,2.5,3381,false,[3,"$25,000","(Aug 2)"]]--></div></article>
     * *****/
    // For saved search, list results:
    // We have <div id="list-container" class="seo-list-container" followed by <div id="search-results" class="pop-bubble">
    // with multiple article elements (class has seo-list-only added):
    // <article id="zpid_80304051" statustype="1" role="article" class="property-listing property-listing-large rollable plisting notification-unread featured-listing seo-list-only image-loaded"
    //longitude="-97768785" latitude="32393621"><figure class="photo left column property-thumb photoex" role="img" data-photourl="http://photos1.zillowstatic.com/p_g/ISdk80i0806vuz0000000000.jpg"><a href="/homedetails/4600-Contrary-Creek-Rd-Granbury-TX-76048/80304051_zpid/" class="routable mask hdp-link"><img class="image-loaded" rel="nofollow" src="http://photos1.zillowstatic.com/p_g/ISdk80i0806vuz0000000000.jpg" alt="4600 Contrary Creek Rd, Granbury, TX"><figcaption class="photo-count">22 Photos</figcaption></a></figure><div class="property-listing-data"><div class="right property-info"><dl class="property-info-sheet pier-1"><dt class="property-address"><a href="/homedetails/4600-Contrary-Creek-Rd-Granbury-TX-76048/80304051_zpid/" class="hdp-link routable" title="4600 Contrary Creek Rd, Granbury, TX Real Estate">4600 Contrary Creek Rd, Granbury, TX</a></dt><div class="colabc"><div class="doz">180 days on Zillow</div></div><dt class="type-uss"><span class="price-reduction"><span class="arrow"></span>$25,000 (08/02)</span></dt><dt><span class="zbt featured-listing ir">Featured Listing</span></dt></dl><dl class="property-info-sheet pier-2"><dt class="type-forSale type show-icon"><div class="type-icon"></div><strong>House For Sale</strong></dt><dt class="price-large"><strong>$224,000</strong></dt><div class="colabc"><div class="zestimate"><span class="right-align definition yui3-tooltip-trigger" data-tooltip-key="zest-tip-list">Zestimate</span><sup>®</sup>: $283K</div></div></dl><dl class="property-info-sheet pier-3"><dd>4</dd><dt>Beds</dt></dl><dl class="property-info-sheet pier-4"><dd>2.5</dd><dt>Baths</dt></dl><dl class="property-info-sheet pier-5"><dd>3,381</dd><dt> Sqft </dt></dl><dl class="property-info-sheet pier-6"><dd>4.14 ac</dd><dt> Lot </dt></dl><dl class="property-info-sheet pier-7"><dd>1994</dd><dt> Built </dt></dl><div class="list-card-actions col-3"><a href="/myzillow/UpdateFavorites.htm?zpid=80304051&amp;operation=add&amp;ajax=false" rel="nofollow" data-fm-zpid="80304051" data-fm-callback="resultListSaveFavoriteSuccessHandler" title="Save this home" data-za-label="Save SEOMap:List" class="fm-save-event" id="auth_SaveFavoriteAuthCheck"><span class="zpi list-save-icon"></span></a><a href="/myzillow/UpdateHiddenProperty.htm?hidezpid=80304051&amp;hideop=hide" rel="nofollow" data-hpm-zpid="80304051" data-hpm-callback="resultListHidePropertySuccessHandler" title="Hide this home" data-za-label="HiddenHomes SEOMap:List" class="hpm-hide-event list-hide-button" id="auth_HidePropertyAuthCheck"><span class="zpi list-hide-icon"></span></a></div></div></div><div class="minibubble template hide">&nbsp;<!--["$224,000","http://photos1.zillowstatic.com/p_a/ISdk80i0806vuz0000000000.jpg",4,2.5,3381,false,[3,"$25,000","(Aug 2)"]]--></div></article>
    // For my homes, map results and list results work with above criteria
    // Capture starting after first <article so we can separate using "</article><article " and all will be in the same format
    QRegExp rxList(".*<div id=\"list-container\" class=\"([^\"]*)\".*<div id=\"search-results\" class=\"pop-bubble\">.*<article (.*)<\\/article>.*");
    QRegExp rxList2(".*<div id=\"list-container\" class=\"([^\"]*)\".*<div id=\"search-results\" class=\"pop-bubble\">.*");
    QString s( ui->webView->page()->mainFrame()->toHtml() );
    if (rxList.exactMatch(s))
    {
        // Update list
        qDebug() << "Got list:" << rxList.cap(1); // empty or seo-list-container
        QStringList lstArticles( rxList.cap(2).split(QRegExp("<\\/article><article\\s+"), QString::SkipEmptyParts) );
        int n;
        if (lstArticles.length() > 0)
        {
            emit startPage();
            // class and role can vary in order. We don't need role
            QRegExp rxArticle( "id=\"([^\"]+)\"\\s+statustype=\"([^\"]+)\"[^>]*\\s+class=\"([^\"]+)\"[^>]*\\s+longitude=\"([^\"]+)\"\\s+latitude=\"([^\"]+)\"\\s*>(.*)" );
            // cap(1) = zpid_*, cap(2) = statustype, cap(3) = class, cap(4) = longitude * 10E6, cap(5) = latitude * 10E6,
            // cap(6) = <figure .*><a .*>.*</a></figure> and everything else
            QRegExp rxPrice( ".*<dt class=\"price-large\"><strong>([^<]+)<\\/strong><\\/dt>.*" ); // within cap(3) of rxFigure
            QRegExp rxFigure( "<figure [^>]+><a href=\"([^\"]*)\"[^>]*>(.*)<\\/a><\\/figure>(.*)"
                              "<dt class=\"property-address\".*<a [^>]+>([^<]+)<\\/a><\\/dt>.*"
                              "<dt class=\"property-data\"[^>]*>(.*)<\\/dt>.*"
                              "<dt class=\"property-lot\".*>(.*)<\\/dt>(.*)<\\/dl>.*");
            QRegExp rxYear(   ".*<dt class=\"property-year\".*>(.*)<\\/dt>.*");
            // cap(1) = url, cap(2) = picture with caption, cap(3) = possible container for price and/or zestimate, cap(4) = address,
            // cap(5) = br/ba/sqft, cap(6) = lot size, cap(7) = container for year built
            QRegExp rxZestimate(".*<div class=\"zestimate\"><span .*>Zestimate<\\/span><sup>.<\\/sup>: ([^<]+)<\\/div>.*");
            //QRegExp rxFigure2( "<figure [^>]+><a href=\"([^\"]*)\"[^>]*>(.*)<\\/a><\\/figure>.*<dt class=\"price-large\"><strong>([^<]+)<\\/strong><\\/dt>.*");
            for (n = 0; n < lstArticles.length(); n++)
            {
                if (!rxArticle.exactMatch(lstArticles[n]))
                {
                    qDebug() << "Entry" << n << "no match for rxArticle";
                    if (n == 0) qDebug() << lstArticles.at(n);
                    continue;
                }
                if (!rxFigure.exactMatch(rxArticle.cap(6)))
                {
                    qDebug() << "Entry" << n << "no match for rxFigure from" << rxArticle.cap(6);
                    if (n == 0) qDebug() << lstArticles.at(n);
                    continue;
                }
                QString isFavorite;
                if (rxArticle.cap(3).contains("type-favorite")) isFavorite = "* ";
                QString sPrice;
                QString sYear;
                if (rxZestimate.exactMatch(rxFigure.cap(3))) sPrice = "Zest: " + rxZestimate.cap(1);
                if (rxPrice.exactMatch(rxFigure.cap(3)))
                {
                    sPrice = rxPrice.cap(1);
                }
                if (rxYear.exactMatch(rxFigure.cap(7)))
                {
                    sYear = "; " + rxYear.cap(1);
                }
                else qDebug() << "No year in" << rxFigure.cap(7);
                // first line is link url,
                // second line is price + address,
                // third line consists of all other facts,
                // fourth line is latitude longitude dbid rowindex (-1 -1 if not present)
                int dbid = -1;
                int rowIndex = -1;
                QString sLink("http://www.zillow.com" + rxFigure.cap(1));
                if (m_urlToRow.contains(sLink)) rowIndex = m_urlToRow[sLink];
                emit article(rxFigure.cap(1) + "\n"
                             + isFavorite + sPrice + " " + rxFigure.cap(4) + "<br/>\n"
                             + rxFigure.cap(5) + "; " + rxFigure.cap(6) + sYear + "\n"
                             + rxArticle.cap(5) + " " + rxArticle.cap(4) + QString().sprintf( " %d %d", dbid, rowIndex ) );
            }
            emit endPage();
            qDebug() << "Found articles:" << lstArticles.length();
            return;
        }
        else qDebug() << "List match but no articles";
    }
    else if (rxList2.exactMatch(s))
    {
        qDebug() << "Partial match on list:" << rxList2.cap(1);
        //QStringList lstArticles( rxList2.cap(2).split("</article><article ", QString::SkipEmptyParts) );
        //qDebug() << "Got article count: " << lstArticles.length();
    }
    else qDebug() << "Not a monitored url - no list match";
}

static double deg2rad( double d )
{
    return d * (M_PI / 180);
}

void MainWindow::setDistanceColumn(int rowId)
{
    QString latLong(safeItemText(rowId, COL_GEO));
    if (!latLong.isEmpty() && ui->chkShowDistance->isChecked())
    {
        QString reference(ui->txtReference->text());
        if (!reference.isEmpty())
        {
            QStringList aLL1(latLong.split(','));
            QStringList aLL2(reference.split(','));
            if (aLL1.length() >= 2 && aLL2.length() >= 2)
            {
                double lat1 = aLL1[0].toDouble();
                double lat2 = aLL2[0].toDouble();
                double lon1 = aLL1[1].toDouble();
                double lon2 = aLL2[1].toDouble();
                double dLat = deg2rad(lat2 - lat1);
                double dLon = deg2rad(lon2 - lon1);
                // sin(dLat/2)^2 + cos(lat1) * cos(lat2) * sin(dLon/2)^2
                double a = sin(dLat / 2) * sin(dLat / 2) +
                        cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * sin(dLon/2) * sin(dLon/2);
                double c = 2 * atan2( sqrt(a), sqrt(1-a) );
                double dist = 6371 * c; // Distance in km
                double distMiles = dist * 0.621371;
                setTableCell(rowId, COL_DISTANCE, QString().sprintf("%5.1f", distMiles) );
                return;
            }
        }
    }
    setTableCell(rowId, COL_DISTANCE, "?" );
}

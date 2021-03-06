// SQL functions

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "columnids.h"
#include "sqldefs.h"

#include <QSqlQuery>
#include <QTableWidgetItem>

bool MainWindow::dbAvailable()
{
    if (m_db.isOpen())
    {
        if (dbVersionOk())
        {
            return true;
        }
    }
    QFileInfo fi(dbFilePath());
    if (!fi.exists())
    {
        logMsg("Creating sqlite database " + m_dbPath, 0);
    }
    //qDebug() << "Instantiating mysql connection, lib paths:" << QCoreApplication::libraryPaths();
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(m_dbPath);
    //qDebug() << "Attempting to open db";
    if (!m_db.open())
    {
        return false;
    }
    if (!dbVersionOk())
    {
        // Reopen database which was closed in dbReset(), called from dbVersionOk() in the failing case
        logMsg("Updating database", 0);
        bool openOK = m_db.open();
        if (!openOK) return false;
        // Create missing tables
        dbAssertTables();
        // Insert current version
        if (!dbInsertVersion()) logMsg("Failed to update db version", 0);
        return openOK;
    }
    // Opened successfully and version check passed
    return true;
}

QString MainWindow::dbFilePath()
{
    QDir appDir(QCoreApplication::applicationDirPath());
    // Use app dir if on C: or D: drive
    //if (!appDir.absolutePath().left(2).compare("c:", Qt::CaseInsensitive)
    //        || !appDir.absolutePath().left(2).compare("d:", Qt::CaseInsensitive))
    //{
        m_dbPath = appDir.absoluteFilePath(SQLITE_DB_BASENAME);
    //}
    //else
    //{
    //    QDir defaultDir(SQLITE_DB_DEFAULTDIR);
    //    if (!defaultDir.exists()) defaultDir.mkpath(defaultDir.absolutePath());
    //    m_dbPath = defaultDir.absoluteFilePath(SQLITE_DB_BASENAME);
    //}
    return m_dbPath;
}

bool MainWindow::dbSingleUpdate( unsigned int fromTo )
{
    QSqlQuery q(m_db);
    switch (fromTo)
    {
    case 0x0102:
        if (!q.exec(SQLITE_TABLEDEF_WEIGHTING)) logMsg( "Failed: " + q.lastError().text(), -1 );
        return true;
    //    if (!q.exec(SQLITE_ALTER_PROPERTY_ADDCOLS)) logMsg( "Failed: " + q.lastError().text(), -1 );
    //    return true;
    default:
        break;
    }
    return false;
}

bool MainWindow::dbVersionOk()
{
    // Check version. Table may not exist
    QSqlQuery q(m_db);
    m_dbVer = -1;
    // Assert dbver table (possibly empty)
    q.exec(SQLITE_TABLEDEF_DBVER);
    QSqlQuery qRead("SELECT version FROM zbr_dbver ORDER BY version DESC", m_db);
    if (!qRead.next())
    {
        logMsg("Forcing rebuild of sqlite database", 0);
        dbReset();
        return false;
    }
    // Attempt upgrade
    m_dbVer = qRead.value(0).toInt();
    if (m_dbVer <= 0)
    {
        logMsg("Forcing rebuild, invalid or missing version", 0);
        dbReset();
        return false;
    }
    if (m_dbVer < SQLITE_DATABASE_VERSION)
    {
        logMsg(QString().sprintf("Attempting upgrade from version %d to %d", m_dbVer, SQLITE_DATABASE_VERSION), 0);
        switch (m_dbVer * 0x100 + SQLITE_DATABASE_VERSION)
        {
        case 0x0102:
          if (!dbSingleUpdate(0x0102)) break;
            dbInsertVersion();
            break;
        }
    } // Upgrade required
    //else
    //{
    //    logMsg(QString().sprintf("Current database version %d found", m_dbVer), 3);
    //}
    return true;
}

void MainWindow::dbReset()
{
    if (m_db.isOpen())
    {
        m_db.close();
    }
    QFileInfo fi(dbFilePath());
    if (!fi.exists())
    {
        logMsg("No sqlite database found as " + m_dbPath, 0);
        return;
    }
    QFile f(m_dbPath);
    if (f.remove())
    {
        logMsg("Successfully removed sqlite db " + m_dbPath, 1);
    }
    else
    {
        logMsg("Failed to remove sqlite db " + m_dbPath + " - " + f.errorString(), -1);
    }
}

bool MainWindow::dbAssertTables()
{
    if (!m_db.isOpen())
    {
        qDebug() << "Assert tables failed, db not open";
        return false;
    }
    QSqlQuery q(m_db);
    int failures = 0;
    const char *defs[] = { SQLITE_TABLEDEFS_ALL };
    int n;
    for (n = 0; n < sizeof(defs)/sizeof(defs[0]); n++)
    {
        qDebug() << "Creating:" << defs[n];
        if (!q.exec(defs[n]))
        {
            failures++;
            qDebug() << "Failed!";
        }
        else
        {
            qDebug() << "OK";
        }
    }
    qDebug() << "Assert failure count:" << failures;
    return (failures == 0);
}

bool MainWindow::dbInsertVersion()
{
    QSqlQuery q(m_db);
    if (q.exec(QString().sprintf("INSERT INTO zbr_dbver (version) VALUES ('%d')", SQLITE_DATABASE_VERSION)))
    {
        m_dbVer = SQLITE_DATABASE_VERSION;
        return true;
    }
    else
    {
        logMsg("Failed to update current version:" + q.lastError().text(), -1);
    }
    return false; // insert failed
}

int MainWindow::loadSpreadsheet(QString orderBy)
{
    if (!dbAvailable())
    {
        qDebug() << "Cannot load: db not available";
        return -1;
    }
    m_sheetLoadBusy = 1;
    loadUsercols();
    QSqlQuery q(m_db);
    QString sQuery("SELECT * FROM zbr_property ORDER BY " + orderBy);
    if (!q.exec(sQuery))
    {
        qDebug() << "Query failed for load:" << q.lastError().text();
        m_sheetLoadBusy = 0;
        return -1;
    }
    int loaded = 0;
    m_urlToRow.clear();
    m_dbidToRow.clear();
    m_zpidToRow.clear();
    // Add to spreadsheet
    int numRows = q.size();
    ui->tblData->setRowCount(numRows);
    qDebug() << "Got num rows:" << numRows << " cols:" << ui->tblData->columnCount();
    assertTableHeaders();
    int rowIndex = 0;
    while (q.next())
    {
        if (rowIndex >= numRows)
        {
            ui->tblData->setRowCount(numRows = rowIndex + 1);
        }
        /***
    id INTEGER PRIMARY KEY AUTOINCREMENT,\
    zpid INTEGER NOT NULL,\
    when_checked INTEGER NOT NULL,\
    detail_url TEXT NOT NULL,\
    lot_size TEXT,\
    year_built INTEGER,\
    price INTEGER,\
    bedrooms TEXT,\
    baths TEXT,\
    sqft INTEGER,\
    zestimate INTEGER,\
    schools TEXT,\
    schools_avg REAL,\
    hoa_fee TEXT,\
    address TEXT,\
    city TEXT,\
    county TEXT,\
    zip TEXT,\
    state TEXT,\
    user1 TEXT,\
         ****/
        QString sUrl(q.value("detail_url").toString());
        m_dbidToRow[q.value("id").toInt()] = rowIndex;
        m_zpidToRow[q.value("zpid").toInt()] = rowIndex;
        m_urlToRow[sUrl] = rowIndex;
        setTableCell(rowIndex, COL_URL, sUrl );
        setTableCell(rowIndex, COL_DBID, q.value("id").toString() );
        setTableCell(rowIndex, COL_LOT, sortableLotSize(q.value("lot_size").toString()) );
        setTableCell(rowIndex, COL_YEAR, q.value("year_built").toString() );
        setTableCell(rowIndex, COL_PRICE, q.value("price").toString() );
        setTableCell(rowIndex, COL_BR, q.value("bedrooms").toString() );
        setTableCell(rowIndex, COL_BA, q.value("baths").toString() );
        setTableCell(rowIndex, COL_SQFT, q.value("sqft").toString() );
        setTableCell(rowIndex, COL_ZESTIMATE, q.value("zestimate").toString() );
        setTableCell(rowIndex, COL_SCHOOLS, q.value("schools").toString() );
        setTableCell(rowIndex, COL_HOA, q.value("hoa_fee").toString() );
        setTableCell(rowIndex, COL_ADDRESS, q.value("address").toString() );
        setTableCell(rowIndex, COL_CITY, q.value("city").toString() );
        setTableCell(rowIndex, COL_COUNTY, q.value("county").toString() );
        setTableCell(rowIndex, COL_ZIP, q.value("zip").toString() );
        setTableCell(rowIndex, COL_STATE, q.value("state").toString() );
        QString sLat(q.value("latitude").toString());
        QString sLong(q.value("longitude").toString());
        if (!sLat.isEmpty() && !sLong.isEmpty() && sLat.toInt() != 0)
        {
            setTableCell(rowIndex, COL_GEO, QString().sprintf("%.6f,%.6f", sLat.toDouble() / 1000000.0, sLong.toDouble() / 1000000.0 ) );
            setDistanceColumn(rowIndex);
        }
        // Display user columns
        int col;
        for (col = COL_START_USER; col < ui->tblData->columnCount(); col++)
        {
            QString name(QString().sprintf("user%d", col - COL_START_USER + 1));
            setTableCell(rowIndex, col, q.value(name).toString() );
        }
        rowIndex++;
    }
    m_sheetLoadBusy = 0;
    return loaded;
}

int MainWindow::loadUsercols()
{
    if (!dbAvailable()) return -1;
    QSqlQuery q(m_db);
    int loaded = 0;
    m_userColMap.clear();
    m_userColCollation.clear();
    if (q.exec("SELECT name, user_name, collation FROM zbr_usercols ORDER BY name"))
    {
        QStringList lstH(columnHeaderList());
        qDebug() << "Asserting table headers (loadUsercols)";
        int n;
        const char *_tblFields[] = { PROPERTY_TABLE_FIELDS };
        for (n = 0; n < lstH.length(); n++)
        {
            if (!strncmp(_tblFields[n], "_nd", 3)) continue;
            if (!strncmp(_tblFields[n], "user", 4)) continue; // add these later
            emit addColumn( _tblFields[n], lstH[n] );
            //m_dlgWeight->addColumn( _tblFields[n], lstH[n] );
        }
        while (q.next())
        {
            QString nameLabel(q.value("name").toString());
            QString name(q.value("user_name").toString());
            int nameCollation = q.value("collation").toInt();
            if (nameLabel.isEmpty() || name.isEmpty()) continue;
            m_userColMap[nameLabel] = name;
            m_userColCollation[nameLabel] = nameCollation;
            emit addColumn(nameLabel, name);
            lstH << name;
            loaded++;
        }
        if (ui->tblData->columnCount() < COL_START_USER + loaded)
        {
            qDebug() << "Adding additional columns:" << loaded;
            ui->tblData->setColumnCount(COL_START_USER + loaded);
        }
        else if (loaded > 0) qDebug() << "Additional user cols:" << loaded;
        ui->tblData->setHorizontalHeaderLabels(lstH);
    }
    else qDebug() << "Column query failed:" << q.lastError();
    return loaded;
}

bool MainWindow::updateDbFromTable( int row, QString zzpid, double avgSchool, int latitude, int longitude )
{
    // Is this insert or update?
    QString dbid(safeItemText(row, COL_DBID));
    int id = -1;
    if (!dbid.isEmpty()) id = dbid.toInt();
    QSqlQuery q(m_db);
    bool success;
    if (id < 0)
    {
        success = q.prepare( "INSERT INTO zbr_property (zpid, when_checked, detail_url, lot_size, year_built, price, bedrooms, baths, sqft, "
                             "zestimate, schools, schools_avg, hoa_fee, address, city, county, state, zip, latitude, longitude) "
                             "VALUES (:zpid, :when_checked, :detail_url, :lot_size, :year_built, :price, :bedrooms, :baths, :sqft, "
                             ":zestimate, :schools, :schools_avg, :hoa_fee, :address, :city, :county, :state, :zip, :latitude, :longitude)" );
    }
    else
    {
        success = q.prepare( "UPDATE zbr_property SET zpid=:zpid, when_checked=:when_checked, detail_url=:detail_url, lot_size=:lot_size, year_built=:year_built, "
                             "price=:price, bedrooms=:bedrooms, baths=:baths, sqft=:sqft, zestimate=:zestimate, schools=:schools, schools_avg=:schools_avg, "
                             "hoa_fee=:hoa_fee, address=:address, city=:city, county=:county, state=:state, zip=:zip, latitude=:latitude, longitude=:longitude "
                             "WHERE id=:id" );
        if (success) q.bindValue(":id", id );
    }
    if (!success)
    {
        logMsg( "Prepare failed:" + q.lastError().text(), -1 );
        return false;
    }
    q.bindValue(":zpid", zzpid.toInt());
    int currentTime = QDateTime::currentDateTime().toTime_t();
    q.bindValue(":when_checked", currentTime);
    q.bindValue(":schools_avg", avgSchool);
    q.bindValue(":detail_url", safeItemText(row, COL_URL));
    q.bindValue(":lot_size", safeItemText(row, COL_LOT));
    q.bindValue(":year_built", safeItemText(row, COL_YEAR));
    q.bindValue(":price", safeItemText(row, COL_PRICE));
    q.bindValue(":bedrooms", safeItemText(row, COL_BR));
    q.bindValue(":baths", safeItemText(row, COL_BA));
    q.bindValue(":sqft", safeItemText(row, COL_SQFT));
    q.bindValue(":zestimate", safeItemText(row, COL_ZESTIMATE));
    q.bindValue(":schools", safeItemText(row, COL_SCHOOLS));
    q.bindValue(":hoa_fee", safeItemText(row, COL_HOA));
    q.bindValue(":address", safeItemText(row, COL_ADDRESS));
    q.bindValue(":city", safeItemText(row, COL_CITY));
    q.bindValue(":county", safeItemText(row, COL_COUNTY));
    q.bindValue(":state", safeItemText(row, COL_STATE));
    q.bindValue(":zip", safeItemText(row, COL_ZIP));
    q.bindValue(":latitude", latitude);
    q.bindValue(":longitude", longitude);
    if (!q.exec())
    {
        logMsg( "Query failed:" + q.lastError().text(), -1 );
        return false;
    }
    if (id < 0)
    {
        id = q.lastInsertId().toInt();
        qDebug() << "Got new db ID:" << id << "for row" << row;
        setTableCell( row, COL_DBID, QString().sprintf("%d",id) );
        m_dbidToRow[id] = row;
        m_zpidToRow[zzpid.toInt()] = row;
    }
    return true;
}

QString MainWindow::sortableLotSize( QString s )
{
    // We'll have 1 acres, 0.9 acres, 56,566 acres (ha ha), 2 acres, 10 acres and need to make it all sortable
    QRegExp rxAcreage("(.*) acres");
    if (!rxAcreage.exactMatch(s)) return s;
    // Look for "lazy agent didn't think units matter when it's sq. ft vs. acres"
    QRegExp rxBig("([0-9]+),([0-9]+.*)");
    if (rxBig.exactMatch(rxAcreage.cap(1)))
    {
        // Convert to sq. feet. Actual 1,000 acre properties need recognition here;
        // we're assuming that fat-fingered realtors are more common than huge properties
        // There are a few properties over 2000 acres in the midwest and west, but NUMEROUS
        // obviously incorrect. Most expensive found: 1200 acres in Gilroy CA for $25M
        // Biggest: around 2500 acres in Steamboat Springs CO (large amounts of shared leased land)
        // Some of the errors include typos where assuming square feet instead of acres
        // doesn't correct the error, and some just plain blatantly misleading values (like
        // giving the acreage of the entire development or community for a condo - yeah right)
        int bigDamnRanch = rxBig.cap(1).toInt() * 1000 + (int)rxBig.cap(2).toDouble();
        double dRanch = bigDamnRanch;
        if (bigDamnRanch >= 2600)
        {
            dRanch /= 43560.0;
        }
        return QString().sprintf("%5.2f", dRanch);
    }
    return QString().sprintf("%5.2f", rxAcreage.cap(1).toDouble());
}

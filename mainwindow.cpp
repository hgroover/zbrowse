#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "columnids.h"
#include "versioninfo.h"
#include "dlgusercolumns.h"

#include "xlsxdocument.h"
#include "xlsxchart.h"
#include "xlsxcellrange.h"

#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QWebView>
#include <QWebFrame>
#include <QWebPage>
#include <QRegExp>
#include <QObjectUserData>
#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QAudioOutput>
#include <QFileDialog>

// Define static tags for dependency checking

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle( windowTitle() + " v" ZBROWSE_VER );
    m_linkChooser = new DlgLinkChooser( this );
    m_linkChooser->showNormal();
    setFocus();
    saveRestore(true);
    m_loadBusy = 0;
    m_sheetLoadBusy = 0;
    m_pageProcessingPending = 0;
    connect( m_linkChooser, SIGNAL(fetchLink(QString,int,bool)), this, SLOT(fetchUrl(QString,int,bool)) );
    connect( this, SIGNAL(startPage()), m_linkChooser, SLOT(newPage()) );
    connect( this, SIGNAL(article(QString)), m_linkChooser, SLOT(newEntry(QString)) );
    connect( this, SIGNAL(endPage()), m_linkChooser, SLOT(endEntry()) );
    if (!dbAvailable())
    {
        logMsg( "Failed to assert DB", 0 );
    }
    else
    {
        loadSpreadsheet("price ASC");
    }
    m_pcmBeep1 = pcmWaveform(8000, 250, 440, 100, 40);
    m_pcmBeep2 = pcmWaveform(8000, 250, 880, 120, 120);
    m_pcmWarn1 = pcmWaveform(8000, 100, 440, 50, 100) + pcmWaveform(8000, 400, 110, 120, 35);
    m_format.setSampleRate(8000);
    m_format.setChannelCount(1);
    m_format.setSampleSize(8);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);

    m_audio = new QAudioOutput(m_format, this);
}

MainWindow::~MainWindow()
{
    delete m_linkChooser;
    m_linkChooser = NULL;
    delete ui;
}

void MainWindow::on_btnAddUrl_clicked()
{
    QString sUrl(ui->txtURL->text());
    if (sUrl.isEmpty()) return;
    fetchUrl(sUrl, -1, false);
}

void MainWindow::on_webView_loadFinished(bool arg1)
{
    // Note that we may be running afoul of this bug, addressed in Qt 5.3:
    // https://bugreports.qt-project.org/browse/QTBUG-35663?page=com.atlassian.streams.streams-jira-plugin:activity-stream-issue-tab
    if (m_loadBusy > 0) m_loadBusy--;
    if (!arg1)
    {
        startPlay(m_pcmWarn1);
        qDebug() << "Load failed";
        return;
    }
    qDebug() << "Load successful:" << ui->webView->url() << "pending:" << m_pageProcessingPending << "busy:" << m_loadBusy;
    if (!m_pageProcessingPending && !m_loadBusy)
    {
        m_pageProcessingPending++;
        QTimer::singleShot( 2500, this, SLOT(onIdleTimer()) );
        qDebug() << "Timer scheduled";
    }
}

// We won't get these events when javascript events are handling Web 2.0 requests
void MainWindow::on_webView_linkClicked(const QUrl &arg1)
{
    qDebug() << "Link clicked:" << arg1;
}

void MainWindow::on_btnRescrape_clicked()
{
    ui->txtDebug->appendPlainText("\n--- Rescrape:\n");
    ui->txtDebug->appendPlainText(ui->webView->page()->mainFrame()->toHtml());
    //on_webView_loadFinished(true);
    processWebView();
}

void MainWindow::on_btnZillow_clicked()
{
    ui->webView->setUrl(QUrl("http://zillow.com"));
    ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::on_btnFetch_clicked()
{
    // Get selected row
    QList<QTableWidgetItem*> lst(ui->tblData->selectedItems());
    if (lst.isEmpty()) return;
    int n;
    for (n = 0; n < lst.length(); n++)
    {
        if (lst[n]->column()!=COL_URL) continue;
        fetchUrl(lst[n]->text(), lst[n]->row(), true);
        qDebug() << "Got url from row" << lst[n]->row();
        break;
    }
}

void MainWindow::on_tblData_itemSelectionChanged()
{
    bool enableButtons = (ui->tblData->selectedItems().count() > 0);
    ui->btnFetch->setEnabled( enableButtons );
    ui->btnDelete->setEnabled( enableButtons );
}

void MainWindow::on_webView_customContextMenuRequested(const QPoint &pos)
{
    qDebug() << "Custom context menu:" << pos;
}

void MainWindow::fetchUrl( QString url, int tableIndex, bool fetch )
{
    // Create a row for it
    if (tableIndex < 0)
    {
        tableIndex = ui->tblData->rowCount();
        ui->tblData->setRowCount(tableIndex+1);
        assertTableHeaders();
    }
    ui->tblData->setItem(tableIndex, 0, new QTableWidgetItem(url));
    // Set in map
    /****
    if (m_urlToRow.contains(url))
    {
        // Already exists
        m_urlToRow[url] = tableIndex;
        ui->webView->setUrl(QUrl(url));
        return;
    }
    ***/
    m_urlToRow[url] = tableIndex;
    // Set webview to url in fetch
    if (fetch)
    {
        ui->webView->setUrl(QUrl(url));
    }
}

void MainWindow::on_webView_loadStarted()
{
    qDebug() << "Load started:" << ui->webView->url();
    m_loadBusy++;
}

void MainWindow::on_btnQuit_clicked()
{
    saveRestore(false);
    close();
}

// Log with specified priority level (-1 = error, 0 = critical, 7 = debug)
void MainWindow::logMsg( QString msg, int priority )
{
    qDebug() << "log[" << priority << "]:" << msg;
}

QString MainWindow::safeItemText( int row, int col )
{
    QTableWidgetItem *item = ui->tblData->item(row, col);
    if (NULL==item) return QString();
    return item->text();
}

QStringList MainWindow::columnHeaderList()
{
    QStringList lstH;
    lstH << "URL";
    lstH << "db ID";
    lstH << "Lot";
    lstH << "Year";
    lstH << "Price";
    lstH << "BR";
    lstH << "Bath";
    lstH << "Sq.Ft";
    lstH << "Zestimate";
    lstH << "Schools";
    lstH << "HOA";
    lstH << "Address";
    lstH << "City";
    lstH << "County";
    lstH << "Zip";
    lstH << "State";
    lstH << "Geo";
    lstH << "Miles";

    return lstH;
}

void MainWindow::assertTableHeaders()
{
    if (ui->tblData->columnCount() == 0)
    {
        // Set up headers
        QStringList lstH(columnHeaderList());
        // FIXME add user columns to headings
        ui->tblData->setColumnCount(lstH.length());
        ui->tblData->setHorizontalHeaderLabels(lstH);
    }
    // Reset width first time only
    static bool firstTime = true;
    if (firstTime)
    {
        firstTime = false;
        while (m_columnWidths.length() < ui->tblData->columnCount()) m_columnWidths.push_back(-1);
        // Set default column widths
        ui->tblData->setColumnWidth(COL_DBID, 40);
        ui->tblData->setColumnWidth(COL_YEAR, 45);
        ui->tblData->setColumnWidth(COL_PRICE, 55);
        ui->tblData->setColumnWidth(COL_ZESTIMATE, 60);
        ui->tblData->setColumnWidth(COL_SQFT, 40);
        ui->tblData->setColumnWidth(COL_LOT, 60);
        ui->tblData->setColumnWidth(COL_SCHOOLS, 100);
        ui->tblData->setColumnWidth(COL_ADDRESS, 120);
        ui->tblData->setColumnWidth(COL_BR, 40);
        ui->tblData->setColumnWidth(COL_BA, 40);
        ui->tblData->setColumnWidth(COL_HOA, 40);
        ui->tblData->setColumnWidth(COL_ZIP, 40);
        ui->tblData->setColumnWidth(COL_STATE, 40);
        ui->tblData->setColumnWidth(COL_GEO, 50);
        ui->tblData->setColumnWidth(COL_DISTANCE, 45);
        int n;
        // Set column widths previously saved
        for (n = 0; n < m_columnWidths.length(); n++)
        {
            if (m_columnWidths[n].toInt() <= 0) continue;
            ui->tblData->setColumnWidth(n, m_columnWidths[n].toInt());
        }
        qDebug() << "Default column width:" << ui->tblData->columnWidth(COL_URL);
    }
}

void MainWindow::on_tblData_clicked(const QModelIndex &index)
{
    qDebug() << "Clicked:" << index;
}

void MainWindow::on_tblData_cellDoubleClicked(int row, int column)
{
    // Fetch if url
    if (column != COL_URL)
    {
        qDebug() << "Ignoring doubleclick:" << column;
        return;
    }
    qDebug() << "Double-click fetch row:" << row;
    fetchUrl(safeItemText(row, column), row, true);
}

void MainWindow::on_pushButton_clicked()
{
    qDebug() << "Sound test:";
    static int cycle = 0;
    switch (cycle++)
    {
    case 0:
        qDebug() << "beep 1";
        startPlay(m_pcmBeep1);
        break;
    case 1:
        qDebug() << "beep2";
        startPlay(m_pcmBeep2);
        break;
    default:
        qDebug() << "warning beep";
        startPlay(m_pcmWarn1);
        cycle = 0;
        break;
    }

}

void MainWindow::on_btnColumns_clicked()
{
    if (!dbAvailable()) return;
    DlgUserColumns dlgColumns(m_db, this);
    connect( &dlgColumns, SIGNAL(userColumnsChanged()), this, SLOT(loadUsercols()) );
    int res = dlgColumns.exec();
    qDebug() << "result:" << res;
}

void MainWindow::on_tblData_cellChanged(int row, int column)
{
    if (m_sheetLoadBusy > 0) return;
    if (column < COL_START_USER) return;
    int userIndex = column - COL_START_USER;
    QString name(QString().sprintf("user%d", userIndex + 1));
    if (!dbAvailable()) return;
    int value = ui->tblData->item(row, column)->text().toInt();
    int dbId = safeItemText(row, COL_DBID).toInt();
    if (!dbId) return;
    QSqlQuery q(m_db);
    if (!q.prepare("UPDATE zbr_property SET " + name + " = :val WHERE id = :id"))
    {
        qDebug() << "Prepare failed:" << q.lastError();
        return;
    }
    //q.bindValue(":name", name);
    q.bindValue(":id", dbId);
    q.bindValue(":val", value );
    if (!q.exec())
    {
        qDebug() << "exec failed:" << q.lastError();
    }
}

void MainWindow::on_btnSaveXls_clicked()
{
    // Get name to save as
    QString fileName(QFileDialog::getSaveFileName( this, "Save as xlsx file", QString(), "XLSX files (*.xlsx)" ));
    if (fileName.isEmpty()) return;
    qDebug() << "Saving as" << fileName;
    QXlsx::Document xlsx;
    xlsx.addSheet("Summary");
    int n, row;
    int cols = ui->tblData->columnCount();
    QXlsx::Format hdrFormat;
    hdrFormat.setFontBold(true);
    for (n = 0; n < ui->tblData->horizontalHeader()->count(); n++)
    {
        xlsx.write(1, n+1, ui->tblData->horizontalHeaderItem(n)->text(), hdrFormat);
    }
    // Set widths
    xlsx.setColumnWidth(COL_DBID + 1, 5.86);
    xlsx.setColumnWidth(COL_LOT + 1, 6.86);
    xlsx.setColumnWidth(COL_YEAR + 1, 5.7);
    xlsx.setColumnWidth(COL_BR + 1, 4.3);
    xlsx.setColumnWidth(COL_BA + 1, 4.5);
    xlsx.setColumnWidth(COL_SQFT + 1, 6.3);
    xlsx.setColumnWidth(COL_SCHOOLS + 1, 12.5);
    xlsx.setColumnWidth(COL_HOA + 1, 4.5);
    xlsx.setColumnWidth(COL_ADDRESS + 1, 19);
    xlsx.setColumnWidth(COL_CITY + 1, 12 );
    xlsx.setColumnWidth(COL_STATE + 1, 5.86 );
    xlsx.setColumnWidth(COL_ZIP + 1, 7);
    xlsx.setColumnWidth(COL_GEO + 1, 4.8);
    for (row = 0; row < ui->tblData->rowCount(); row++)
    {
        for (n = 0; n < cols; n++)
        {
            QTableWidgetItem *i = ui->tblData->item(row, n);
            int sheetRow = row + 2;
            if (i!=NULL)
            {
                switch (n)
                {
                case COL_URL:
                    xlsx.write( sheetRow, n + 1, i->text() );
                    break;
                case COL_DBID:
                    xlsx.write( sheetRow, n + 1, i->text().toInt() );
                    break;
                case COL_LOT:
                case COL_DISTANCE:
                    xlsx.write( sheetRow, n + 1, i->text().toDouble() );
                    break;
                case COL_YEAR:
                case COL_PRICE:
                case COL_BR:
                case COL_BA:
                case COL_ZESTIMATE:
                case COL_SQFT:
                case COL_ZIP:
                {
                    QString s(i->text());
                    if (!s.isEmpty())
                        xlsx.write( sheetRow, n + 1, s.toInt() );
                }
                    break;
                case COL_SCHOOLS:
                case COL_HOA:
                case COL_ADDRESS:
                case COL_CITY:
                case COL_COUNTY:
                case COL_STATE:
                case COL_GEO:
                    xlsx.write( sheetRow, n + 1, i->text() );
                    break;
                default:
                    xlsx.write( sheetRow, n + 1, i->text().toInt() );
                    break;
                }

            }
        }
    }

    xlsx.saveAs(fileName);
}

void MainWindow::on_btnDelete_clicked()
{
    if (!dbAvailable()) return;
    // Delete selected records
    // Get selected rows
    QList<QTableWidgetItem*> lst(ui->tblData->selectedItems());
    if (lst.isEmpty()) return;
    int n;
    QString sQuery( "DELETE FROM zbr_property WHERE id IN (" );
    QString sSep("");
    int lastRow = -1;
    int deleteCount = 0;
    QList<int> rowList;
    for (n = 0; n < lst.length(); n++)
    {
        if (lst[n]->row()==lastRow) continue;
        lastRow = lst[n]->row();
        QTableWidgetItem *i = ui->tblData->item(lastRow, COL_DBID);
        if (NULL==i) continue;
        sQuery += sSep;
        sSep = ",";
        deleteCount++;
        sQuery += i->text();
        rowList << lastRow;
    }
    if (deleteCount > 0)
    {
        sQuery += ')';
        qDebug() << "deleting" << deleteCount << "entries:" << sQuery;
        QSqlQuery q(m_db);
        if (!q.exec(sQuery))
        {
            logMsg( "Query failed:" + q.lastError().text() );
        }
        else
        {
            logMsg( QString().sprintf("%d rows affected", q.numRowsAffected()) );
            // Remove rows in reverse order
            for (n = rowList.length() - 1; n >= 0; n--)
            {
                ui->tblData->removeRow(rowList[n]);
            }
        }
    }
}

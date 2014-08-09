#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "columnids.h"

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_linkChooser = new DlgLinkChooser( this );
    m_linkChooser->showNormal();
    setFocus();
    saveRestore(true);
    m_loadBusy = 0;
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
    ui->btnFetch->setEnabled( ui->tblData->selectedItems().count() > 0 );
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

void MainWindow::assertTableHeaders()
{
    if (ui->tblData->columnCount() == 0)
    {
        // Set up headers
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
        lstH << "Dist";
        // FIXME add user columns to headings
        ui->tblData->setColumnCount(lstH.length());
        ui->tblData->setHorizontalHeaderLabels(lstH);
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

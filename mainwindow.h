#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QtSql>
#include <QAudio>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QFile>
#include <QVariant>

#include "dlglinkchooser.h"
#include "dlgweight.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

   static QStringList columnHeaderList();

private slots:
    void on_btnAddUrl_clicked();

    void on_webView_loadFinished(bool arg1);

    void on_webView_linkClicked(const QUrl &arg1);

    void on_btnRescrape_clicked();

    void on_btnZillow_clicked();

    void on_btnFetch_clicked();

    void on_tblData_itemSelectionChanged();

    void on_webView_customContextMenuRequested(const QPoint &pos);

    void on_webView_loadStarted();

    void on_btnQuit_clicked();

    void on_tblData_clicked(const QModelIndex &index);

    void on_tblData_cellDoubleClicked(int row, int column);

    void on_pushButton_clicked();

    void on_btnColumns_clicked();

    void on_tblData_cellChanged(int row, int column);

    void on_btnSaveXls_clicked();

    void on_btnDelete_clicked();

    void on_btnWeight_clicked();

public slots:
    void fetchUrl( QString url, int tableIndex, bool fetch );
    void saveRestore( bool isLoading );
    // Log with specified priority level (-1 = error, 0 = critical, 7 = debug)
    void logMsg( QString msg, int priority = 3 );
    void setTableCell( int row, int col, QString value );

    // --- in parsefns.cpp ---
    void processWebView();
    void onIdleTimer();
    void setDistanceColumn(int rowId);
    // --- end parsefns.cpp ---

signals:
    void startPage();
    void endPage();
    void article(QString articleBody);
    void addColumn(QString dbColumn, QString displayColumn);
    void cellChanged( int row, int col, QString value );
    void closing();

private:
    Ui::MainWindow *ui;
    DlgLinkChooser *m_linkChooser;
    DlgWeight *m_dlgWeight;

    // Rebuilt whenever reading from database
    QMap<QString,int> m_urlToRow; // full URL (e.g. http://www.zillow.com/details/...) to origin:0 row index
    QMap<int,int> m_zpidToRow; // Zillow id to row index
    QMap<int,int> m_dbidToRow; // Database ID to row index

    // True if close already requested
    bool m_closeRequested;

    // Column widths in table widget set while running
    QList<QVariant> m_columnWidths;

    // Map user columns (user1, user2, etc) to user-defined names and collation values. If not present, not used.
    QMap<QString,QString> m_userColMap;
    QMap<QString,int> m_userColCollation;

    // Queued entries to fetch (by table row index) - used only if fetching more than one entry
    QList<int> m_fetchQueue;

    // Non-zero when load is in progress
    int m_loadBusy;
    // Page processing pending - non-zero when onTimerIdle() is already scheduled to process page
    int m_pageProcessingPending;

    // Non-zero when loading spreadsheet
    int m_sheetLoadBusy;

    // SQLite database connection
    QString m_dbPath;
    QSqlDatabase m_db;
    int m_dbVer;

    //-- Defined in sqlfns.cpp ---
    bool dbAvailable();
    bool dbVersionOk();
    bool dbSingleUpdate( unsigned int fromTo );
    QString dbFilePath();
    void dbReset();
    bool dbAssertTables();
    bool dbInsertVersion();
    int loadSpreadsheet(QString orderBy);
protected slots:
    int loadUsercols();
    void processQueue();
private:
    bool updateDbFromTable( int row, QString zzpid, double avgSchool, int latitude, int longitude );
    QString sortableLotSize( QString s );
    //-- end sqlfns.cpp ---

    QByteArray m_pcmBeep1;
    QByteArray m_pcmBeep2;
    QByteArray m_pcmWarn1;
    QAudioFormat m_format;
    QAudioOutput *m_audio;
    QBuffer m_audioData;

    //-- Defined in audio.cpp --
    QByteArray pcmWaveform( int rateHz, int durationMs, int freqHz, int amplitudeStart, int amplitudeEnd ); // Generate 8-bit PCM waveform (simple sine wave)
    void startPlay( QByteArray& pcm );
protected slots:
    void handleStateChanged(QAudio::State newState);
    void testLinkHandler(QString cmd);
private:
    //-- end audio.cpp --

    void assertTableHeaders();
    QString safeItemText( int row, int col );
};

#endif // MAINWINDOW_H

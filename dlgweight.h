#ifndef DLGWEIGHT_H
#define DLGWEIGHT_H

#include <QDialog>
#include <QTableWidget>
#include <QMap>
#include <QtSql>

namespace Ui {
class DlgWeight;
}

class MainWindow;

class DlgWeight : public QDialog
{
    Q_OBJECT

public:
    explicit DlgWeight(QWidget *parent);
    ~DlgWeight();

    void initDb( QSqlDatabase& db );
    void loadDbValues();

public slots:
    void addColumn( QString dbColumn, QString displayColumn );
    void rebuildDisplay();
    void cellChanged( int row, int col, QString value );

private slots:
    void on_tblWeights_cellChanged(int row, int column);

    void on_tblWeightedRanking_cellDoubleClicked(int row, int column);

    void on_pushButton_clicked();

    void on_btnHide_clicked();

    void on_btnRefresh_clicked();

    void on_tblWeightedRanking_doubleClicked(const QModelIndex &index);

    void on_btnSaveXlsx_clicked();

    void on_lblInfo_linkActivated(const QString &link);

signals:
    void navigateTo( int dbId, bool loadPage );

private:
    Ui::DlgWeight *ui;
    MainWindow *m_main;
    QMap<QString,QString> m_displayToDb;
    QMap<QString,int> m_dbToRow;
    QMap<QString,int> m_dbExists;
    QMap<QString,double> m_dbToWeight;
    QMap<QString,int> m_dbToTransform;
    QMap<QString,QString> m_dbToLabel;
    QMap< int, QMap<int,QString> > m_tableData;
    int m_addActive;
    QSqlDatabase *m_db;

    QString safeTextItem( int row, int col );
    double safeFloatItem( int row, int col );
    int safeIntItem( int row, int col );

    // Rebuild lblInfo content with or without refresh message
    void rebuildInfo( bool withRefresh );
};

#endif // DLGWEIGHT_H

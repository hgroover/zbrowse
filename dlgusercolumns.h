#ifndef DLGUSERCOLUMNS_H
#define DLGUSERCOLUMNS_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMap>

namespace Ui {
class DlgUserColumns;
}

class DlgUserColumns : public QDialog
{
    Q_OBJECT

public:
    explicit DlgUserColumns(QSqlDatabase& db, QWidget *parent = 0);
    ~DlgUserColumns();

signals:
    int userColumnsChanged();

private slots:
    void on_btnOK_clicked();

    void on_pushButton_clicked();

private:
    Ui::DlgUserColumns *ui;
    QSqlDatabase& m_db;
    QMap<int,QString> m_orgValues;
};

#endif // DLGUSERCOLUMNS_H

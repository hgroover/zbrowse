#include "dlgusercolumns.h"
#include "ui_dlgusercolumns.h"

#include "columnids.h"
#include "sqldefs.h"

#include <QtSql>
#include <QSqlQuery>
#include <QRegExp>
#include <QStringList>
#include <QTableWidgetItem>

DlgUserColumns::DlgUserColumns(QSqlDatabase& db, QWidget *parent) :
    QDialog(parent),
    m_db(db),
    ui(new Ui::DlgUserColumns)
{
    //m_db = db;
    ui->setupUi(this);
    if (!m_db.isOpen())
    {
        close();
        return;
    }
    ui->tblColumns->setColumnCount(2);
    QStringList lstH;
    lstH << "Name";
    lstH << "Collation";
    ui->tblColumns->setHorizontalHeaderLabels(lstH);
    ui->tblColumns->setRowCount(10);
    QSqlQuery q(m_db);
    int loaded = 0;
    if (q.exec("SELECT name, user_name, collation FROM zbr_usercols ORDER BY name"))
    {
        QRegExp rxUser("user(\\d+)");
        while (q.next())
        {
            QString nameLabel(q.value("name").toString());
            QString name(q.value("user_name").toString());
            qDebug() << "Name:" << nameLabel << " label:" << name;
            int nameCollation = q.value("collation").toInt();
            if (nameLabel.isEmpty() || name.isEmpty()) continue;
            if (!rxUser.exactMatch(nameLabel))
            {
                qDebug() << "No match:" << nameLabel;
                continue;
            }
            int row = rxUser.cap(1).toInt() - 1;
            if (row >= ui->tblColumns->rowCount())
            {
                qDebug() << "Out of range:" << row;
                continue;
            }
            ui->tblColumns->setItem(row, 0, new QTableWidgetItem(name));
            ui->tblColumns->setItem(row, 1, new QTableWidgetItem(q.value("collation").toInt()));
            m_orgValues[row] = name;
            loaded++;
        }
        qDebug() << "number loaded:" << loaded;
    }
    else
    {
        qDebug() << "Query failed:" << q.lastError();
    }
}

DlgUserColumns::~DlgUserColumns()
{
    delete ui;
}

void DlgUserColumns::on_btnOK_clicked()
{
    // Save and exit
    int row;
    QSqlQuery q;
    bool isUpdate;
    for (row = 0; row < ui->tblColumns->rowCount(); row++)
    {
        QTableWidgetItem *item = ui->tblColumns->item(row, 0);
        if (NULL == item) continue;
        QString userName( item->text() );
        if (userName.isEmpty()) continue;
        isUpdate = m_orgValues.contains(row);
        if (isUpdate)
        {
            // FIXME also check for changes in collation
            if (m_orgValues[row] == userName) continue;
            if (!q.prepare("UPDATE zbr_usercols SET user_name=:username WHERE name=:name"))
            {
                qDebug() << "Query failed prep:" << q.lastError();
                continue;
            }
        }
        else
        {
            if (!q.prepare("INSERT INTO zbr_usercols (name, user_name, collation) VALUES (:name, :username, :coll)"))
            {
                qDebug() << "Query failed prep:" << q.lastError();
                continue;
            }
        }
        q.bindValue(":name", QString().sprintf("user%d", row+1));
        q.bindValue(":username", userName);
        q.bindValue(":coll", 1);
        if (!q.exec())
        {
            qDebug() << "Query exec failed:" << (isUpdate ? " (update)" : " (insert)") << q.lastError();
        }
    }
    close();
    setResult(1);
    // Emit signal if actual changes made
    emit userColumnsChanged();
}

void DlgUserColumns::on_pushButton_clicked()
{
    // Exit without saving
    close();
}

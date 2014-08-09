#include "dlglinkchooser.h"
#include "ui_dlglinkchooser.h"

#include <QDebug>
#include <QStringList>

DlgLinkChooser::DlgLinkChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgLinkChooser)
{
    ui->setupUi(this);
}

DlgLinkChooser::~DlgLinkChooser()
{
    delete ui;
}

void DlgLinkChooser::on_textBrowser_anchorClicked(const QUrl &arg1)
{
    qDebug() << "Text browser:" << arg1;
    emit fetchLink( arg1.toString(), -1, true );
}

void DlgLinkChooser::newPage()
{
    ui->textBrowser->clear();
    ui->textBrowser->setText("<h3>House list</h3>\n");
    m_links.clear();
}

void DlgLinkChooser::newEntry(QString articleBody)
{
    qDebug() << "NewEntry:" << articleBody;
    QStringList a(articleBody.split('\n'));
    QString sText(a[1]);
    if (sText.isEmpty()) sText = a[0];
    QStringList aGeo;
    int dbid = -1;
    int rowIndex = -1;
    if (a.length() > 3)
    {
        aGeo = a[3].split(' ');
        if (aGeo.length() >= 4)
        {
            dbid = aGeo[2].toInt();
            rowIndex = aGeo[3].toInt();
        }
    }
    QString sLink("http://www.zillow.com" + a[0]);
    if (-1 == dbid && -1 == rowIndex)
    {
        m_links.append("<a href=\"" + sLink + "\" title=\"Add this house to table\">" + a[1] + "</a> " + a[2]);
    }
    else
    {
        m_links.append("<a href=\"" + sLink + "\" title=\"Existing house @" + a[1] + "\">(existing)</a>" );
    }
    //ui->textBrowser->append("<a href=\"" + sLink + "\" title=\"Process this\">" + a[1] + "</a> | ");
}

void DlgLinkChooser::endEntry()
{
    ui->textBrowser->append( m_links.join("<br/>") );
    //ui->textBrowser->append("end of list");
}


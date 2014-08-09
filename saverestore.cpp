#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QString>


void MainWindow::saveRestore( bool isLoading )
{
    QSettings set("Zillow Community", "Zillow Browser");
    if (isLoading)
    {
        restoreGeometry( set.value("main.geometry").toByteArray() );
        m_linkChooser->restoreGeometry( set.value("link.geometry").toByteArray() );
        ui->chkShowDistance->setChecked( set.value("show.distance", true).toBool() );
        ui->txtReference->setText( set.value("geo.origin", "32.9275,-96.752343").toString() );
    }
    else
    {
        set.setValue("main.geometry", saveGeometry() );
        set.setValue("link.geometry", m_linkChooser->saveGeometry() );
        set.setValue("show.distance", ui->chkShowDistance->isChecked() );
        set.setValue("geo.origin", ui->txtReference->text() );
    }
}


#include "dlgweight.h"
#include "ui_dlgweight.h"

#include "mainwindow.h"
#include "columnids.h"
#include "sqldefs.h"

#include "xlsxdocument.h"
#include "xlsxchart.h"
#include "xlsxcellrange.h"

#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlResult>
#include <QDesktopServices>
#include <QFileDialog>

DlgWeight::DlgWeight(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgWeight)
{
    ui->setupUi(this);
    m_main = (MainWindow *) parent;
    QStringList lblH;
    lblH << "Column";
    lblH << "Weight";
    lblH << "Transform";
    ui->tblWeights->setColumnCount(lblH.length());
    m_addActive = 1;
    ui->tblWeights->setHorizontalHeaderLabels(lblH);
    m_addActive = 0;
    m_db = NULL;
}

DlgWeight::~DlgWeight()
{
    delete ui;
}

void DlgWeight::initDb( QSqlDatabase& db )
{
    m_db = &db;
}

void DlgWeight::on_tblWeights_cellChanged(int row, int column)
{
    if (0 != m_addActive) return;
    // Apply edits and save to db
    //qDebug() << "Cell changed:" << row << "," << column;
    if (column == 0) return;
    QString sLabel(ui->tblWeights->item(row, 0)->text());
    if (!m_displayToDb.contains(sLabel)) return;
    QString sColname(m_displayToDb[sLabel]);
    QString sWeight, sTransform;
    double dWeight;
    int nTransform;
    bool isUpdate = m_dbExists.contains(sColname);
    if (isUpdate) isUpdate = (m_dbExists[sColname] != 0);
    QSqlQuery q(*m_db);
    switch (column)
    {
    case 1:
        sWeight = ui->tblWeights->item(row, column)->text();
        dWeight = sWeight.toDouble();
        m_dbToWeight[sColname] = dWeight;
        if (isUpdate)
        {
            if (!q.prepare("UPDATE zbr_weighting SET weight=:weight WHERE colname=:colname"))
            {
                qDebug() << "Prepare failure:" << q.lastError().text();
                return;
            }
            q.bindValue(":weight", dWeight);
        }
        else
        {
            if (!q.prepare("INSERT INTO zbr_weighting (colname, transform, weight) VALUES (:colname, :transform, :weight)"))
            {
                qDebug() << "Prepare (insert) failed:" << q.lastError().text();
                return;
            }
            q.bindValue(":weight", dWeight);
            q.bindValue(":transform", 1);
        }
        q.bindValue(":colname", sColname);
        break;
    case 2:
        sTransform = ui->tblWeights->item(row, column)->text();
        nTransform = sTransform.toInt();
        m_dbToTransform[sColname] = nTransform;
        if (isUpdate)
        {
            if (!q.prepare("UPDATE zbr_weighting SET transform=:transform WHERE colname=:colname"))
            {
                qDebug() << "Prep transform failed:" << q.lastError().text();
                return;
            }
            q.bindValue(":transform", nTransform);
        }
        else
        {
            if (!q.prepare("INSERT INTO zbr_weighting (colname, transform, weight) VALUES (:colname, :transform, :weight)"))
            {
                qDebug() << "Prep transform (insert) failed:" << q.lastError().text();
                return;
            }
            q.bindValue(":weight", "0");
            q.bindValue(":transform", nTransform);
        }
        q.bindValue(":colname", sColname);
        break;
    default:
        qDebug() << "Unknown column" << column;
        return;
    }
    if (!q.exec())
    {
        qDebug() << "Query failed:" << q.lastError().text();
        return;
    }
    m_dbExists[sColname] = 1;
    //rebuildDisplay();
    rebuildInfo(true);
}

void DlgWeight::on_tblWeightedRanking_cellDoubleClicked(int row, int column)
{
    // Navigate in main and load page
}

void DlgWeight::on_pushButton_clicked()
{
}

void DlgWeight::on_btnHide_clicked()
{
    // Hide this dialog
    hide();
}

void DlgWeight::addColumn( QString dbColumn, QString displayColumn )
{
    int row = ui->tblWeights->rowCount();
    qDebug() << "Adding  row" << row << "db" << dbColumn << "lbl" << displayColumn;
    m_addActive++;
    ui->tblWeights->setRowCount(row + 1);
    ui->tblWeights->setItem( row, 0, new QTableWidgetItem(displayColumn) );
    ui->tblWeights->setItem( row, 1, new QTableWidgetItem("0") );
    ui->tblWeights->setItem( row, 2, new QTableWidgetItem("1") );
    m_displayToDb[displayColumn] = dbColumn;
    m_dbToRow[dbColumn] = row;
    m_dbToWeight[dbColumn] = 0;
    m_dbToTransform[dbColumn] = 1;
    m_addActive--;
}

void DlgWeight::loadDbValues()
{
    if (NULL == m_db) return;
    QSqlQuery q(*m_db);
    if (!q.exec("SELECT colname, user_name, transform, weight FROM zbr_weighting w LEFT JOIN zbr_usercols u ON (w.colname=u.name)"))
    {
        qDebug() << "QUery failed:" << q.lastError().text();
        return;
    }
    while (q.next())
    {
        // Do we have it?
        QString sColname(q.value("colname").toString());
        if (!m_dbToRow.contains(sColname)) continue;
        int row = m_dbToRow[sColname];
        QString sWeight(q.value("weight").toString());
        QString sTransform(q.value("transform").toString());
        ui->tblWeights->setItem(row, 1, new QTableWidgetItem(sWeight));
        ui->tblWeights->setItem(row, 2, new QTableWidgetItem(sTransform));
        m_dbExists[sColname] = 1;
        m_dbToWeight[sColname] = sWeight.toDouble();
        m_dbToTransform[sColname] = sTransform.toInt();
        m_dbToLabel[sColname] = q.value("user_name").toString();
    }
}

void DlgWeight::rebuildDisplay()
{
    if (!isVisible()) return;
    if (!m_tableData.contains(0))
    {
        qDebug() << "No table data (0)";
        return;
    }
    if (!m_tableData[0].contains( COL_START_USER - 1))
    {
        qDebug() << "Table not yet populated";
        return;
    }
    qDebug() << "Rebuilding display: row count:" << m_tableData.count();
    // Get row data from main
    ui->tblWeightedRanking->clear();
    ui->tblWeightedRanking->setRowCount(m_tableData.count());
    // Display columns
    int columnCount = 0;
    int n;
    // Find highest-numbered user column
    for (n = 0; n < 9; n++) if (m_tableData[0].contains(n+COL_START_USER)) columnCount = n+1; // Number of user columns
    ui->tblWeightedRanking->setColumnCount( columnCount + 13 );
    qDebug() << "User column count:" << columnCount;
    QStringList lstH;
    lstH << "Rank";
    lstH << "Price";
    lstH << "BR";
    lstH << "BA";
    lstH << "SqFt";
    lstH << "Lot";
    lstH << "Year";
    lstH << "Schools";
    lstH << "Distance";
    for (n = 1; n < 10; n++)
    {
        QString sUser( QString().sprintf("user%d", n) );
        if (!m_dbToLabel.contains(sUser)) break;
        lstH << m_dbToLabel[sUser];
        ui->tblWeightedRanking->setColumnWidth(9 + n - 1, 56);
    }
    lstH << "Address";
    lstH << "City";
    lstH << "State";
    lstH << "Zip";
    ui->tblWeightedRanking->setHorizontalHeaderLabels(lstH);
    ui->tblWeightedRanking->setColumnWidth(0, 56);
    ui->tblWeightedRanking->setColumnWidth(1, 64);
    ui->tblWeightedRanking->setColumnWidth(2, 42);
    ui->tblWeightedRanking->setColumnWidth(3, 42);
    ui->tblWeightedRanking->setColumnWidth(4, 56);
    ui->tblWeightedRanking->setColumnWidth(5, 56);
    ui->tblWeightedRanking->setColumnWidth(6, 56);
    ui->tblWeightedRanking->setColumnWidth(8, 56);
    ui->tblWeightedRanking->setColumnWidth(9 + columnCount + 0, 120);
    ui->tblWeightedRanking->setColumnWidth(9 + columnCount + 1,  80);
    ui->tblWeightedRanking->setColumnWidth(9 + columnCount + 2,  48);
    ui->tblWeightedRanking->setColumnWidth(9 + columnCount + 3,  56);
    int row;
    for (row = 0; row < ui->tblWeightedRanking->rowCount(); row++)
    {
        // Build rank value with transforms
        double rank = 0.0;
        //  0     1     2   3    4     5        6       7       8           9..?
        // Rank, price, br, ba, sqft, lot, year built, schools, distance, user columns, address, city, state, zip
        QString sPrice(safeTextItem(row, COL_PRICE));
        int nPrice = sPrice.toInt();
        // FIXME figure out a transformation to apply to price ranking
        ui->tblWeightedRanking->setItem(row, 1, new QTableWidgetItem(sPrice));

        QString sBr(safeTextItem(row, COL_BR));
        ui->tblWeightedRanking->setItem(row, 2, new QTableWidgetItem(sBr));

        QString sBath(safeTextItem(row, COL_BA));
        ui->tblWeightedRanking->setItem(row, 3, new QTableWidgetItem(sBath));

        QString sSqft(safeTextItem(row, COL_SQFT));
        ui->tblWeightedRanking->setItem(row, 4, new QTableWidgetItem(sSqft));

        QString sLot(safeTextItem(row, COL_LOT));
        ui->tblWeightedRanking->setItem(row, 5, new QTableWidgetItem(sLot));

        QString sYear(safeTextItem(row, COL_YEAR));
        ui->tblWeightedRanking->setItem(row, 6, new QTableWidgetItem(sYear));

        QString sSchools(safeTextItem(row, COL_SCHOOLS));
        if (m_dbToWeight.contains("schools"))
        {
            QStringList asch(sSchools.split(QRegExp("\\s+")));
            if (asch.length()>0)
            {
                double dSchools = asch[0].toDouble();
                qDebug() << "schools:" << sSchools << "toFLoat:" << dSchools;
                double dWeight = m_dbToWeight["schools"];
                rank += (dWeight * dSchools);
            }
        }
        ui->tblWeightedRanking->setItem(row, 7, new QTableWidgetItem(sSchools));

        QString sDistance(safeTextItem(row, COL_DISTANCE));
        ui->tblWeightedRanking->setItem(row, 8, new QTableWidgetItem(sDistance));

        int col;
        int localCol = 9;
        for (col = 0 ; col < columnCount; col++, localCol++)
        {
            QString sDBCol(QString().sprintf("user%d", col + 1));
            QString sValue(safeTextItem(row, COL_START_USER + col));
            ui->tblWeightedRanking->setItem(row, localCol, new QTableWidgetItem(sValue));
            int nValue = sValue.toInt();
            if (nValue == 0) continue;
            if (!m_dbToWeight.contains(sDBCol)) continue;
            if (!m_dbToTransform.contains(sDBCol)) continue;
            double weight = m_dbToWeight[sDBCol];
            int transform = m_dbToTransform[sDBCol];
            if (weight == 0.0) continue;
            // FIXME handle other transforms
            rank += (nValue * weight);
        }

        // Finally, address, city, state, zip
        QString sAddress(safeTextItem(row, COL_ADDRESS));
        ui->tblWeightedRanking->setItem(row, localCol, new QTableWidgetItem(sAddress));

        localCol++;
        QString sCity(safeTextItem(row, COL_CITY));
        ui->tblWeightedRanking->setItem(row, localCol, new QTableWidgetItem(sCity));

        localCol++;
        QString sState(safeTextItem(row, COL_STATE));
        ui->tblWeightedRanking->setItem(row, localCol, new QTableWidgetItem(sState));

        localCol++;
        QString sZip(safeTextItem(row, COL_ZIP));
        ui->tblWeightedRanking->setItem(row, localCol, new QTableWidgetItem(sZip));

        // Apply weighting and transforms
        QTableWidgetItem *wi;
        ui->tblWeightedRanking->setItem(row, 0, wi = new QTableWidgetItem(QString().sprintf("%5.2f", rank)));
        wi->setToolTip(QString().sprintf("%d", row));
    }
    // Grade by weighted score
    ui->tblWeightedRanking->sortByColumn(0);
    qDebug() << "Rebuild complete";
    rebuildInfo(false);
}

QString DlgWeight::safeTextItem( int row, int col )
{
    if (!m_tableData.contains(row)) return QString();
    if (!m_tableData[row].contains(col)) return QString();
    return m_tableData[row][col];
}

double DlgWeight::safeFloatItem( int row, int col )
{
    return safeTextItem(row,col).toDouble();
}

int DlgWeight::safeIntItem( int row, int col )
{
    return safeTextItem(row,col).toInt();
}

void DlgWeight::cellChanged( int row, int col, QString value )
{
    if (!m_tableData.contains(row))
    {
        m_tableData[row] = QMap<int,QString>();
        //qDebug() << "cellChanged inserted row" << row;
    }
    //qDebug() << "CellChanged(" << row << "," << col << ")=" << value;
    m_tableData[row].insert(col, value);
}

void DlgWeight::on_btnRefresh_clicked()
{
    rebuildDisplay();
}

void DlgWeight::on_tblWeightedRanking_doubleClicked(const QModelIndex &index)
{
    // Get URL
    // Get current row index
    int row = index.row();
    QTableWidgetItem *i = ui->tblWeightedRanking->item(row, 0);
    if (NULL==i) return;
    // Get original row index from tooltip text for column 0
    QString rowText(i->toolTip());
    row = rowText.toInt();
    if (!m_tableData.contains(row)) return;
    if (!m_tableData[row].contains(COL_URL)) return;
    QString sUrl(m_tableData[row][COL_URL]);
    qDebug() << "Opening:" << sUrl;
    if (sUrl.isEmpty()) return;
    QDesktopServices::openUrl(QUrl(sUrl));
}

void DlgWeight::on_btnSaveXlsx_clicked()
{
    // Get name to save as
    QString fileName(QFileDialog::getSaveFileName( this, "Save as xlsx file", QString(), "XLSX files (*.xlsx)" ));
    if (fileName.isEmpty()) return;
    qDebug() << "Saving as" << fileName;
    QXlsx::Document xlsx;
    xlsx.addSheet("Ranking");
    int n, row;
    int cols = ui->tblWeights->rowCount();
    xlsx.write( 1, 1, "Field" );
    xlsx.write( 2, 1, "Weight" );
    xlsx.write( 3, 1, "Transform" );
    for (row = 0; row < cols; row++)
    {
        QTableWidgetItem *i = ui->tblWeights->item(row, 0);
        if (NULL==i) continue;
        xlsx.write( 1, 2 + row, i->text() );
        i = ui->tblWeights->item(row, 1);
        xlsx.write( 2, 2 + row, i->text().toDouble() );
        i = ui->tblWeights->item(row, 2);
        xlsx.write( 3, 2 + row, i->text().toInt() );
    }
    QXlsx::Format hdrFormat;
    hdrFormat.setFontBold(true);
    for (n = 0; n < ui->tblWeightedRanking->horizontalHeader()->count(); n++)
    {
        xlsx.write(5, n+2, ui->tblWeightedRanking->horizontalHeaderItem(n)->text(), hdrFormat);
    }
    xlsx.write(5, 1, "Rank(c)", hdrFormat);
    // Set widths
    cols = ui->tblWeightedRanking->columnCount();
    xlsx.setColumnWidth(2, 6.57); // Static rank
    xlsx.setColumnWidth(4, 4.14); // BR
    xlsx.setColumnWidth(5, 4.14); // BA
    xlsx.setColumnWidth(6, 6.0); // SF
    xlsx.setColumnWidth(7, 5.71); // Lot
    xlsx.setColumnWidth(8, 5.14); // Year
    xlsx.setColumnWidth(9, 14.0); // Schools
    for (n = 11; n < cols - 2; n++) xlsx.setColumnWidth(n, 4.71);
    xlsx.setColumnWidth(cols - 2, 20.0);
    xlsx.setColumnWidth(cols - 1, 12.0);
    xlsx.setColumnWidth(cols - 0, 5.71);
    xlsx.setColumnWidth(cols + 1, 5.71);
    for (row = 0; row < ui->tblWeightedRanking->rowCount(); row++)
    {
        for (n = 0; n < cols; n++)
        {
            QTableWidgetItem *i = ui->tblWeightedRanking->item(row, n);
            int sheetRow = row + 6;
            if (i!=NULL)
            {
                switch (n)
                {
                /******
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
                    ****/
                //  0     1     2   3    4     5        6       7       8           9..?
                // Rank, price, br, ba, sqft, lot, year built, schools, distance, user columns, address, city, state, zip
                case 0:
                case 5:
                case 8:
                    xlsx.write( sheetRow, n + 2, i->text().toDouble() );
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                case 6:
                    xlsx.write( sheetRow, n + 2, i->text().toInt() );
                    break;
                default:
                    if (n + 1 == cols || n < cols - 4)
                    {
                        // zipcode and user columns
                        xlsx.write( sheetRow, n + 2, i->text().toInt() );
                        break;
                    } // else fall through
                case 7:
                    xlsx.write( sheetRow, n + 2, i->text() );
                    break;
                }

            }
        }
    }

    xlsx.saveAs(fileName);

}

// Rebuild lblInfo content with or without refresh message
void DlgWeight::rebuildInfo( bool withRefresh )
{
    QString s;
    int row;
    double dTotal = 0.0;
    for (row = 0; row < ui->tblWeights->rowCount(); row++)
    {
        QTableWidgetItem *i = ui->tblWeights->item(row, 1);
        if (NULL==i) continue;
        dTotal += (i->text().toDouble() * 10.0);
    }
    s += QString().sprintf("Maximum weighted score: %.1f", dTotal);
    if (withRefresh) s += "\nClick <a href=\"refresh\">refresh</a> button to regenerate";
    ui->lblInfo->setText(s);
}

void DlgWeight::on_lblInfo_linkActivated(const QString &link)
{
    qDebug() << "Got link:" << link;
    if (link == "refresh") rebuildDisplay();
}

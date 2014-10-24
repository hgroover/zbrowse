#ifndef DLGLINKCHOOSER_H
#define DLGLINKCHOOSER_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class DlgLinkChooser;
}

class DlgLinkChooser : public QDialog
{
    Q_OBJECT

public:
    explicit DlgLinkChooser(QWidget *parent = 0);
    ~DlgLinkChooser();

public slots:
    void newPage();
    void newEntry(QString articleBody);
    void endEntry();

signals:
    void fetchLink(QString url, int tableIndex, bool doFetch);
    void testLink(QString link);

private slots:
    void on_textBrowser_anchorClicked(const QUrl &arg1);


private:
    Ui::DlgLinkChooser *ui;
    QStringList m_links;
};

#endif // DLGLINKCHOOSER_H

#ifndef FILECHOOSER_H
#define FILECHOOSER_H

#include <qwidget.h>

class QLineEdit;
class QPushButton;
class Q3FileDialog;

class FileChooser : public QWidget
{
    Q_OBJECT

public:
    FileChooser( QWidget *parent, const char *name, int mode);
    ~FileChooser();

    QString fileName() const;
    QLineEdit   *lineEdit;
    QPushButton *button;
    Q3FileDialog *file_dia;

public slots:
    void setFileFilter( const QString &ffilter);
    void setDialogCaption( const QString &dcaption);
    void setDisabled( bool );

signals:
    void fileNameChanged( const QString & );

private slots:
    void chooseFile();
    void fileSelected_s();

private:
    int mode;
    QString m_ffilter;
    QString m_dcaption;
};
#endif

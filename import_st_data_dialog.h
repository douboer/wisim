/******************************************************************************************/
/**** PROGRAM: import_st_data_dialog.h                                                 ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#ifndef IMPORT_ST_DATA_DIALOG_H
#define IMPORT_ST_DATA_DIALOG_H

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QPushButton;
class QLabel;
class QLineEdit;
class QComboBox;
class NetworkClass;
class FileChooser;

class importStDataDia : public QDialog
{
    Q_OBJECT

public:
    importStDataDia( NetworkClass* np_param, QWidget* parent = 0, const char* name = 0,
            bool modal = FALSE, Qt::WFlags fl = 0 );
    ~importStDataDia();

    QLabel*      cs_fmt_textLabel;
    QComboBox*   cs_fmt_comboBox;

    QLabel*      period_textLabel;
    QLineEdit*   period_lineEdit;

    QLabel*      csc_node_textLabel;
    FileChooser* csc_node_fileChooser;

    QLabel*      csc_traffic_textLabel;
    FileChooser* csc_traffic_fileChooser;

    QPushButton* ok_pushButton;
    QPushButton* cancel_pushButton;

protected:
    Q3GridLayout* import_st_data_dialogLayout;

    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QSpacerItem* spacer3;
    QSpacerItem* spacer4;
    QSpacerItem* spacer5;
    QSpacerItem* spacer6;


private:
    NetworkClass* np;

    QString extension;

protected slots:
    virtual void languageChange();

    void ok_pushButton_clicked();
    void cancel_pushButton_clicked();

    void csc_node_fileChooser_textChanged( const QString & );
    void csc_traffic_fileChooser_textChanged( const QString & );

};

#endif // IMPORT_ST_DATA_DIALOG_H

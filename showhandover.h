
#ifndef SHOWHANDOVER_DIA
#define SHOWHANDOVER_DIA

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
class QLabel;
class QLineEdit;
class QPushButton;
class NetworkClass;

class showHandoverDia : public QDialog
{
    Q_OBJECT

public:
    showHandoverDia( NetworkClass* np_param, QWidget* parent = 0, const char* name = 0,
            bool modal = FALSE, Qt::WFlags fl = 0 );
    ~showHandoverDia();

    QLabel*       fromTextLabel;
    QLabel*       toTextLabel;
    QLineEdit*    fromLineEdit;
    QLineEdit*    toLineEdit;
    QPushButton*  buttonOk;
    QPushButton*  buttonCancel;


protected:
    Q3GridLayout*  fromToDialogLayout;

    Q3HBoxLayout*  layout;

    QSpacerItem*  spacer1;
    QSpacerItem*  spacer2;
    QSpacerItem*  spacer3;
    QSpacerItem*  spacer4;
    QSpacerItem*  spacer5;
    QSpacerItem*  spacer6;


private:
    NetworkClass* np;


private slots:
    void okBtnClicked();
    void cancelBtnClicked();


protected slots:
    virtual void languageChange();

};

#endif // SHOWHANDOVER_DIA

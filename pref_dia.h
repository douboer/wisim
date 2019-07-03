/******************************************************************************************/
/**** PROGRAM: pref_dia.h                                                              ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#ifndef PREF_DIA_H 
#define PREF_DIA_H

#include <qdialog.h>
#include <qwidget.h>
#include <q3listview.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>
#include "WiSim.h"

class NetworkClass;
class FigureEditor;

class Q3ButtonGroup;
class QComboBox;
class Q3GridLayout;
class Q3GroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;

template<class T> class ListClass;

#if HAS_ORACLE
class DbStDia;
#endif

class GenericSettingsDia : public QWidget
{   Q_OBJECT
public:
    GenericSettingsDia(NetworkClass *np_param, QWidget* parent);
    ~GenericSettingsDia();
    QRadioButton  *en_btn;
    QRadioButton  *zh_btn;
    QSpinBox *cell_size_spinbox;
    QComboBox *pwr_unit_combobox;
    QComboBox *cell_name_combobox;

    ListClass<int> *pwr_unit_list;

private:
    NetworkClass *np;
};

class PrefDia : public QDialog
{
    Q_OBJECT
public:
    PrefDia(FigureEditor* editor_param, QWidget* parent);
    ~PrefDia();
    GenericSettingsDia       *generic_widget;
#if HAS_ORACLE
    DbStDia       *database_widget;
#endif

signals:
    void pwr_unit_changed();

private slots:
    void save_btn_clicked();
    void cancel_btn_clicked();

private:
    NetworkClass *np;
    FigureEditor *editor;
};

#endif

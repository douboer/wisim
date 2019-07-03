/*******************************************************/
/* cvg_part_wizard.h file gives the part/all cells     */
/* choice to do simulation                             */
/*******************************************************/

#ifndef CVG_PART_WIZARD_H
#define CVG_PART_WIZARD_H

#include <qvariant.h>
#include <q3wizard.h>
#include <q3valuevector.h>
#include <qmap.h>
#include <qstring.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>

typedef QMap<QString, int> CellIndexMap;

class Q3ButtonGroup;
class QCheckBox;
class QComboBox;
class Q3GridLayout;
class Q3HBoxLayout;
class QLabel;
class QLineEdit;
class Q3ListBox;
class Q3ListBoxItem;
class QRadioButton;
class QSpacerItem;
class QStringList;
class QToolButton;
class Q3VBoxLayout;
class QWidget;

class FigureEditor;
class NetworkClass;

class Cvg_Part_Wizard : public Q3Wizard
{
    Q_OBJECT

public:
    Cvg_Part_Wizard( FigureEditor* editor_para, NetworkClass*, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~Cvg_Part_Wizard();

    QWidget* chooseWizardPage;
    Q3ButtonGroup* buttonGroup1;
    QRadioButton* chooseAll;
    QRadioButton* choosePartBtn;
    QLabel* textLabel1;
    QLineEdit* maxDist;
    QWidget* partWizardPage;
    QLabel* textLabel1_2;
    QToolButton* DeleteToolBtn;
    QComboBox* Category_comboBox;
    QToolButton* ChooseToolBtn;
    Q3ListBox* Source_listBox;
    Q3ListBox* Dest_listBox;
    //QWidget* WizardPage;
    QCheckBox *useGPMCheckBox;

    bool getChoice();
    double getMaxDist();
    QStringList& getPartCellList();// display name
    Q3ValueVector <int>& getPartCellVector( ); // real index

public slots:
    virtual void checkToJump( int i );
    virtual void init();
    virtual void changeOptions( const QString & );
    virtual void selectCells();
    virtual void deleteCells();
    virtual void changeDistValue( const QString & );

protected:
    Q3HBoxLayout* layout6;
    QSpacerItem* spacer1;

protected slots:
    virtual void languageChange();
    virtual void next(); 
    virtual void accept();
    void setBtnEnable();
 
private:
    // the data interface for the WiSim
    FigureEditor *editor;
    NetworkClass* np;

    bool m_bIsPart; 
    double m_dMaxDistance;
    // need an array to store the user's choice cell list
    QStringList *m_CellList; // dest.listbox display values
    Q3ValueVector <int> m_CellVector; // index values, wanted

    
    // added on Nov. 22, 2004
    CellIndexMap m_Map;       // flag
    CellIndexMap m_oMapIndex; // cmd index

    QStringList m_osList; // source listbox display value
    void sortStringList(QStringList &);
    


signals:
  void chooseAllSig();
    
     

};

#endif // CVG_PART_WIZARD_H

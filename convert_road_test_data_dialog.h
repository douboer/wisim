/******************************************/
/* This class creates a dialog for user to*/
/* input the requirment data for road     */
/* test data conversion.                  */
/* Auther:                                */
/*         Wei Ben                        */
/* Data  :                                */
/*         Sept. 07, 2004                 */
/******************************************/

/*
 * The road test convertation dialog to display
 * Method:
 *        1. supply the "InputFile" chosen as a QFileDialog
 *        2. supply the outputFile" chosen as a QFileDiallg
 *        3. supply the "file format as a QSpinBox
 *        4. supply the "threshold and resolution" as QSpinBox
 *        5. supply the "number of test file" as  QTextEdit
 *
 */

#ifndef CONVERTROADTESTDATADIALOG_H
#define CONVERTROADTESTDATADIALOG_H

#include <qdialog.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
#include <Q3Frame>
#include "element.h"
#include "helpdialog.h"
#include "filechooser.h"
#include "road_test_pair.h"

class Q3ButtonGroup;
class Q3Frame;
class Q3HBoxLayout;
class QLabel;
class QPushButton;
class QSpinBox;
class QComboBox;
class Q3VBoxLayout;
class QLineEdit;
class QPushButton;
class Q3ListBox;
// added on Nov. 16, 2004
class QStringList;


class ConvertRoadTestDataDialog: public QDialog {

  Q_OBJECT

public:

  ConvertRoadTestDataDialog(QWidget* parent = 0, const char* name = "Road Test Points Convert",
                bool modal = TRUE, Qt::WFlags f = 0);

  ~ConvertRoadTestDataDialog();

  QLabel *oDataFormatLabel;
  QLabel *oFileNumLabel;
  QLabel *oFileButtonLabel;
  QLabel *oResolutionLabel;
  QLabel *oOutputTextLabel;
  QLabel *oThresholdLabel;
  QLabel *oInputFileNumLabel;

  Q3Frame *oInputValuesFrame;
  QComboBox *oFileTypeComboBox;
  QSpinBox *oThresholdSpinBox;
  QSpinBox *oResolutionSpinBox;
  QPushButton *oInputPushButton;
  QPushButton *oOutputPushButton;
  QLineEdit *oFileNumEdit;
  Q3ListBox *oDisplayInputFiles;

  FileChooser *fileChooser;

  QPushButton *okPushButton;
  QPushButton *cancelPushButton;
  QPushButton *oTypeHelp;
  QPushButton *removeBtn;
  QComboBox* oUnitComboBox;

 signals:

  void listboxchanged(bool bYes );

protected slots:

  void chooseInputFiles();
  void chooseOutputFile(const QString&);
  void chooseOutputFile();
  void runPerl();
  void validNum(const QString &);
  void cancelResponse();
  void openHelp();
  virtual void changeOptions( const QString & );
  void removeItem();

  void changeUnitOpt( const QString &);
  
  void updateOkEnabled(bool bNo );

protected:

    Q3VBoxLayout *roadtestFormLayout;
    Q3VBoxLayout *oInputValuesFrameLayout;
    Q3HBoxLayout *inputFileTypeLayout;
    Q3HBoxLayout *OutputxLayout;
    Q3VBoxLayout *addValuesFrameLayout;
    Q3VBoxLayout *addValuesButtonGroupLayout;
    Q3HBoxLayout *decimalPlacesLayout;
    Q3HBoxLayout *thresholdLayout;
    Q3HBoxLayout *buttonsLayout;
    Q3HBoxLayout *resolutionLayout;
    Q3HBoxLayout *FileNumLayout;
    Q3HBoxLayout *FileButtonLayout;
    Q3HBoxLayout *OutputLayout;
    Q3HBoxLayout *buttonLayout;

 private:

    //member variables:
    Element* data;
    helpDialog* help;
    void getInput();
    void setOutputFile(QString filename);
    bool validInput();
    QString sExtension;

    // added by Nov. 16
    SpecialFormatDialog *oTypeSpecialDlg;
    bool m_bTypeC;
    QStringList list;

};

#endif




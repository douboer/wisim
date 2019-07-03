 /*
 * The data class, save data for road test convert file (perl input).
 * Method: 
 *          classical getters and setters methods.
 *
 * Auther: Wei Ben
 * Date  : Sep. 7, 2004 
 *
 */

#ifndef ELEMENT_H
#define ELEMENT_H

#include <qstring.h>
#include <qstringlist.h>

class QStringList;
class QString;

class Element {
 
 private:
  
  QString sOutput_file;
  QStringList vInput_files;
  int iNum_Input;
  QString sFormat;
  int iThreshold;
  double dResolution;

  QString sHelpFileName;
  QString sUnitName;

 public:


  Element(); 

  Element(const QString& output, const QStringList& input, int iNum_input, QString cFormat, int threshold,  double resolution ) {
    sOutput_file = output;
    vInput_files = QStringList(input);
    iNum_input = iNum_input;
    sFormat = cFormat;
    iThreshold = threshold;
    dResolution = resolution;
    //default setting for help file
    sHelpFileName = QString("file_type_help.txt");
  }

  ~Element();


  
  QString getOutputFile()const;
  QStringList getInputFile() const;
  int getNumberOfInput() const;
  QString getFileFormat() const ;
  int getThreshold() const;
  double getResolution() const;
  QString getHelpFile() const;
  QString getUnitName() const;
  

  void setOutputFile( QString s);
  void setInputFiles( QStringList& list);
  void setNumInput( int iNum );
  void setFormat(QString c );
  void setThreshold( int threshold);
  void setResolution( double resl);
  void setHelpFile( const char* s);

  void setUnitName( QString& s);

  void writeToFile();
  bool validation();
  
};
#endif

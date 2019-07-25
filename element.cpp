/********************************************/
/* This class is a data class stored the    */
/* the user input, and does some validation */
/* Auther:  Wei  Ben                        */
/* Date  :  Sept. 7 2004                    */
/********************************************/


#include <qstringlist.h>
#include <qstring.h>
#include <qfile.h>
//Added by qt3to4:
#include <Q3TextStream>
#include "wisim.h"
#include "element.h"

extern char *wisim_home;

Element::Element() {
  sHelpFileName = QString(wisim_home) + QString(QChar(FPATH_SEPARATOR)) + "file_type_help.htm";
}

Element::~Element() {

}

QString Element::getOutputFile() const {
    return sOutput_file;
  }

QStringList Element::getInputFile() const {
    return vInput_files;
  }

int Element::getNumberOfInput() const {
    return iNum_Input;
  }


QString Element::getFileFormat() const {
    return sFormat;
}

int Element::getThreshold() const {
    return iThreshold;
}

double Element::getResolution() const {
    return dResolution;
}

QString Element::getUnitName() const {

  return sUnitName;
}

void Element::setUnitName( QString& s) {

  sUnitName = s;
}

void Element::setOutputFile( QString s) {
    sOutput_file = s;
    //qDebug(sOutput_file);
}
  
void Element::setInputFiles( QStringList& list) {
    vInput_files = QStringList(list);
}
 
void Element::setNumInput( int iNum) {
    iNum_Input = iNum;
}

void Element::setFormat( QString c) {
    sFormat = c;
}

void Element::setThreshold( int  threshold) {
    iThreshold = threshold;
}

void Element::setResolution( double  resl) {
    dResolution = resl;
}

/***************************************************************/
/* put all the user inputs to a file                           */
/* Notice: the writing sequence = convert_rtd.pl read sequence */
/* created:                                                    */
/* modifed: Nov. 16, 2004                                      */
/***************************************************************/
void Element::writeToFile() {
  
  // ----------modified on Nov 29, 2004 ------------------
  // changed to use CVECTOR() to adapt to no-GUI, no QT support
  char *txtfile;
  txtfile = CVECTOR(strlen(wisim_home) + 1 + strlen("rtd_input.txt"));
  sprintf(txtfile, "%s%crtd_input.txt", wisim_home, FPATH_SEPARATOR); 
  QFile file(txtfile);
  // -----------------------------------------------------

  if ( file.open( QIODevice::WriteOnly ) ) {
    Q3TextStream stream( &file );
    stream << sFormat << "\n";
    stream << iNum_Input << "\n";

    for ( QStringList::Iterator it = vInput_files.begin(); it != vInput_files.end(); ++it )
      stream << *it << "\n";

    stream << sUnitName << "\n";
    stream << iThreshold << "\n";
    stream << dResolution << "\n";
    stream << sOutput_file << "\n";

    file.close();
  }

  else {
    printf("can't open temperary file\n");
    exit(1);
  }



}

bool Element::validation() {
  bool result;
  if(sOutput_file == "") 
    result = false;
  else if(vInput_files.empty())
    result = false;
  else if (sFormat.isEmpty())
    result = false;
  else if (iThreshold == 0)
    result = false;
  else if( dResolution == 0)
    result = false;
  else 
    result = true;
  return result;
}


QString Element::getHelpFile() const {
  return sHelpFileName;
}

void Element::setHelpFile(const char* s) {
  sHelpFileName = QString(s);
}

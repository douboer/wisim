#include "cconst.h"
#include "filechooser.h"
#include <qlineedit.h>
#include <qpushbutton.h>
#include <q3filedialog.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3HBoxLayout>

FileChooser::FileChooser( QWidget *parent, const char *name, int p_mode )
    : QWidget( parent, name )
{

    mode = p_mode;
    file_dia = new Q3FileDialog;
    
    Q3HBoxLayout *layout = new Q3HBoxLayout( this );
    layout->setMargin( 0 );

    lineEdit = new QLineEdit( this, "filechooser_lineedit" );
    layout->addWidget( lineEdit );

    connect( lineEdit, SIGNAL( textChanged( const QString & ) ),
	     this, SIGNAL( fileNameChanged( const QString & ) ) );

    button = new QPushButton( "...", this, "filechooser_button" );
    button->setFixedWidth( button->fontMetrics().width( " ... " ) );
    button->setFixedWidth( 35 );
    layout->addWidget( button );

    connect( button, SIGNAL( clicked() ),
	     this, SLOT( chooseFile() ) );
    connect( file_dia, SIGNAL( fileSelected ( const QString & ) ), this, SLOT( fileSelected_s() ) );

    setDialogCaption("Choose File to Open...");

    setFocusProxy( lineEdit );
}

FileChooser::~FileChooser()
{
//    delete file_dia; 
}

void FileChooser::setFileFilter( const QString &fn ) 
{
    m_ffilter = fn;
}

void FileChooser::setDialogCaption( const QString &dcaption ) 
{
    m_dcaption = dcaption;
}

QString FileChooser::fileName() const
{
    return lineEdit->text();
}

void FileChooser::chooseFile()
{
    QString file_name;
    // xxxxx QString filt    = tr("Files") + QString(" (%1);;").arg(m_ffilter) + tr("All Files") + " (*)";
    QString title   = QString(tr("file_dialog"));
    QString caption = QString( "%1").arg( m_dcaption );

    if (mode == CConst::readFileMode) {
        file_name = file_dia->getOpenFileName( QString::null, m_ffilter, this, title, caption);
    } else {
        file_name = file_dia->getSaveFileName( QString::null, m_ffilter, this, title, caption);
    }

    lineEdit->setText(file_name);

    emit fileNameChanged( file_name );
}

void FileChooser::fileSelected_s()
{
    qDebug( "XXXXXXXXXXXXX test XXXXXXXXXXXXXX\n" );
}

void FileChooser::setDisabled( bool flag )
{
    switch ( flag ) {
        case TRUE :
            button->setDisabled( TRUE );
            lineEdit->setDisabled( TRUE );
            break;
        case FALSE :
            button->setDisabled( FALSE );
            lineEdit->setDisabled( FALSE );
            break;
    }
}

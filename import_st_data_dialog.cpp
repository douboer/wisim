/******************************************************************************************/
/**** PROGRAM: import_st_data_dialog.cpp                                                     ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include "import_st_data_dialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3CString>

#include "wisim.h"
#include "cconst.h"
#include "filechooser.h"


/*
 *
 */
importStDataDia::importStDataDia( NetworkClass* np_param, QWidget* parent,
    const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent, name, modal, fl )
{

    std::cout << "importStDataDia ..." << std::endl;

    if ( !name )
	setName( "importStDataDia" );

    np = np_param;

    import_st_data_dialogLayout = new Q3GridLayout( this, 1, 1, 11, 6, "import_st_data_dialogLayout"); 

    cancel_pushButton = new QPushButton( this, "cancel_pushButton" );
    import_st_data_dialogLayout->addWidget( cancel_pushButton, 5, 4 );

    spacer1 = new QSpacerItem( 51, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    import_st_data_dialogLayout->addItem( spacer1, 5, 0 );

    ok_pushButton = new QPushButton( this, "ok_pushButton" );
    import_st_data_dialogLayout->addMultiCellWidget( ok_pushButton, 5, 5, 1, 2 );

    spacer2 = new QSpacerItem( 51, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    import_st_data_dialogLayout->addItem( spacer2, 5, 3 );

    spacer3 = new QSpacerItem( 51, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    import_st_data_dialogLayout->addItem( spacer3, 5, 5 );

    cs_fmt_comboBox = new QComboBox( FALSE, this, "cs_fmt_comboBox" );
    import_st_data_dialogLayout->addMultiCellWidget( cs_fmt_comboBox, 0, 0, 2, 3 );

    spacer4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    import_st_data_dialogLayout->addItem( spacer4, 4, 3 );

    csc_traffic_fileChooser = new FileChooser( this, "csc_traffic_fileChooser", CConst::saveFileMode );
    import_st_data_dialogLayout->addMultiCellWidget( csc_traffic_fileChooser, 3, 3, 2, 5 );

    csc_traffic_textLabel = new QLabel( this, "csc_traffic_textLabel" );
    import_st_data_dialogLayout->addMultiCellWidget( csc_traffic_textLabel, 3, 3, 0, 1 );

    spacer5 = new QSpacerItem( 91, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    import_st_data_dialogLayout->addMultiCell( spacer5, 0, 0, 4, 5 );

    cs_fmt_textLabel = new QLabel( this, "cs_fmt_textLabel" );
    import_st_data_dialogLayout->addWidget( cs_fmt_textLabel, 0, 0 );

    spacer6 = new QSpacerItem( 184, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    import_st_data_dialogLayout->addMultiCell( spacer6, 1, 1, 4, 5 );

    period_lineEdit = new QLineEdit( this, "period_lineEdit" );
    import_st_data_dialogLayout->addMultiCellWidget( period_lineEdit, 1, 1, 2, 3 );

    csc_node_fileChooser = new FileChooser( this, "csc_node_fileChooser", CConst::saveFileMode );
    import_st_data_dialogLayout->addMultiCellWidget( csc_node_fileChooser, 2, 2, 2, 5 );

    csc_node_textLabel = new QLabel( this, "csc_node_textLabel" );
    import_st_data_dialogLayout->addWidget( csc_node_textLabel, 2, 0 );

    period_textLabel = new QLabel( this, "period_textLabel" );
    import_st_data_dialogLayout->addMultiCellWidget( period_textLabel, 1, 1, 0, 1 );


    connect( csc_node_fileChooser->lineEdit, SIGNAL( textChanged ( const QString & ) ),
            this, SLOT( csc_node_fileChooser_textChanged( const QString & ) ));
    connect( csc_traffic_fileChooser->lineEdit, SIGNAL( textChanged ( const QString & ) ),
            this, SLOT( csc_traffic_fileChooser_textChanged( const QString & ) ));

    connect( ok_pushButton, SIGNAL( clicked() ), this, SLOT( ok_pushButton_clicked() ) );
    connect( cancel_pushButton, SIGNAL( clicked() ), this, SLOT( cancel_pushButton_clicked() ) );


    // initilization
    extension = "txt";
    QString filt = tr("Text Files") + " (*.txt);;" + tr("All Files") + " (*)";
    csc_node_fileChooser ->setFileFilter( filt );
    csc_node_fileChooser ->setDialogCaption("Choose File...");
    csc_traffic_fileChooser ->setFileFilter( filt );
    csc_traffic_fileChooser ->setDialogCaption("Choose File...");

    languageChange();
    resize( QSize(509, 184).expandedTo(minimumSizeHint()) );

    exec();
}

/*
 *  Destroys the object and frees any allocated resources
 */
importStDataDia::~importStDataDia()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void importStDataDia::languageChange()
{
    setCaption( tr( "Import ST Data" ) );

    cs_fmt_textLabel->setText( tr( "CS Format" ) );
    csc_node_textLabel->setText( tr( "CSC Node File" ) );
    csc_traffic_textLabel->setText( tr( "CSC Traffic File" ) );
    period_textLabel->setText( tr( "Hour of Day (Period)" ) );

    cs_fmt_comboBox->clear();
    cs_fmt_comboBox->insertItem( tr( "Melco" ) );
    cs_fmt_comboBox->insertItem( tr( "Sanyo" ) );

    ok_pushButton->setAccel( QKeySequence( tr( "Alt+O" ) ) );
    ok_pushButton->setText( tr( "&Ok" ) );
    cancel_pushButton->setText( tr( "&Cancel" ) );
    cancel_pushButton->setAccel( QKeySequence( tr( "Alt+C" ) ) );

}


void importStDataDia::ok_pushButton_clicked()
{
    hide();

    char *chptr;

    chptr = np->line_buf;
    chptr += sprintf(chptr, "import_st_data ");
    chptr += sprintf(chptr, ((cs_fmt_comboBox->currentItem() == 0)
                ? "-fmt melco "
                : "-fmt sanyo "
                ));

    QString csc_node_str = csc_node_fileChooser ->lineEdit->text();
    if (!csc_node_str.isEmpty()) {
        QRegExp rx = QRegExp("\\." + extension + "$");
        if (rx.search(csc_node_str) == -1) {
            csc_node_str += "." + extension;
        }

        Q3CString qcs(2*csc_node_str.length());
        qcs = csc_node_str.local8Bit();

        chptr += sprintf(chptr, "-fcsc \'%s\' ", (const char *) qcs);
    } else {
        return;
    }

    QString csc_traffic_str = csc_traffic_fileChooser ->lineEdit->text();
    if (!csc_traffic_str.isEmpty()) {
        QRegExp rx = QRegExp("\\." + extension + "$");
        if (rx.search(csc_traffic_str) == -1) {
            csc_traffic_str += "." + extension;
        }

        Q3CString qcs(2*csc_traffic_str.length());
        qcs = csc_traffic_str.local8Bit();

        chptr += sprintf(chptr, "-f \'%s\' " , (const char *) qcs);
    } else {
        return;
    }
    chptr += sprintf(chptr, "-p %s",  period_lineEdit->text().latin1());

    np->process_command( np->line_buf);

    delete this;
}


void importStDataDia::cancel_pushButton_clicked()
{
    delete this;
}


void importStDataDia::csc_node_fileChooser_textChanged( const QString & str )
{
    if ( str != "" )
        ok_pushButton->setDisabled( false );
    else
        ok_pushButton->setDisabled( true );
}


void importStDataDia::csc_traffic_fileChooser_textChanged( const QString & str )
{
    if ( str != "" )
        ok_pushButton->setDisabled( false );
    else
        ok_pushButton->setDisabled( true );
}



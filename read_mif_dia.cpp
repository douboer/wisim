
/******************************************************************************************/
/**** PROGRAM: read_mif_dia.cpp                                                        ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include "WiSim.h"
#include "hot_color.h"
#include "list.h"
#include "map_layer.h"
#include "read_mif_dia.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <q3filedialog.h>
#include <q3buttongroup.h>
#include <qcolordialog.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3CString>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

ReadMIFDia::ReadMIFDia(NetworkClass* np_param, QWidget* parent)
    : QDialog( parent, 0, true)
{
    setName( "Read MIF Dialog" );
    setCaption( tr("Read MIF") );
    np = np_param;
    int mif_name_repeat_num = 0;

    read_mif_vLayout = new Q3VBoxLayout( this, 11, 6, "read_mif_vlayout");

    Q3HBoxLayout *file_name_hLayout = new Q3HBoxLayout( 0, 11, 6, "file_name_hlayout" );
    file_name_lbl = new QLabel( this, "file_name_lbl" );
    file_name_lbl->setText( tr("File name") );
    file_name_lbl->setMaximumWidth( 80 );
    file_name_hLayout->addWidget( file_name_lbl, 0, Qt::AlignLeft );

    file_name_val = new QLineEdit( this, "file_name_val" );
    file_name_val->setMaximumSize( QSize( 120, 32767 ) );
    file_name_hLayout->addWidget( file_name_val, 0, Qt::AlignLeft );

    file_name_toolbtn = new QToolButton( this, "file_name_toolbtn" );
    file_name_toolbtn->setMaximumWidth( 100 );
    file_name_toolbtn->setText( "..." );
    file_name_hLayout->addWidget( file_name_toolbtn, 0, Qt::AlignLeft );
    read_mif_vLayout->addLayout( file_name_hLayout, 0 );

    Q3HBoxLayout *color_hLayout = new Q3HBoxLayout( 0, 11, 6, "color_hlayout" );
    color_lbl = new QLabel( this, "color_lbl" );
    color_lbl->setText( tr("Color") );
    color_lbl->setMaximumWidth( 100 );
    color_hLayout->addWidget( color_lbl, 0, Qt::AlignLeft );

    // color = np->default_color_list[np->map_layer_list->getSize()];
    color = np->hot_color->get_color(np->map_layer_list->getSize());
    color_btn = new QPushButton( this, "color_btn" );
    color_btn->setText( "" );
    color_btn->setMaximumWidth( 80 );
    color_btn->setPaletteBackgroundColor( color );
    color_hLayout->addWidget( color_btn, 0, Qt::AlignLeft );

    read_mif_vLayout->addLayout( color_hLayout, 0 );

    read_type_btngroup = new Q3ButtonGroup( this, "read_type_btngroup");
    read_type_btngroup->setColumnLayout(0, Qt::Vertical );
    read_type_btngroup->layout()->setSpacing( 6 );
    read_type_btngroup->layout()->setMargin( 11 );
    read_type_btngroupLayout = new Q3GridLayout( read_type_btngroup->layout() );
    read_type_btngroupLayout->setAlignment( Qt::AlignTop );

    filt_radiobtn = new QRadioButton( read_type_btngroup, "filt_radiobtn" );
    read_type_btngroup->insert( filt_radiobtn, 0 );
    filt_radiobtn->setEnabled( true );
    filt_radiobtn->setChecked( true );
    filt_radiobtn->setText( tr("Filter objects outside the system boundary") );
    filt_radiobtn->setMaximumSize( QSize( 300, 32767 ) );
    read_type_btngroupLayout->addWidget( filt_radiobtn, 0, 0 );

    include_radiobtn = new QRadioButton( read_type_btngroup, "include_radiobtn" );
    read_type_btngroup->insert( include_radiobtn, 1 );
    include_radiobtn->setEnabled( true );
    include_radiobtn->setText( tr("Include all objects") );
    include_radiobtn->setMaximumSize( QSize( 300, 32767 ) );
    read_type_btngroupLayout->addWidget( include_radiobtn, 1, 0 );
    read_mif_vLayout->addWidget( read_type_btngroup, 0 );

    Q3HBoxLayout *map_layer_hlayout = new Q3HBoxLayout( 0, 11, 6, "map_layer_hlayout" );
    map_layer_name_lbl = new QLabel( this, "map_layer_name_lbl" );
    map_layer_name_lbl->setText( tr("Map Layer Name") );
    map_layer_name_lbl->setMaximumWidth( 100 );
    map_layer_hlayout->addWidget( map_layer_name_lbl, 0, Qt::AlignLeft );

    map_layer_val = new QLineEdit( this, "map_layer_name_val");
    map_layer_val->setMaximumSize( QSize( 300, 32767 ) );
    map_layer_hlayout->addWidget( map_layer_val, 0, Qt::AlignLeft );
    read_mif_vLayout->addLayout( map_layer_hlayout, 0 );
    for( int idx=0; idx<np->map_layer_list->getSize(); idx++) {
        if( strncmp((*(np->map_layer_list))[idx]->name,"map_layer_", 10)==0 ) {
            mif_name_repeat_num = mif_name_repeat_num + 1; 
        }
    }
    map_layer_val->setText( QString("map_layer_%1").arg(mif_name_repeat_num) );

    Q3HBoxLayout *ok_cancel_hlayout = new Q3HBoxLayout( 0, 11, 6, "ok_cancel_hlayout" );
    ok_btn = new QPushButton( this, "ok_btn" );
    ok_btn->setText( tr("&Ok") );
    ok_btn->setMaximumWidth( 80 );
    ok_cancel_hlayout->addWidget( ok_btn, 0, Qt::AlignCenter );

    cancel_btn = new QPushButton( this, "cancel_btn" );
    cancel_btn->setText( tr("&Cancel") );
    cancel_btn->setMaximumWidth( 80 );
    ok_cancel_hlayout->addWidget( cancel_btn, 0, Qt::AlignCenter );
    read_mif_vLayout->addLayout( ok_cancel_hlayout, 0 );

    connect( ok_btn, SIGNAL( clicked() ), this, SLOT(ok_btn_clicked()) );
    connect( cancel_btn, SIGNAL( clicked() ), this, SLOT(cancel_btn_clicked()) );
    connect( color_btn, SIGNAL( clicked() ), this, SLOT(color_btn_clicked()) );
    connect( file_name_toolbtn, SIGNAL( clicked() ), this, SLOT(file_name_toolbtn_clicked()) );

    exec();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ReadMIFDia::~ReadMIFDia()
{
}

void ReadMIFDia::ok_btn_clicked()
{
    int     filt_val = 0;
    QString filename = NULL;
    QString name     = NULL;

    if ( filt_radiobtn->isChecked() ) {
        filt_val = 1;
    } 

    if ( include_radiobtn->isChecked() ) {
        filt_val = 0;
    }

    filename = file_name_val->text();
    name = map_layer_val->text();

    if ( !filename.isEmpty() && !name.isEmpty() ) {
        Q3CString qcs(2*name.length());
        qcs = filename.local8Bit();

        sprintf(np->line_buf, "read_map_layer -f \'%s\' -fmt mif -name %s -filter %d", (const char *) qcs, name.latin1(), filt_val);
        np->process_command(np->line_buf);

        sprintf(np->line_buf, "set_color -map_layer_idx %d -map_layer %d", np->map_layer_list->getSize()-1, color);
        np->process_command(np->line_buf);

        delete this;
    }
}


void ReadMIFDia::cancel_btn_clicked()
{
    delete this;
}

void ReadMIFDia::color_btn_clicked()
{
    QColor qcolor = QColorDialog::getColor(color);
    if (qcolor.isValid()) {
        color = (qcolor.rgb())&(0xFFFFFF);
        color_btn->setPaletteBackgroundColor( color );
    }
}

void ReadMIFDia::file_name_toolbtn_clicked()
{
    Q3FileDialog *file_name_dia = new Q3FileDialog;
    QString filename = file_name_dia->getOpenFileName( "", tr("MIF Files") + " (*.mif);;" + tr("All Files") + " (*)",
                           this, "read MIF dialog", tr("Choose MIF File") );

    if ( !filename.isEmpty() ) {
        file_name_val->setText( filename );
    }
}

void ReadMIFDia::read_type_btngroup_clicked(int i)
{
    switch( i ) {
        case 0:
            filt_radiobtn->setChecked( true );
            include_radiobtn->setChecked( false );
            break;
        case 1:
            filt_radiobtn->setChecked( false );
            include_radiobtn->setChecked( true );
            break;
        default:
            printf("WARNING:Type Raido Button error in read MIF dialog!\n");
            break;
    }
}


/******************************************************************************************/
/**** PROGRAM: clutter_sim_dia.cpp                                                     ****/
/**** Implementation of Class PropModMgrDia                                            ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include <q3buttongroup.h>
#include <q3header.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <q3listview.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <q3ptrlist.h> 
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h> 
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <q3whatsthis.h>
#include <q3widgetstack.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <Q3CString>
#include <Q3VBoxLayout>

#include "cconst.h"
#include "WiSim.h"
#include "WiSim_gui.h"
#include "clutter_data_analysis.h"
#include "expo_prop_wizard.h"
#include "gconst.h"
#include "icons_test.h"
#include "list.h"
#include "map_clutter.h"
#include "prop_mod_mgr_dia.h"
#include "prop_type_name_dia.h"
#include "sector_prop_table.h"
#include "info_wid.h"

#include <iostream>

/******************************************************************************************/
/**** FUNCTION: PropModelListItem::PropModelListItem                                   ****/
/******************************************************************************************/
PropModelListItem::PropModelListItem(Q3ListView *parent, int m_pm_idx)
    : Q3ListViewItem(parent), pm_idx(m_pm_idx)
{
}
/******************************************************************************************/
/**** FUNCTION: PropModelListItem::~PropModelListItem                                  ****/
/******************************************************************************************/
PropModelListItem::~PropModelListItem()
{
}
/******************************************************************************************/
/**** FUNCTION: PropModelListItem:: getIndex & setIndex                                ****/
/******************************************************************************************/
int  PropModelListItem::getIndex()             { return pm_idx; }
void PropModelListItem::setIndex(int m_pm_idx) { pm_idx = m_pm_idx; }
/******************************************************************************************/
/******************************************************************************************/
/**** FUNCTION: PropModelListItem::compare                                             ****/
/******************************************************************************************/
int PropModelListItem::compare(Q3ListViewItem *qlvi, int col, bool ascending ) const
{
    int retval;

    QString k1 = key(col, ascending);
    QString k2 = qlvi->key(col, ascending);
    retval = qstringcmp(k1, k2);

    return(retval);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: ToolButtonWithText::ToolButtonWithText                                 ****/
/******************************************************************************************/
ToolButtonWithText::ToolButtonWithText( QWidget* parent, const char* name ) : QWidget( parent, name )
{
    vlayout = new Q3VBoxLayout(this, 0, 6); 
    toolButton = new QToolButton(this);
    vlayout->addWidget( toolButton );
    textLabel = new QLabel( this, "textLabel" );
    vlayout->addWidget( textLabel );

    resize( QSize(60, 45).expandedTo(minimumSizeHint()) );
}
/******************************************************************************************/
/**** FUNCTION: ToolButtonWithText::~ToolButtonWithText                                ****/
/******************************************************************************************/
ToolButtonWithText::~ToolButtonWithText()
{
}
/******************************************************************************************/
/**** FUNCTION: ToolButtonWithText::setIconText                                        ****/
/******************************************************************************************/
void ToolButtonWithText::setIconSet( const QIcon& icon )
{
    toolButton->setIconSet( icon );
}
/******************************************************************************************/
/**** FUNCTION: ToolButtonWithText::setText                                            ****/
/******************************************************************************************/
void ToolButtonWithText::setText( const QString& txt )
{
    textLabel->setText( txt );
}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::PropModMgrDia                                           ****/
/******************************************************************************************/
PropModMgrDia::PropModMgrDia( NetworkClass *np_param, QWidget* parent )
    : QDialog( parent, 0, true), np( np_param )
{
    setName( "PropModMgrDia" );
    setMinimumSize( QSize( 550, 400 ) );

    PropAnaDiaLayout = new Q3GridLayout( this, 1, 1, 11, 6, "PropAnaDiaLayout"); 
    ok_cancel_Layout = new Q3HBoxLayout( 0, 0, 6, "ok_cancel_Layout"); 

    //if let setAutoDefault( true ).  when we press enter key,  the corresponding button will be clicked, otherwise will not.
    ok_cancel_Layout->addStretch();

    buttonClose = new QPushButton( this );
    buttonClose->setAutoDefault( FALSE );
    ok_cancel_Layout->addWidget( buttonClose );

    ok_cancel_Layout->addStretch();

    buttonCancel = new QPushButton( this );
    buttonCancel->setAutoDefault( FALSE );
    ok_cancel_Layout->addWidget( buttonCancel );

    ok_cancel_Layout->addStretch();

    buttonApply = new QPushButton( this );
    buttonApply->setAutoDefault( FALSE );
    ok_cancel_Layout->addWidget( buttonApply );

    ok_cancel_Layout->addStretch();

    PropAnaDiaLayout->addMultiCellLayout( ok_cancel_Layout, 1, 1, 0, 1 );

    /*
        toolButton Layout
     */
    toolbtn_listview_layout = new Q3VBoxLayout( 0, 0, 0, "toolbtn_listview_layout"); 

    buttonGroup1 = new Q3ButtonGroup( this, "buttonGroup1" );
    buttonGroup1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, buttonGroup1->sizePolicy().hasHeightForWidth() ) );
    buttonGroup1->setMaximumSize( QSize( 300, 32767 ) );
    buttonGroup1->setColumnLayout(0, Qt::Vertical );
    buttonGroup1->layout()->setSpacing( 6 );
    buttonGroup1->layout()->setMargin( 11 );
    buttonGroup1Layout = new Q3HBoxLayout( buttonGroup1->layout() );
    buttonGroup1Layout->setAlignment( Qt::AlignTop );

    create_toolButton = new ToolButtonWithText( buttonGroup1, "create_toolButton" );
    create_toolButton->setIconSet( QIcon( TestIcon::icon_create ) );
    create_toolButton->setText(tr("Create"));
    buttonGroup1Layout->addWidget( create_toolButton );
    create_toolButton->hide();

    spacer1 = new QSpacerItem( 30, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    buttonGroup1Layout->addItem( spacer1 );

    delete_toolButton = new ToolButtonWithText( buttonGroup1, "delete_toolButton" );
    delete_toolButton->setIconSet( QIcon( TestIcon::icon_delete ) );
    delete_toolButton->setText( tr("Delete"));
    buttonGroup1Layout->addWidget( delete_toolButton );

    spacer2 = new QSpacerItem( 30, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    buttonGroup1Layout->addItem( spacer2 );

    assign_toolButton = new ToolButtonWithText( buttonGroup1, "assign_toolButton" );
    assign_toolButton->setIconSet( QIcon( TestIcon::icon_choose ) );
    assign_toolButton->setText( tr("Assign") );
    buttonGroup1Layout->addWidget( assign_toolButton );

    QSpacerItem* spacer3 = new QSpacerItem( 30, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    buttonGroup1Layout->addItem( spacer3 );

    toolbtn_listview_layout->addWidget( buttonGroup1 );

    listView = new Q3ListView( this, "listView" );
    listView->addColumn( "TESTS                                                       ");
    listView->header()->setClickEnabled( FALSE, listView->header()->count() - 1 );
    listView->header()->setResizeEnabled( FALSE, listView->header()->count() - 1 );
    listView->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)7, 0, 0, listView->sizePolicy().hasHeightForWidth() ) );
    listView->setMaximumSize( QSize( 300, 32767 ) );
    listView->header()->hide();        

    /* if the user presses the Ctrl key when clicking on an item, the clicked item gets toggled and all other items are left untouched. 
       And if the user presses the Shift key while clicking on an item, all items between the current item and the clicked item get 
       selected or unselected, depending on the state of the clicked item. Also, multiple items can be selected by dragging the mouse over them. 
     */
    listView->setSelectionMode( Q3ListView::Extended );

    // Clicking a - icon closes the item (hides its children) and clicking a + icon opens the item;
    listView->setRootIsDecorated ( TRUE );

    toolbtn_listview_layout->addWidget( listView );

    PropAnaDiaLayout->addLayout( toolbtn_listview_layout, 0, 0 );

    prop_info_layout = new Q3VBoxLayout( 0, 0, 0, "prop_info_layout"); 

    head_lbl = new QLabel( this, "head_lbl" );
    head_lbl->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 10, 0, head_lbl->sizePolicy().hasHeightForWidth() ) );
    head_lbl->setMinimumSize( QSize( 0, 30 ) );
    head_lbl->setMaximumSize( QSize( 32767, 30 ) );
    head_lbl->setPaletteBackgroundColor( QColor( 85, 170, 255 ) );
    prop_info_layout->addWidget( head_lbl );

    about_wid = new QWidget;
    about_widLayout = new Q3HBoxLayout( about_wid, 11, 6, "about_widLayout");
    about_lbl = new QLabel( about_wid, "about_lbl" );
    about_widLayout->addWidget( about_lbl);

    prop_widstack = new Q3WidgetStack( this ); 
    prop_info_layout->addWidget( prop_widstack );
    PropAnaDiaLayout->addLayout( prop_info_layout, 0, 1 );

    /*  Create two type propagation model parameters dialog.   */
    expo_info_wid      = new ExponentialInfoWidClass( np );
    seg_info_wid       = new SegmentInfoWidClass( np );
#if HAS_CLUTTER
    clutter_info_wid   = new ClutterInfoWidClass( np );
#endif

    prop_widstack->addWidget( expo_info_wid );
    prop_widstack->addWidget( seg_info_wid );
#if HAS_CLUTTER
    prop_widstack->addWidget( clutter_info_wid );
#endif
    prop_widstack->addWidget( about_wid );

    generate_list(0);

    //signal and slot
    connect( create_toolButton->toolButton, SIGNAL( clicked() ),             this, SLOT( create_toolButton_clicked() ) );
    connect( delete_toolButton->toolButton, SIGNAL( clicked() ),             this, SLOT( delete_selected_model() ) );
    connect( assign_toolButton->toolButton, SIGNAL( clicked() ),             this, SLOT( assign_toolButton_clicked() ) );
    connect(    expo_info_wid, SIGNAL( modified() ), this, SLOT( entryEdited() ) );
    connect(     seg_info_wid, SIGNAL( modified() ), this, SLOT( entryEdited() ) );
    connect( clutter_info_wid, SIGNAL( modified() ), this, SLOT( entryEdited() ) );
    
    /* The signal of selectionChanged(QListViewItem*) is emitted only when the selected item has changed in Single selection mode.
     * If in Multi selection mode, we must use clicked(QListViewItem* ) signal.
     *******************************************************************************************************************************/
    connect( listView,        SIGNAL( currentChanged(Q3ListViewItem* ) ), this, SLOT( listView_selectchanged(Q3ListViewItem* ) ) );

    connect( buttonClose,      SIGNAL( clicked() ),                         this, SLOT(  close_btn_clicked() ) );
    connect( buttonCancel,     SIGNAL( clicked() ),                         this, SLOT( cancel_btn_clicked() ) );
    connect( buttonApply,      SIGNAL( clicked() ),                         this, SLOT(  apply_btn_clicked() ) );

    languageChange();
    resize( QSize(800, 350).expandedTo(minimumSizeHint() ) );

    state = GConst::viewState;
    updateStateChange();

    exec();
}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::~PropModMgrDia                                          ****/
/******************************************************************************************/
PropModMgrDia::~PropModMgrDia()
{
}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::updateStateChange                                       ****/
/******************************************************************************************/
void PropModMgrDia::updateStateChange()
{
    buttonClose ->setEnabled( (state == GConst::viewState) ? true : false );
    buttonCancel->setEnabled( (state == GConst::viewState) ? false : true );
    buttonApply ->setEnabled( (state == GConst::viewState) ? false : true );

    listView    ->setEnabled( (state == GConst::viewState) ? true : false );

    create_toolButton->setEnabled( (state == GConst::viewState) ? true : false );
    delete_toolButton->setEnabled( (state == GConst::viewState) ? true : false );
    assign_toolButton->setEnabled( (state == GConst::viewState) ? true : false );
}
/******************************************************************************************/
/**** FUNCTION: GenericInfoWidClass::entryEdited                                       ****/
/******************************************************************************************/
void PropModMgrDia::entryEdited()
{
    if (state == GConst::viewState) {
        state = GConst::editState;
        updateStateChange();
    }
}   
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::generate_list                                           ****/
/******************************************************************************************/
void PropModMgrDia::generate_list(int selected_idx)
{
    int prop_idx;
    PropModelListItem *pmli;
    PropModelListItem *sel_pmli = (PropModelListItem *) NULL;

    listView->blockSignals(true);

    while(listView->firstChild()) {
        delete(listView->firstChild());
    }

    for( prop_idx=0; prop_idx<=np->num_prop_model-1; prop_idx++ ) {
        pmli = new PropModelListItem(listView, prop_idx);
        pmli->setText(0, np->prop_model_list[prop_idx]->get_strid());
        if (prop_idx == selected_idx) {
            sel_pmli = pmli;
        }
    }

    if (sel_pmli) {
        sel_pmli->setSelected(true);
        listView->setCurrentItem(sel_pmli);
        listView_selectchanged(sel_pmli);
    } else {
        listView_selectchanged((PropModelListItem *) NULL);
    }

    listView->blockSignals(false);
}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::languageChange                                          ****/
/******************************************************************************************/
void PropModMgrDia::languageChange()
{
    setCaption( tr( "Propagation Model Manager" ) );

    buttonClose->setText( tr( "&Close" ) );
    buttonClose->setAccel( Qt::ALT+Qt::Key_C );

    buttonCancel->setText( tr( "Cancel" ) );
    buttonApply->setText( tr( "Apply" ) );

    buttonGroup1->setTitle( QString::null );

    create_toolButton->toolButton->setTextLabel( tr("Create Propagation Model") );
    create_toolButton->toolButton->setAutoRaise( true );

    delete_toolButton->toolButton->setTextLabel( tr("Delete Propagation Model") );
    delete_toolButton->toolButton->setAutoRaise( true );

    assign_toolButton->toolButton->setTextLabel( tr("Assign Propagation Model") );
    assign_toolButton->toolButton->setAutoRaise( true );

    about_wid->setCaption( tr( "About" ) );
    about_lbl->setText( "<h3><p align=\"center\">" + tr("Propagation Models") + "</p></h3>" );

    // listView->header()->setLabel( 0, tr( "xxxx" ) );

    // top_level_item->setText( 0, tr( "Propagation Models List" ) );
}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::create_toolButton_clicked                               ****/
/******************************************************************************************/
void PropModMgrDia::create_toolButton_clicked()
{
    prop_type_name_dia = new PropTypeNameDia( np, 0, 0, TRUE );
    connect( prop_type_name_dia->ok_pushButton, SIGNAL( clicked() ), this, SLOT( prop_type_name_dia_ok_btn_clicked() ) );

    prop_type_name_dia->exec();

}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::prop_type_name_dia_ok_btn_clicked                       ****/
/******************************************************************************************/
void PropModMgrDia::prop_type_name_dia_ok_btn_clicked()
{
    int pm_idx;

    m_prop_name = prop_type_name_dia->name_lineEdit->text();

    if ( prop_type_name_dia->expo_radioButton->isChecked() ) {
        m_prop_type = CConst::PropExpo;
    } else if ( prop_type_name_dia->seg_radioButton->isChecked() ) {
        m_prop_type = CConst::PropSegment;
#if HAS_CLUTTER
    } else if ( prop_type_name_dia->clt_radioButton->isChecked() ) {
        m_prop_type = CConst::PropClutterWtExpoSlope;
#endif
    } else {
        CORE_DUMP;
    }

    Q3CString qcs(2*m_prop_name.length());
    qcs = m_prop_name.local8Bit();

    int uniq_name = 1;    
    for (pm_idx=0; (pm_idx<=np->num_prop_model-1)&&(uniq_name); pm_idx++) {
        if (strcmp(np->prop_model_list[pm_idx]->get_strid(), (const char *) qcs)==0) {
            uniq_name = 0;
        }
    }

    if ( uniq_name ) {
        switch(m_prop_type) {
            case CConst::PropExpo:
                sprintf(np->line_buf, "create_prop_model -name %s -type expo", (const char *) qcs);
                np->process_command(np->line_buf);
                break;
            case CConst::PropSegment:
                sprintf(np->line_buf, "create_prop_model -name %s -type segment", (const char *) qcs);
                np->process_command(np->line_buf);
                break;
            default:
                CORE_DUMP;
                break;
        }
        delete prop_type_name_dia;
        generate_list(np->num_prop_model-1);
    } else {
        QMessageBox::warning( prop_type_name_dia, "Warning",
                              QString( tr("Propagation model name already exists")),
                              QMessageBox::Warning,
                              0
                            );
        // prop_type_name_dia->exec();
    }
}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::delete_selected_model                                   ****/
/******************************************************************************************/
void PropModMgrDia::delete_selected_model()
{
    int i;
    char *chptr;
    ListClass<int> *del_list = new ListClass<int>(1);

    PropModelListItem *pmli = (PropModelListItem *) listView->firstChild();
    while(pmli) {
        if (pmli->isSelected()) {
            del_list->append(pmli->getIndex());
        }
        pmli = (PropModelListItem *) pmli->nextSibling();
    }

    if (del_list->getSize()) {
        chptr = np->line_buf;
        chptr += sprintf(chptr, "delete_prop_model -pm_idx \'");
        for (i=0; i<=del_list->getSize()-1; i++) {
            pm_idx = (*del_list)[i];
            chptr += sprintf(chptr, " %d", pm_idx);
        }
        chptr += sprintf(chptr, " \' -force");
        np->process_command(np->line_buf);
    }

    pm_idx = (*del_list)[del_list->getSize()-1] + 1 - del_list->getSize();
    if (pm_idx > np->num_prop_model-1) {
        pm_idx = np->num_prop_model-1;
    }

    generate_list(pm_idx);
}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::assign_toolButton_clicked                               ****/
/******************************************************************************************/
void PropModMgrDia::assign_toolButton_clicked()
{
     /*
        not implement yet, maybe use it late; 
      */
    sector_prop_table = new SectorPropTable(np);
}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::listView_selectchanged                                  ****/
/******************************************************************************************/
void PropModMgrDia::listView_selectchanged( Q3ListViewItem* v_listitem )
{
    int i, num_row;
    QString str_coef, str, prop_name;
    Q3Header *th;
    PropModelClass *pm;
    ExpoPropModelClass *epm;
    SegmentPropModelClass *spm;
    ClutterExpoLinearPropModelClass *cpm;

    printf("CALL PropModMgrDia::listView_selectchanged() ");
    if (v_listitem == (Q3ListViewItem *) NULL) {
        printf("NULL");
    } else {
        printf("pm_idx = %d", ((PropModelListItem *) v_listitem)->getIndex());
    }
    printf("\n");

    if (v_listitem == (Q3ListViewItem *) NULL) {
        pm_idx = -1;
    } else {
        pm_idx = ((PropModelListItem *) v_listitem)->getIndex();
    }

    if (pm_idx != -1) {
        pm = np->prop_model_list[pm_idx];
        prop_name = QString(pm->get_strid());
        head_lbl->setText(prop_name);
        switch(pm->type()) {
            case CConst::PropExpo:
                prop_widstack->raiseWidget( expo_info_wid );
                epm = ((ExpoPropModelClass *) pm);
                expo_info_wid->setParam(epm);
                prop_widstack->raiseWidget( expo_info_wid );
                break;
            case CConst::PropSegment:
                prop_widstack->raiseWidget( seg_info_wid );
                spm = ((SegmentPropModelClass *) pm);
                seg_info_wid->setParam(spm);
                prop_widstack->raiseWidget( seg_info_wid );
                break;
#if HAS_CLUTTER
            case CConst::PropClutterExpoLinear:
                cpm = ((ClutterExpoLinearPropModelClass *) pm);

                /**************************************************************************/
                /**** Clutter Coefficient Table                                        ****/
                /**************************************************************************/
                num_row = clutter_info_wid->clutter_view_table->table->numRows();
                for ( i=num_row-1; i>=0; i-- ) {
                    clutter_info_wid->clutter_view_table->table->removeRow(i);
                }

                th = clutter_info_wid->clutter_view_table->table->verticalHeader();
                for (i=0; i<=cpm->num_clutter_type-1; i++ ) {
                    clutter_info_wid->clutter_view_table->table->insertRows(i, 1);
                    clutter_info_wid->clutter_view_table->table->setText(i, 0, QString("%1").arg(cpm->mvec_x[cpm->useheight+i]));
                    th->setLabel( i, QString("Clutter Type %1").arg(i) );
                }
                /**************************************************************************/

                prop_widstack->raiseWidget( clutter_info_wid );
                clutter_info_wid->setParam( cpm );
                break;
#endif
            default:
                prop_widstack->raiseWidget( about_wid );
                head_lbl->setText("Propagation Models");
                break;
        }
    } else {
        prop_widstack->raiseWidget( about_wid );
        head_lbl->setText("Propagation Models");
    }
}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::close_btn_clicked                                       ****/
/******************************************************************************************/
void PropModMgrDia::close_btn_clicked( )
{
    delete this;
}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::cancel_btn_clicked                                      ****/
/******************************************************************************************/
void PropModMgrDia::cancel_btn_clicked()
{
    // printf("CANCEL BUTTON CLICKED\n");

    QString s;

    s = "Are you sure you want to discard modifications?";

    if ( QMessageBox::question( this, "WiSim", s,
                                QMessageBox::Yes,
                                QMessageBox::No | QMessageBox::Default | QMessageBox::Escape,
                                Qt::NoButton
                              ) == QMessageBox::Yes ) {
        state = GConst::viewState;
        updateStateChange();
        listView->setSelected(listView->currentItem(), true);
        listView_selectchanged(listView->currentItem());
    }
}
/******************************************************************************************/
/**** FUNCTION: PropModMgrDia::apply_btn_clicked                                       ****/
/******************************************************************************************/
void PropModMgrDia::apply_btn_clicked( )
{
    PropModelClass *pm = np->prop_model_list[pm_idx];

    switch(pm->type()) {
        case CConst::PropExpo:
            expo_info_wid->applyParamEdit(pm_idx);
            break;
        case CConst::PropSegment:
            seg_info_wid->applyParamEdit(pm_idx);
            break;
        case CConst::PropClutterExpoLinear:
            clutter_info_wid->applyParamEdit(pm_idx);
            break;
        default:
            CORE_DUMP;
            break;
    }

    if (!np->error_state) {
        state = GConst::viewState;
        updateStateChange();
        listView->setSelected(listView->currentItem(), true);
        listView_selectchanged(listView->currentItem());
    } else {
        delete this;
    }
}
/******************************************************************************************/

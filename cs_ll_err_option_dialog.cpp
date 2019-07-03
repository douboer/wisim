/******************************************************************************************/
/**** PROGRAM: cs_ll_err_option_dialog.h                                               ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include "cs_ll_err_option_dialog.h"

#include <math.h>
#include <stdio.h>
#include <vector>

#include "road_test_data.h"
#include "WiSim.h"
#include "list.h"
#include "pref.h"
#include "phs.h"
#include "antenna.h"
#include "main_window.h"

#if HAS_GUI
#include "cs_lonlat_error_dialog.h"
#include "WiSim_gui.h"
#include <q3ptrlist.h>
#include <q3strlist.h>
#include <q3progressdialog.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <q3groupbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qmessagebox.h> 
#include <q3hbox.h>
#include <q3vbox.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3VBoxLayout>
#endif

using namespace std;

typedef vector<int> VEC_INT; 
typedef vector<long> VEC_LONG;

extern MainWindowClass *main_window;

void angle_range( ListClass<double>* angle, double rate, double& start_a, double& end_a );
int in_angle_range( double angle, double start_a, double end_a );
void add_error_cell_vector( VEC_LONG& new_num_vec, VEC_LONG& new_desc_vec,
                            vector<VEC_LONG>& dest_vec, unsigned& num_elem, int p );


CsLonlatErrorOptionDialog::CsLonlatErrorOptionDialog( QWidget * parent, const char * name )
                        :Q3Wizard( parent, name )
{
    setupPage1();
    setupPage2();
    setupPage3();
}

void CsLonlatErrorOptionDialog::setupPage1()
{
    
    page1 = new Q3VBox( this );
    
    page1->setSpacing(50);
    
    Q3ButtonGroup* bgroup = new Q3ButtonGroup( 1, Qt::Horizontal, "", page1 );
    bgroup->setExclusive( TRUE );
    rb1 = new QRadioButton( "CS position locating(recommended)", bgroup );
    rb2 = new QRadioButton( "CS position checking", bgroup );
    rb1->setChecked( TRUE );

    addPage( page1, "Option Wizard" );
    setNextEnabled( page1, TRUE );
//    setHelpEnabled( page1, TRUE );

}


void CsLonlatErrorOptionDialog::setupPage2()
{
  page2 = new Q3VBox( this );
    page2->setCaption("Check CS Error Option");

    Q3VBoxLayout* topLayout = new Q3VBoxLayout( page2 );
    topLayout->setMargin( 15 );
    topLayout->setSpacing( 15 );

    group1 = new Q3GroupBox(0,Qt::Horizontal,"Criterion 1",page2);
    group1->setCheckable(TRUE); 
    Q3HBoxLayout* group1Layout = new Q3HBoxLayout(group1->layout());
    group1Layout->setSpacing(10);
    QLabel* label1 = new QLabel("Distance  >=",group1);
    edit_distance1 = new QLineEdit("300",group1);
    QLabel* label2 = new QLabel("meters and RSSI  >=",group1);
    edit_RSSI1 = new QLineEdit("50",group1);
    QLabel* label3 = new QLabel("dBuV",group1);

    group1Layout->addWidget(label1);
    group1Layout->addWidget(edit_distance1);
    group1Layout->addWidget(label2);
    group1Layout->addWidget(edit_RSSI1);
    group1Layout->addWidget(label3);
    topLayout->addWidget(group1);

    group2 = new Q3GroupBox(0,Qt::Horizontal,"Criterion 2",page2);
    group2->setCheckable(TRUE); 
    Q3HBoxLayout* group2Layout = new Q3HBoxLayout(group2->layout());
    group2Layout->setSpacing(10);
    QLabel* label4 = new QLabel("Percentage of those farther but RSSI stronger  >=",group2);
    edit_percentage2 = new QLineEdit("50",group2);
    QLabel* label5 = new QLabel("%",group2);
    group2Layout->addWidget(label4);
    group2Layout->addWidget(edit_percentage2);
    group2Layout->addWidget(label5);

    topLayout->addWidget(group2);

    group3 = new Q3GroupBox(0,Qt::Vertical,"Criterion 3",page2);
    group3->setCheckable(TRUE); 
    Q3VBoxLayout* group3Layout = new Q3VBoxLayout(group3->layout());
    group3Layout->setAlignment( Qt::AlignTop );

    Q3HBoxLayout* group3Layout1 = new Q3HBoxLayout(group3Layout);
    group3Layout1->setSpacing(10);
    QLabel* label6 = new QLabel("Compute points from other CSs that are at least",group3);
    edit_distance3 = new QLineEdit("20",group3);
    QLabel* label7 = new QLabel("meters",group3);
    group3Layout1->addWidget(label6);
    group3Layout1->addWidget(edit_distance3);
    group3Layout1->addWidget(label7);

    QLabel* label8 = new QLabel("nearer to this CS than its own points",group3);
    group3Layout->addWidget(label8);

    topLayout->addWidget(group3);

/*  QHBoxLayout* okcancelLayout = new QHBoxLayout( topLayout );
    QPushButton* button_OK = new QPushButton( "OK", page2 );
    QPushButton* button_Cancel = new QPushButton("Cancel",page2);
    QSpacerItem* blank = new QSpacerItem( width()/4, button_OK->height() );
    okcancelLayout->addItem(blank);
    okcancelLayout->addWidget(button_OK);
    okcancelLayout->addWidget(button_Cancel);*/

    page2->resize(1,1);

/*  connect( button_OK, SIGNAL(clicked()), this, SLOT(onOKClick()) );
    connect( button_Cancel, SIGNAL(clicked()), this, SLOT(close()) );   
*/  
    addPage( page2, "Check Options" ); 
    
    setNextEnabled( page2, FALSE );
    setFinishEnabled( page2, TRUE );
}


void CsLonlatErrorOptionDialog::setupPage3()
{
    page3 = new Q3VBox( this );
    addPage( page3, "Locating Options" );
    page3->setSpacing(50);
    QLabel* label1 = new QLabel("Click \"Finish\" to run CS position locating.", page3);
    QLabel* label2 = new QLabel("Report file can be found in the same directory as the geometry file.", page3);
    
    setNextEnabled( page3, FALSE );
    setFinishEnabled( page3, TRUE );
}


void CsLonlatErrorOptionDialog::next()
{
    if( rb1->isChecked() ) 
        showPage( page2 );
    else 
        showPage( page1 );
    Q3Wizard::next(); 
        
    setNextEnabled( currentPage(), FALSE );
    setFinishEnabled( currentPage(), TRUE );

}


void CsLonlatErrorOptionDialog::back()
{
    showPage( page1 );
    Q3Wizard::back(); 
}

       
void CsLonlatErrorOptionDialog::accept()
{
    if( !(group1->isChecked()) && !(group2->isChecked()) && !(group3->isChecked()) ) {
            QMessageBox::warning( this, "Error",
                QString("At least one criterion must be selected" ) );
            return;
        }
 
    c1_thre_RSSI = (int)((edit_RSSI1->text()).toDouble());
    c1_thre_distance = (edit_distance1->text()).toDouble();
    c2_thre_percentage = (edit_percentage2->text()).toDouble()/100.0;
    c3_thre_dis_diff = (int)((edit_distance3->text()).toDouble());

    check_cs_error();   
}


CsLonlatErrorOptionDialog::~CsLonlatErrorOptionDialog()
{
}


void CsLonlatErrorOptionDialog::check_cs_error()
{
    int rtd_idx, cell_idx;
    RoadTestPtClass *rtp;
    SectorClass *sector;

    VEC_INT vec_one_cell_rtd;
    vector<VEC_INT> vec_cell_rtd;
    unsigned vec_idx;

    vector<VEC_LONG> error_cell;

#if HAS_GUI
    Q3ProgressDialog* pd = new Q3ProgressDialog( "Operation in progress.", "Cancel", 100, 0, 0, true );    
    pd->show();
#endif
    vec_cell_rtd.resize( np->num_cell );
    for( rtd_idx=0; rtd_idx<=np->road_test_data_list->getSize()-1; rtd_idx++ ) {
        rtp = &( (*np->road_test_data_list)[rtd_idx] );
        vec_cell_rtd[rtp->cell_idx].push_back( rtd_idx );
    }

    int dbuv_to_dbm = -113;
    long num_abnorm = 0;
    long num_norm=0;

    vector<long> vec_error_cell_num1;
    vector<long> vec_error_cell_desc1;
    vector<long> vec_error_cell_desc1_1;

/*****
    the first criteria : distance > 300m and power > 50 dBuV
****/
    if(rb2->isChecked()) {
        if(group1->isChecked()) {
            bool display_chosen1 = true;
            double percentage1;
            double percentage_thre1=0.0;
            double points_thre1=1;
        
            c1_thre_RSSI = c1_thre_RSSI + dbuv_to_dbm;
        
            for( cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++ ) {
        
#if HAS_GUI
                if( pd->wasCanceled() )  return;
                char label[100];
                sprintf( label, "checking cell_%-4d using critetia 1\n", cell_idx );
                pd->setLabelText( label );
                pd->setProgress( (int)(100*(double)(((double)cell_idx+1.0)/(double)np->num_cell)) );
                qApp->processEvents();
#endif

                num_abnorm = 0;
                num_norm = 0;
                for( vec_idx=0; vec_idx+1<=vec_cell_rtd[cell_idx].size(); vec_idx++ ) {
                    rtd_idx = vec_cell_rtd[cell_idx][vec_idx];
                    rtp = &( (*(np->road_test_data_list))[rtd_idx] );
                    int x0 = (np->cell_list)[cell_idx]->posn_x;
                    int y0 = (np->cell_list)[cell_idx]->posn_y;
                    int x1 = rtp->posn_x;
                    int y1 = rtp->posn_y;
                    double dis = sqrt((double)( (x0-x1)*(x0-x1) + (y0-y1)*(y0-y1) ));
                    dis *= np->resolution;
                    if( rtp->pwr_db > c1_thre_RSSI ) {
                        if( dis>c1_thre_distance )
                            num_abnorm++;
                        else
                            num_norm++;
                    }
                }
                if( num_abnorm+num_norm > 0 ) 
                    percentage1 =(double) num_abnorm/(double)(num_abnorm+num_norm);
                else 
                    percentage1 = 0.0;
        
                if( num_abnorm>= points_thre1 && percentage1>=percentage_thre1 ) {
                    vec_error_cell_num1.push_back( cell_idx );
                    vec_error_cell_desc1.push_back( num_abnorm );
                    vec_error_cell_desc1_1.push_back( (int)(10000*percentage1));
                }
            }
        }//end check1
/****
    the second criteria: 
        for each cell, compares every 2 rtps: 
        if (rtp1_dis>rtp2_dis && rtp1_pwr>rtp2_pwr ) 
        then this pair is considered as abnormal
        else considered as normal
        if the number of the former is greater than the number of the latter( or use a percentage to describe )
        then this cell is suspected as LonLat error 
****/
        vector<long> vec_error_cell_num2;
        vector<long> vec_error_cell_desc2;

        if(group2->isChecked()) {
            bool display_chosen2 = true;
            
        //    double percentage_thre2 = 0.5;
            double percentage2;
            unsigned vec_idx2;
            int rtd_idx2;
            RoadTestPtClass *rtp2;
        
            for( cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++ ) {
#if HAS_GUI
                if( pd->wasCanceled() )  return;
                char label[100];
                sprintf( label, "checking cell_%-4d using critetia 2\n", cell_idx );
                pd->setLabelText( label );
                pd->setProgress( (int)(100*(double)(((double)cell_idx+1.0)/(double)(np->num_cell))) );
                qApp->processEvents();
#endif

                num_abnorm = 0;
                num_norm = 0;
                for( vec_idx=0; vec_idx+2<=vec_cell_rtd[cell_idx].size(); vec_idx++ ) {
                    for( vec_idx2=vec_idx+1; vec_idx2+1<=vec_cell_rtd[cell_idx].size(); vec_idx2++ ) {
                        rtd_idx  = vec_cell_rtd[cell_idx][vec_idx];
                        rtd_idx2 = vec_cell_rtd[cell_idx][vec_idx2];
                        rtp  = &( (*(np->road_test_data_list))[rtd_idx] );
                        rtp2 = &( (*(np->road_test_data_list))[rtd_idx2] );
                        int x0 = (np->cell_list)[cell_idx]->posn_x;
                        int y0 = (np->cell_list)[cell_idx]->posn_y;
                        int x1 = rtp ->posn_x;
                        int y1 = rtp ->posn_y;
                        int x2 = rtp2->posn_x;
                        int y2 = rtp2->posn_y;
                        double dis  = sqrt((double)( (x0-x1)*(x0-x1) + (y0-y1)*(y0-y1) ));
                        double dis2 = sqrt((double)( (x0-x2)*(x0-x2) + (y0-y2)*(y0-y2) ));
                        dis *= np->resolution;
                        dis2*= np->resolution;
                        if( (dis - dis2) * (rtp->pwr_db - rtp2->pwr_db) > 0 )
                            num_abnorm++;
                        else 
                            num_norm++;
                    }
                }
                if( num_abnorm+num_norm > 0 ) 
                    percentage2 =(double) num_abnorm/(double)(num_abnorm+num_norm);
                else 
                    percentage2 = 0.0;
                if( percentage2 >= c2_thre_percentage ) {
                    vec_error_cell_num2.push_back( cell_idx );
                    vec_error_cell_desc2.push_back( (int)(10000*percentage2) );
                }
            }
        }//end check2

/****
    the third criteria:
        for each cell, if on the RT track, 
        RTPs distribute at farther position rather than nearer position
        this cell is suspected as LonLat error
        However, for a cell with direcitonal antenna, 
        additional condition should be taken into consideration.
****/
        vector<long> vec_error_cell_num3;
        vector<long> vec_error_cell_desc3;

        if(group3->isChecked()) {
            int display_chosen3 = 2;
        
            int rtd_all_idx;
            int sector_idx;
            double min_dis_own; 
            
            unsigned c3_thre_rtd_num = 0; //cell with num_RTP no more than c3_thre_num will not be checked
            int c3_thre_abnorm_num = 10;  //cell with abnorm RTP no less than c3_thre_abnorm_num will be picked out
            //int c3_thre_dis_diff = 20;    //dis2: distance of RTPs from other cells
                                          //if( dis2 < min_dis_own-c3_thre_dis_diff ), this RTP is flagged as abnormal RTP
            double start_a, end_a, range_a;
            ListClass<double>* angle_list = new ListClass<double>(10);
            double rate = 0.9;
               
            for( cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++ ) {

#if HAS_GUI
                if( pd->wasCanceled() )  return;
                char label[100];
                sprintf( label, "checking cell_%-4d using critetia 3\n", cell_idx );
                pd->setLabelText( label );
                pd->setProgress( (int)(100*(double)(((double)cell_idx+1.0)/(double)(np->num_cell))) );
                qApp->processEvents();
#endif

                min_dis_own = 10000.0;
                num_abnorm = 0;
                if( vec_cell_rtd[cell_idx].size() <= c3_thre_rtd_num )  
                    continue;
                int x0 = (np->cell_list)[cell_idx]->posn_x;
                int y0 = (np->cell_list)[cell_idx]->posn_y;
                double angle = 0;

                for( sector_idx=0; sector_idx<=(np->cell_list)[cell_idx]->num_sector-1; sector_idx++ ) {
                    sector = (np->cell_list)[cell_idx]->sector_list[sector_idx];
                    int is_omni = (np->antenna_type_list)[sector->antenna_type]->get_is_omni();
                    for( vec_idx=0; vec_idx+1<=vec_cell_rtd[cell_idx].size(); vec_idx++ ) {           
                        rtd_idx  = vec_cell_rtd[cell_idx][vec_idx];
                        rtp  = &( (*(np->road_test_data_list))[rtd_idx] );
                        int x1 = rtp ->posn_x;
                        int y1 = rtp ->posn_y;
                        double dis  = sqrt((double)( (x0-x1)*(x0-x1) + (y0-y1)*(y0-y1) ));
                        if( !is_omni ) {
                            angle = get_angle( x0, y0, x1, y1 );
                            angle_list->append( angle );
                        }
                        min_dis_own = dis<min_dis_own ? dis : min_dis_own;
                    }
                    if( !is_omni ) {
                        //calculate the angle range of the sector's RTPs
                        angle_range( angle_list, rate, start_a, end_a );
                    //    printf( "angle range of cell_%d: (%f, %f)\t", cell_idx, start_a, end_a );
                    
                        angle_list->reset();
        
                        //adjust the angle_range to PI/4 at least    
                        if( start_a <= end_a ) 
                            range_a = end_a-start_a;
                        else 
                            range_a = 2.0*PI-end_a+start_a;   
                        if( range_a < PI/4.0 ) {
                            start_a -= (PI/4.0-range_a)/2.0;
                            start_a = start_a<0 ? start_a+2.0*PI : start_a;
                            end_a += (PI/4.0-range_a)/2.0;
                            end_a = end_a>=2.0*PI ? end_a-2.0*PI : end_a;
                        }
                        //printf( "adjusted: (%f, %f)\n", start_a, end_a );
                    }
                    //find those RTPs in ALL RT data:
                    //    1, distances between them and the cell are less than the min_dis of the cells' own RTPs.
                    //    2, in angle_range, if the sector has a directional antenna.       
                    for( rtd_all_idx=0; rtd_all_idx<=(np->road_test_data_list)->getSize()-1; rtd_all_idx++ ) {
                        rtp = &( (*(np->road_test_data_list))[rtd_all_idx] );
                        int x2 = rtp->posn_x;
                        int y2 = rtp->posn_y;
                        double dis2 = sqrt((double)( (x0-x2)*(x0-x2) + (y0-y2)*(y0-y2) ));
                        angle = get_angle( x0, y0, x2, y2 );
                        if( dis2 < min_dis_own-c3_thre_dis_diff/np->resolution ) {
                            if( !is_omni ) {
                                   angle = get_angle( x0, y0, x2, y2 );
                                   if( !in_angle_range(angle, start_a, end_a) )
                                       continue;
                            }
                            num_abnorm++;
                        }
                    }
                }
                if( num_abnorm >= c3_thre_abnorm_num ) {
                    vec_error_cell_num3.push_back( cell_idx );
                    vec_error_cell_desc3.push_back( num_abnorm );
                }
            }
        }//end check3
    
        error_cell.resize( vec_error_cell_num1.size() + vec_error_cell_num2.size() + vec_error_cell_num3.size() );
        unsigned num_error_cell = 0;
        for( unsigned i=0; i<vec_error_cell_num1.size(); i++ ) {
            error_cell[i].resize( 4 );
            error_cell[i][0] = vec_error_cell_num1[i];
            error_cell[i][1] = vec_error_cell_desc1[i];
            error_cell[i][2] = 0;
            error_cell[i][3] = 0;
            num_error_cell++;
        }
        add_error_cell_vector( vec_error_cell_num2, vec_error_cell_desc2, error_cell, num_error_cell, 2 );
        add_error_cell_vector( vec_error_cell_num3, vec_error_cell_desc3, error_cell, num_error_cell, 3 );
        error_cell.resize( num_error_cell );    
    
    #if HAS_GUI
        QString label_c1 = "";
        QString label_c2 = "";
        QString label_c3 = "";
        if(group1->isChecked())
            label_c1 = "Criterion 1: Distance >= " + edit_distance1->text() + " meters and RSSI >= " + edit_RSSI1->text() + " dBuV\n";
        if(group2->isChecked())
            label_c2 = "Criterion 2: Percentage of those farther but RSSI stronger >= " + edit_percentage2->text() + "%\n";
        if(group3->isChecked())
            label_c3 = "Criterion 3: Compute points from other CSs that are at least " + edit_distance3->text() + 
                           " meters\n\t   nearer to this CS than its own points";
        CsLonlatErrorDialog* error_dialog = new CsLonlatErrorDialog( error_cell,QString(label_c1+label_c2+label_c3),np, 0, 0 );
        error_dialog->show();
    #endif
    }


    /****
    CS locating
    ****/
    else {
        int c4_thre_RSSI = 40;
        c4_thre_RSSI = c4_thre_RSSI + dbuv_to_dbm;
        int c4_thre_num_rtp = 0;

        FILE* fp_cs_check;
        if( NULL==(fp_cs_check = fopen("cs_position_check.txt","w+")) ) {
            sprintf(np->msg,"Cannot open output file");
            PRMSG(stdout,np->msg);
            return;
        }

        fprintf(fp_cs_check, "CS_INDEX\tGW-CSC-CS\tCSID\tDISTANCE(meter)\tposn_x(adjusted)\tposn_y(adjusted)\n");

        for( cell_idx=0; cell_idx<=np->num_cell-1; cell_idx++ ) {
#if HAS_GUI
            if( pd->wasCanceled() )  return;
            char label[100];
            sprintf( label, "checking cell_%-4d using critetia 4\n", cell_idx );
            pd->setLabelText( label );
            pd->setProgress( (int)(100*(double)(((double)cell_idx+1.0)/(double)(np->num_cell))) );
            qApp->processEvents();
#endif
    
            double x=0;
            double y=0;
            int num_high_sig=0;
            double power_all=0.0;
    
            int x0 = (np->cell_list)[cell_idx]->posn_x;
            int y0 = (np->cell_list)[cell_idx]->posn_y;
    
            for( vec_idx=0; vec_idx+1<=vec_cell_rtd[cell_idx].size(); vec_idx++ ) {
                rtd_idx = vec_cell_rtd[cell_idx][vec_idx];
                rtp = &( (*(np->road_test_data_list))[rtd_idx] );
                if( rtp->pwr_db <= c4_thre_RSSI ) 
                    continue;
                if( rtp->posn_x<0 || rtp->posn_x>100000 || rtp->posn_y<0 || rtp->posn_y>100000 )
                  continue;
    
                double x1 = rtp->posn_x;
                double y1 = rtp->posn_y;
    
                //double power=((rtp->pwr_db-c4_thre_RSSI)/5.0+1.0)*((rtp->pwr_db-c4_thre_RSSI)/5.0+1.0)*((rtp->pwr_db-c4_thre_RSSI)/5.0+1.0);
                double power=pow(1.3, (rtp->pwr_db-c4_thre_RSSI));
    
                x+=power*(double)x1;
                y+=power*(double)y1;
                power_all+=power;
                  
                num_high_sig++;

            }
            if(num_high_sig<=c4_thre_num_rtp) {
                fprintf(fp_cs_check,"%d\tnum_rtp(RSSI>%d)<=%d\n",cell_idx,c4_thre_RSSI-dbuv_to_dbm,c4_thre_num_rtp);
            }
            else {
                double avg_x = x/power_all;
                double avg_y = y/power_all;
                double dis = sqrt((double)( (avg_x-x0)*(avg_x-(double)x0) + (avg_y-y0)*(avg_y-(double)y0) ))*np->resolution;
      
                char *hexstr = CVECTOR(2*PHSSectorClass::csid_byte_length);
                char *gwcsccsstr = CVECTOR(6);
                PHSSectorClass * sector = (PHSSectorClass *)(np->cell_list[cell_idx]->sector_list[0]);
                hex_to_hexstr(hexstr, ((PHSSectorClass *) sector)->csid_hex, ((PHSSectorClass *) sector)->csid_byte_length);
                sprintf(gwcsccsstr, "%.6d", ((PHSSectorClass *) sector)->gw_csc_cs);
    
                char pri[100];
    
                if(dis<50.0)
                    strcpy(pri, "");
                else if(dis<100.0)
                    strcpy(pri, "Low");
                else if(dis<150.0)
                    strcpy(pri, "Middle");              
                else
                    strcpy(pri, "High priority");
                    
                fprintf(fp_cs_check,"%d\t%s\t%s\t%d\t%ld\t%ld\t%s\n",cell_idx,gwcsccsstr,hexstr,
                    (int)dis,(long)(np->idx_to_x(avg_x)), (long)(np->idx_to_y(avg_y)), pri );
                    
    
                //draw the cs location
                main_window->editor->vec_cs_new_x.push_back(avg_x);
                main_window->editor->vec_cs_new_y.push_back(avg_y);
                main_window->editor->vec_cs_index.push_back(cell_idx);
                main_window->editor->vec_dis.push_back(dis);
            }
        }

        std::cout << "main_window->editor->vec_cs_new_x.size() " + main_window->editor->vec_cs_new_x.size() << std::endl;

        //main_window->editor->if_show_new_location = 1;
        main_window->editor->show_cs_new_location();

        fclose(fp_cs_check);
    }//end cs locating

    hide();
}


void angle_range( ListClass<double>* angle, double rate, double& start_a, double& end_a )
{
    if( angle->getSize() <= 0 ) {
        printf( "error in angle_range(): vector has no elements\n" );
        return;
    }
    if( rate < 0.5 ) {
        printf( "error in angle_range(): rate must >= 0.5\n" );
        return;
    }
    if( angle->getSize() == 1 ) {
        start_a = (*angle)[0];
        end_a = (*angle)[0];
    }

    ListClass<double> angle_range_list(10);
//    vector<double> vec_angle_range;
//    vector<double>::iterator min_angle_range_pos;
    double min_angle_range = 2*PI;
    int min_angle_range_idx = 0;
    int angle_idx;

    (*angle).sort();
    for( angle_idx=0; angle_idx<=angle->getSize()-1; angle_idx++ ) {
        if( angle_idx+(int)(angle->getSize()*rate) <= angle->getSize() )
            angle_range_list.append( (*angle)[angle_idx+(int)(angle->getSize()*rate)-1] - (*angle)[angle_idx] );
        else
            angle_range_list.append( 2.0*PI-(*angle)[angle_idx]+
                                       (*angle)[angle_idx+(int)(angle->getSize()*rate)-1-angle->getSize()] );
        if( angle_range_list[angle_idx] < min_angle_range )  {
            min_angle_range = angle_range_list[angle_idx];
            min_angle_range_idx = angle_idx;
        }
    }
    
//    min_angle_range_pos = min_element( vec_angle_range.begin(), vec_angle_range.end() );
//    min_angle_range_idx = min_angle_range_pos - vec_angle_range.begin();
    start_a = (*angle)[min_angle_range_idx];
    if( min_angle_range_idx+(int)(angle->getSize()*rate) <= angle->getSize() )
        end_a = (*angle)[ min_angle_range_idx+(int)(angle->getSize()*rate)-1];
    else
        end_a = (*angle)[ min_angle_range_idx+(int)(angle->getSize()*rate)-1-angle->getSize()];
}


int in_angle_range( double angle, double start_a, double end_a )
{
    if( start_a <= end_a )
        return (angle>=start_a && angle<=end_a) ? 1 : 0;
    else
        return (angle>=start_a || angle<=end_a) ? 1 : 0;
} 


void add_error_cell_vector( VEC_LONG& new_num_vec, VEC_LONG& new_desc_vec, 
                            vector<VEC_LONG>& dest_vec, unsigned& num_elem, int p ) 
{
    unsigned i, j;
    int pos;
    for( i=0; i<new_num_vec.size(); i++ ) {
        pos = -1;
        for( j=0; j<num_elem; j++ ) {
            if( new_num_vec[i] == dest_vec[j][0] ) {
                pos = j;
                break;
            }
        }
        if( pos >= 0 )  //add error_desc to dest_vec[pos]
            dest_vec[pos][p] = new_desc_vec[i];
        else { //append to the dest_vec
            dest_vec[num_elem].resize( 4 );
            dest_vec[num_elem][0] = new_num_vec[i];
            dest_vec[num_elem][1] = 0;
            dest_vec[num_elem][2] = 0;
            dest_vec[num_elem][3] = 0;
            dest_vec[num_elem][p] = new_desc_vec[i];
            num_elem++;
        }
    }
}




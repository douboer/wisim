/******************************************************************************************/
/**** PROGRAM: cs_lonlat_error_dialog.cpp                                              ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include "cs_lonlat_error_dialog.h"
#include "phs.h"
#include <q3table.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <q3filedialog.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <vector>

using namespace std;

vector<VEC_LONG> cs_error_list1;

int sort_compare1( const void *arg1, const void *arg2 ) //original, first by criteria, second by cell index
{
  int index1 = *((int*)arg1);
  int index2 = *((int*)arg2);
  if ( index1>index2 )
	  return 1;
  else if ( index1<index2 )
	  return -1;
  else
	  return 0;
}

int sort_compare2( const void *arg1, const void *arg2 ) //by cell index
{
  int index1 = *((int*)arg1);
  int index2 = *((int*)arg2);
  if ( cs_error_list1[index1][0]>cs_error_list1[index2][0] )
	  return 1;
  else if ( cs_error_list1[index1][0]<cs_error_list1[index2][0] )
	  return -1;
  else
	  return 0;
}

int sort_compare3( const void *arg1, const void *arg2 ) //by criterion1, then cell index
{
  int index1 = *((int*)arg1);
  int index2 = *((int*)arg2);
  if ( cs_error_list1[index1][1]<cs_error_list1[index2][1] )
	  return 1;
  else if ( cs_error_list1[index1][1]>cs_error_list1[index2][1] )
	  return -1;
  else {
	  if ( cs_error_list1[index1][0]>cs_error_list1[index2][0] )
		  return 1;
	  else if ( cs_error_list1[index1][0]<cs_error_list1[index2][0] )
		  return -1;
	  else return 0;
  }
}

int sort_compare4( const void *arg1, const void *arg2 ) //by by criterion2, then cell index
{
  int index1 = *((int*)arg1);
  int index2 = *((int*)arg2);
  if ( cs_error_list1[index1][2]<cs_error_list1[index2][2] )
	  return 1;
  else if ( cs_error_list1[index1][2]>cs_error_list1[index2][2] )
	  return -1;
  else {
	  if ( cs_error_list1[index1][0]>cs_error_list1[index2][0] )
		  return 1;
	  else if ( cs_error_list1[index1][0]<cs_error_list1[index2][0] )
		  return -1;
	  else return 0;
  }
}

int sort_compare5( const void *arg1, const void *arg2 ) //by by criterion3, then cell index
{
  int index1 = *((int*)arg1);
  int index2 = *((int*)arg2);
  if ( cs_error_list1[index1][3]<cs_error_list1[index2][3] )
	  return 1;
  else if ( cs_error_list1[index1][3]>cs_error_list1[index2][3] )
	  return -1;
  else {
	  if ( cs_error_list1[index1][0]>cs_error_list1[index2][0] )
		  return 1;
	  else if ( cs_error_list1[index1][0]<cs_error_list1[index2][0] )
		  return -1;
	  else return 0;
  }
}

int sort_compare6( const void *arg1, const void *arg2 ) //Intelligent sort
{
  int index1 = *((int*)arg1);
  int index2 = *((int*)arg2);
  int i;
  double score1=0.0;
  double score2=0.0;

  for(i=1;i<4;i++) {
	  if (cs_error_list1[index1][i]!=0)
	    score1+=50.0;
  }
  if( cs_error_list1[index1][1] <50 )
	  score1+=(double)(cs_error_list1[index1][1]);
  else
	  score1+=50.0;
  score1+=(double)(cs_error_list1[index1][2]/200.0);
  if( cs_error_list1[index1][3] <1000 )
	  score1+=(double)(cs_error_list1[index1][3]/20.0);
  else
	  score1+=50.0;

  for(i=1;i<4;i++) {
	  if (cs_error_list1[index2][i]!=0)
	    score2+=50.0;
  }
  if( cs_error_list1[index2][1] <50 )
	  score2+=(double)(cs_error_list1[index2][1]);
  else
	  score2+=50.0;
  score2+=(double)(cs_error_list1[index2][2]/200.0);
  if( cs_error_list1[index2][3] <1000 )
	  score2+=(double)(cs_error_list1[index2][3]/20.0);
  else
	  score2+=50.0;

  if ( score1<score2 )
	  return 1;
  else if ( score1>score2 )
	  return -1;
  else {
	  score1=(double)(cs_error_list1[index1][1])+(double)(cs_error_list1[index1][2]/200.0)+(double)(cs_error_list1[index1][3]/20.0);
      score2=(double)(cs_error_list1[index2][1])+(double)(cs_error_list1[index2][2]/200.0)+(double)(cs_error_list1[index2][3]/20.0);
	  if (score1<score2)
		  return 1;
	  else
		  return -1;
  }
}

CsLonlatErrorDialog::CsLonlatErrorDialog( vector<VEC_LONG>& cell_error_list, QString StrLabel, NetworkClass* network_p, QWidget * parent, const char * name )
                        :QDialog( parent, name ), cs_error_list(cell_error_list), criteria_desc(StrLabel), np(network_p)
{
    cs_error_list1 = cs_error_list;
    setCaption( "Suspect LonLat Error Cell List" );
    
    Q3VBoxLayout* topLayout = new Q3VBoxLayout( this );
    topLayout->setMargin( 20 );
    topLayout->setSpacing( 20 );

/*    QLabel* lb1 = new QLabel( "criterion 1: long distance, strong signal\n"
                              "criterion 2: longer distance, stronger signal\n"
                              "criterion 3: no points on nearer track\n"
                              "             (display num_points from other cells)", this );
*/
    QLabel* lb1 = new QLabel( StrLabel, this );

    topLayout->addWidget( lb1 );

	QString str_lb2;
	str_lb2.sprintf("Num_CS_Error: %d", cs_error_list.size() );
	QLabel* lb3 = new QLabel( str_lb2, this );

    topLayout->addWidget( lb3 );
    
    cs_error_table = new Q3Table( cs_error_list.size(), 6, this, name );   
    (cs_error_table->horizontalHeader())->setLabel( 0, tr( "Cell_idx" ) );
	(cs_error_table->horizontalHeader())->setLabel( 1, tr( "GW-CSC\n-CS" ) );
	(cs_error_table->horizontalHeader())->setLabel( 2, tr( "CSID" ) );
    (cs_error_table->horizontalHeader())->setLabel( 3, tr( "Criterion1\n(points)" ) );
    (cs_error_table->horizontalHeader())->setLabel( 4, tr( "Criterion2\n(%)" ) );
    (cs_error_table->horizontalHeader())->setLabel( 5, tr( "Criterion3\n(points)" ) );
    cs_error_table->setLeftMargin( 0 );
    cs_error_table->adjustColumn( 0 );
    cs_error_table->adjustColumn( 1 );
    cs_error_table->adjustColumn( 2 );
    cs_error_table->adjustColumn( 3 );
	cs_error_table->adjustColumn( 4 );
	cs_error_table->adjustColumn( 5 );
    cs_error_table->adjustSize();

//    cs_error_table->setSorting( true );
 
    topLayout->addWidget( cs_error_table );

    QLabel* lb2 = new QLabel( "Sort by:", this );
    QComboBox* sortBox = new QComboBox( this );
	sortBox->insertItem( "Intelligent sort" );
    sortBox->insertItem( "Criteria, Cell index" );
    sortBox->insertItem( "Cell index" );
    sortBox->insertItem( "Criterion1, Cell index" );
    sortBox->insertItem( "Criterion2, Cell index" );
    sortBox->insertItem( "Criterion3, Cell index" );

    connect( sortBox, SIGNAL(activated(const QString&)), this, SLOT(onSort(const QString&)) );

    Q3HBoxLayout* sortLayout = new Q3HBoxLayout( topLayout, 5 );
    sortLayout->addWidget( lb2 );
    sortLayout->addWidget( sortBox );
    sortLayout->setStretchFactor( lb2, 20 );
    sortLayout->setStretchFactor( sortBox, 50 );

    QPushButton* button_save = new QPushButton( "Save", this );
    QPushButton* button_exit = new QPushButton( "Exit", this );
    QSpacerItem* blank = new QSpacerItem( width()/10, button_save->height() );

    Q3HBoxLayout* saveexitLayout = new Q3HBoxLayout( topLayout, 20 );
    saveexitLayout->addItem( blank );
    saveexitLayout->addWidget( button_save );
    saveexitLayout->addWidget( button_exit );

    connect( button_save, SIGNAL(clicked()), this, SLOT(onSaveClick()) );
    connect( button_exit, SIGNAL(clicked()), this, SLOT(close()) );

	sorted_table_index = (int*)malloc(cs_error_list.size()*(sizeof(int)));
	//initialize sorted_index
	for( unsigned int i=0; i<cs_error_list.size(); i++ ) 
		sorted_table_index[i] = i;

	//intelligent sort
	qsort(sorted_table_index,cs_error_list.size(),sizeof(int),sort_compare6 );
	sortTable();

	int newh = sizeHint().height()>600? 600: sizeHint().height();
	resize(sizeHint().width()+20, newh);


}


CsLonlatErrorDialog::~CsLonlatErrorDialog()
{
}


void CsLonlatErrorDialog::onSaveClick()
{
    FILE *outfile;
    QString outfilename = Q3FileDialog::getSaveFileName(
                       "",
                       "text file (*.txt)",
                       this,
                       "save file dialog"
                       "Choose a file to save" );

	if( outfilename.isEmpty() || outfilename.isNull() )
        return;

    //if user doesnot enter a suffix, add ".txt" automatically
    printf( ".....%d", outfilename.find( '.', 0 ) );
    if( outfilename.find( '.', 0 ) == -1 ) {
        outfilename += ".txt";
    }

    if( (outfile = fopen( outfilename, "w+" )) == NULL ) {
        QMessageBox::warning( this, "Error", "Open output file error" );
        return;
    }

	fprintf( outfile, criteria_desc );
    fprintf( outfile, "\nnum_cs_error: %d\n", cs_error_list.size() );
    fprintf( outfile, "CELL_INDEX\tGW-CSC-CS\tCSID\tCRITERION1\tCRITERION2\tCRITERION3\n" );
    fflush( outfile );


    for( unsigned int i=0; i<cs_error_list.size(); i++ ) {
		int index = sorted_table_index[i];
		char *hexstr = CVECTOR(2*PHSSectorClass::csid_byte_length);
		char *gwcsccsstr = CVECTOR(6);
		PHSSectorClass * sector = (PHSSectorClass *)(np->cell_list[cs_error_list[index][0]]->sector_list[0]);
		hex_to_hexstr(hexstr, ((PHSSectorClass *) sector)->csid_hex, ((PHSSectorClass *) sector)->csid_byte_length);
		sprintf(gwcsccsstr, "%.6d", ((PHSSectorClass *) sector)->gw_csc_cs);
        fprintf( outfile, "%ld\t%s\t%s\t%ld\t%ld\t%ld\n", cs_error_list[index][0], gwcsccsstr, hexstr, cs_error_list[index][1],
                 cs_error_list[index][2], cs_error_list[index][3] ); 
    }
    fclose( outfile );
}

void CsLonlatErrorDialog::onSort( const QString& selection )
{
	if( selection == "Criteria, Cell index" ) {
		qsort(sorted_table_index,cs_error_list.size(),sizeof(int),sort_compare1 );
		sortTable();
	}
	else if( selection == "Cell index" ) {
		qsort(sorted_table_index,cs_error_list.size(),sizeof(int),sort_compare2 );
		sortTable();
	}
	else if ( selection == "Criterion1, Cell index" ) {
		qsort(sorted_table_index,cs_error_list.size(),sizeof(int),sort_compare3 );
		sortTable();
	}
	else if ( selection == "Criterion2, Cell index" ) {
		qsort(sorted_table_index,cs_error_list.size(),sizeof(int),sort_compare4 );
		sortTable();
	}
	else if ( selection == "Criterion3, Cell index" ) {
		qsort(sorted_table_index,cs_error_list.size(),sizeof(int),sort_compare5 );
		sortTable();
	}
	else if ( selection == "Intelligent sort" ) {
		qsort(sorted_table_index,cs_error_list.size(),sizeof(int),sort_compare6 );
		sortTable();
	}
}

void CsLonlatErrorDialog::sortTable()
{
    for( unsigned int i=0; i<cs_error_list.size(); i++ ) { 
		int index = sorted_table_index[i];
        QString str;

		str.setNum( cs_error_list[index][0] );
		cs_error_table->setItem( i, 0, new Q3TableItem( cs_error_table, Q3TableItem::Never, str ) );

		char *hexstr = CVECTOR(2*PHSSectorClass::csid_byte_length);
		char *gwcsccsstr = CVECTOR(6);
		PHSSectorClass * sector = (PHSSectorClass *)(np->cell_list[cs_error_list[index][0]]->sector_list[0]);
		hex_to_hexstr(hexstr, ((PHSSectorClass *) sector)->csid_hex, ((PHSSectorClass *) sector)->csid_byte_length);
		sprintf(gwcsccsstr, "%.6d", ((PHSSectorClass *) sector)->gw_csc_cs);
		str = gwcsccsstr;
		str.append(" ");
		cs_error_table->setItem( i, 1, new Q3TableItem( cs_error_table, Q3TableItem::Never, str ) );
		str = hexstr;
		str.append(" ");
		cs_error_table->setItem( i, 2, new Q3TableItem( cs_error_table, Q3TableItem::Never, str ) );

        for( int j=1; j<4; j++ ) {
            if( cs_error_list[index][j] == 0 )
                str = tr( "" );
            else {
				if( 2==j )
                    str.setNum( (double)(cs_error_list[index][j])/100.0 );
                else str.setNum( cs_error_list[index][j] );
            }
            cs_error_table->setItem( i, j+2, new Q3TableItem( cs_error_table, Q3TableItem::Never, str ) );
        }
    }
	cs_error_table->adjustColumn( 0 );
    cs_error_table->adjustColumn( 1 );
    cs_error_table->adjustColumn( 2 );
    cs_error_table->adjustColumn( 3 );
	cs_error_table->adjustColumn( 4 );
	cs_error_table->adjustColumn( 5 );
//    cs_error_table->adjustSize();

}

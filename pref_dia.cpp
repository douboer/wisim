/******************************************************************************************/
/**** PROGRAM: pref_dia.cpp                                                     ****/
/**** Chengan 4/01/05                                                                  ****/
/******************************************************************************************/

#include <q3buttongroup.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <Q3VBoxLayout>

#include "cconst.h"
#include "wisim.h"
#include "wisim_gui.h"
#include "list.h"
#include "main_window.h"
#include "pref.h"
#include "pref_dia.h"
#include "strint.h"

#if HAS_ORACLE
#include <qsplitter.h>
#include <q3filedialog.h>

#include "pref_dia_db.h"
#include "database_fn.h"
#endif

extern MainWindowClass *main_window;

#if HAS_ORACLE
/*****************************************************************************************/
/**** FUNCTION: ConnectionNameDia::ConnectionNameDia                                  ****/
/*****************************************************************************************/
ConnectionNameDia::ConnectionNameDia(NetworkClass *np_param,QWidget* parent, const char* name)
  : QDialog( parent, name, true)
{
    np = np_param;
    setCaption( tr("Add Connection") );
    //resize( 350, 400 );
    
    repetition_flag = 0;
  
    Q3VBoxLayout *vLayout = new Q3VBoxLayout(this,6,11);
    Q3HBoxLayout *hLayout = new Q3HBoxLayout(0,6,11);
 
    //QGridLayout *database_diaLayout = new QGridLayout(this ,1,1,11,6,"db_diaLayout");

    QLabel *addLabel = new QLabel( "Connection: ", this );
    vLayout->addWidget(addLabel,1);

    addLineEdit = new QLineEdit( this );
    vLayout->addWidget(addLineEdit,1);
    addLineEdit->resize(  300, 100 ) ;
    addLineEdit->setText("WiSim");

    Q3ButtonGroup *dbButtonGroup = new Q3ButtonGroup(1,Qt::Horizontal,tr("Database Setting"),this );
    vLayout->addWidget(dbButtonGroup,1);

    dbButtonGroup->setColumnLayout(0, Qt::Vertical );
    dbButtonGroup->layout()->setSpacing( 6 );
    dbButtonGroup->layout()->setMargin( 11 );
    Q3GridLayout *dbsetting_diaLayout = new Q3GridLayout(dbButtonGroup->layout());
    dbsetting_diaLayout->setAlignment(Qt::AlignTop);

    QLabel *nameLabel = new QLabel( "Oracle Service Name",dbButtonGroup );
    dbsetting_diaLayout->addWidget(nameLabel ,0 ,0, Qt::AlignLeft );

    nameLineEdit = new QLineEdit( dbButtonGroup, "Name" );
    nameLineEdit->resize(  300, 100 );
    dbsetting_diaLayout->addWidget( nameLineEdit,0,1, Qt::AlignLeft );
    nameLineEdit->setText("");

    QLabel *portLabel = new QLabel("Port",dbButtonGroup );
    dbsetting_diaLayout->addWidget(portLabel, 1,0, Qt::AlignLeft);

    portLineEdit = new QLineEdit( dbButtonGroup, "Pt");
    portLineEdit->resize( 300, 100 );
    dbsetting_diaLayout->addWidget(portLineEdit, 1,1, Qt::AlignLeft);
    portLineEdit->setText("");

    QLabel *ip_addrLabel = new QLabel( "IP Address",dbButtonGroup );
    dbsetting_diaLayout->addWidget(ip_addrLabel ,2, 0, Qt::AlignLeft );

    ipLineEdit = new QLineEdit( dbButtonGroup, "IP Address" );
    ipLineEdit->resize( 300, 100  );
    dbsetting_diaLayout->addWidget( ipLineEdit, 2, 1, Qt::AlignLeft );
    ipLineEdit->setText("");

    QLabel *sidLabel = new QLabel( "SID",dbButtonGroup );
    dbsetting_diaLayout->addWidget(sidLabel , 3,0, Qt::AlignLeft );

    sidLineEdit = new QLineEdit( dbButtonGroup, "Sid" );
    sidLineEdit->resize(  300, 100  );
    dbsetting_diaLayout->addWidget( sidLineEdit, 3, 1, Qt::AlignLeft );
    sidLineEdit->setText("");

    QLabel *usr_nameLabel = new QLabel( "User Name",dbButtonGroup );
    dbsetting_diaLayout->addWidget(usr_nameLabel , 4,0 , Qt::AlignLeft );

    usr_nameLineEdit = new QLineEdit( dbButtonGroup, "Usrname" );
    usr_nameLineEdit->resize( 300, 100 );
    dbsetting_diaLayout->addWidget( usr_nameLineEdit, 4, 1, Qt::AlignLeft );
    usr_nameLineEdit->setText("");

    QLabel *passwdLabel = new QLabel( "Password",dbButtonGroup );
    dbsetting_diaLayout->addWidget(passwdLabel , 5, 0, Qt::AlignLeft );

    passwdLineEdit = new QLineEdit( dbButtonGroup, "Passwd" );
    passwdLineEdit->resize(  300, 100 );
    dbsetting_diaLayout->addWidget( passwdLineEdit, 5, 1, Qt::AlignLeft);
    passwdLineEdit->setText("");

    QPushButton *ok_btn = new QPushButton("&Ok", this);
    hLayout->addWidget(ok_btn,1);
    ok_btn->setMaximumWidth(80);

    QPushButton *cancel_btn = new QPushButton("&Cancel", this);
    hLayout->addWidget(cancel_btn,1);
    cancel_btn->setMaximumWidth(80);

    vLayout->addLayout(hLayout, Qt::AlignVCenter);

    show();
       
    connect( ok_btn,     SIGNAL( clicked() ),  this,  SLOT( ok_btn_clicked()   ) );
    connect( cancel_btn, SIGNAL( clicked() ),  this,  SLOT( cancel_btn_clicked()) );

    exec();

}
/**************************************************************************************/
/**** FUNCTION: ConnectionNameDia:: ok_btn_clicked()                             ****/
/**************************************************************************************/
void ConnectionNameDia::ok_btn_clicked() 
{
     int i;     
     PrefClass *pre = np->preferences;

      if ((strcmp(addLineEdit->text(),"") != 0)&&
         (strcmp(nameLineEdit->text(),"") != 0) &&
         (strcmp(ipLineEdit->text(),"") != 0) &&
         (strcmp(portLineEdit->text(),"") != 0 ) &&
         (strcmp(sidLineEdit->text(),"") != 0)  &&
         (strcmp(usr_nameLineEdit->text(),"") != 0) &&
         (strcmp(passwdLineEdit->text(),"") != 0) )
     {
     for (i=0; i<pre->num_db; i++)
     {
         if ( strcmp(addLineEdit->text(),pre->db_list[i]->connection)== 0 )
         {
              QMessageBox *prefMsgBox = new QMessageBox( tr("Error"),
                tr("Name already used!"), QMessageBox::Information, 1 | QMessageBox::Default,
                0, 0, 0, 0, TRUE);
              prefMsgBox->setButtonText( 1, tr("Cancel") );
              prefMsgBox->show();
              prefMsgBox->exec();
              delete prefMsgBox;
              return;
         }
     }
     pre->num_db++;
     DatabasePrefClass **db_ptr = (DatabasePrefClass**)malloc((pre->num_db)*sizeof(DatabasePrefClass*));
     for (i=0; i<pre->num_db; i++)
         db_ptr[i] = new DatabasePrefClass();
     for (i=0; i<pre->num_db-1; i++)
     {
         db_ptr[i]->connection = strdup(pre->db_list[i]->connection);
         db_ptr[i]->name       = strdup(pre->db_list[i]->name);
         db_ptr[i]->ipaddr     = strdup(pre->db_list[i]->ipaddr);
         db_ptr[i]->port       = strdup(pre->db_list[i]->port);
         db_ptr[i]->sid        = strdup(pre->db_list[i]->sid);
         db_ptr[i]->user       = strdup(pre->db_list[i]->user);
         db_ptr[i]->password   = strdup(pre->db_list[i]->password);
     }
      
     db_ptr[i]->connection = strdup(addLineEdit->text());
     db_ptr[i]->name       = strdup(nameLineEdit->text());
     db_ptr[i]->ipaddr     = strdup(ipLineEdit->text());
     db_ptr[i]->port       = strdup(portLineEdit->text());
     db_ptr[i]->sid        = strdup(sidLineEdit->text());
     db_ptr[i]->user       = strdup(usr_nameLineEdit->text());
     db_ptr[i]->password   = strdup(passwdLineEdit->text());

     pre->db_list = db_ptr;
     delete this;
     }
   else {
        QMessageBox *prefMsgBox = new QMessageBox( tr("Error"),
                 tr("Incomplete Settings!"), QMessageBox::Information, 1 | QMessageBox::Default,
                 0, 0, 0, 0, TRUE);
        prefMsgBox->setButtonText( 1, tr("Cancel") );
        prefMsgBox->show();
        prefMsgBox->exec();
        delete prefMsgBox;
   }
   return;
}
/**************************************************************************************/
/**** FUNCTION: ConnectionNameDia::cancel_btn_clicked()                            ****/
/**************************************************************************************/
void ConnectionNameDia::cancel_btn_clicked()
{
     repetition_flag = 1;
     delete this;
}
/**************************************************************************************/
/**** FUNCTION: ConnectionNameDia::~ConnectionNameDia                              ****/
/**************************************************************************************/
ConnectionNameDia::~ConnectionNameDia()
{
}
/**********************************************************************************/
/**** DbStDia::DbStDia()                                                       ****/
/**********************************************************************************/
DbStDia::DbStDia( class NetworkClass *np_param, QWidget* parent )
   : QWidget(parent, 0,true)
{
    np = np_param;
    char *str,*str1,*str2;

    Q3HBoxLayout *topLayout = new Q3HBoxLayout(this,6,11);  
    Q3VBoxLayout *leftLayout = new Q3VBoxLayout(0,6,11);    
 
    QPushButton *add_btn    = new QPushButton( tr("&Add"), this );
    add_btn->setMaximumWidth(80);

    delete_btn = new QPushButton( tr("&Delete"), this );
    delete_btn->setMaximumWidth(80);
    delete_btn->setDisabled(true);

    leftLayout->addWidget(add_btn,1);
    leftLayout->addWidget(delete_btn,1);

    connect_btn = new QPushButton( tr("&Connect"), this );
    connect_btn->setMaximumWidth(80);
    leftLayout->addWidget(connect_btn,1);
    connect_btn->setDisabled(true);

    QPushButton *export_btn = new QPushButton( tr("&Export"), this );
    export_btn->setMaximumWidth(80);
    leftLayout->addWidget(export_btn,1);

    topLayout->addLayout(leftLayout, Qt::AlignVCenter);

    QSplitter *split = new QSplitter( Qt::Horizontal, this, "splitter" );
    topLayout->addWidget( split,1 );

    dbListView = new Q3ListView(split,tr("Database Connection"));
    
    topLayout->addWidget(dbListView,1);
    dbListView->addColumn("       Connection      ");
    dbListView->setMinimumWidth(135);

    int num_db = np->preferences->num_db;
    dbnListView = (Q3ListViewItem**)malloc(num_db*sizeof(Q3ListViewItem*));
    for (int i =0; i<num_db; i++)
    {
        dbnListView[i] = new Q3ListViewItem(dbListView);
        dbnListView[i]->setText(0,tr(np->preferences->db_list[i]->connection));
     }

    DBClass TempDB;
    char *errmsg;
    errmsg = CVECTOR(255);
    if (num_db>0){
      if (np->preferences->selected_db != -1)
      {
        if (TempDB.connectDB(np->preferences->db_list[np->preferences->selected_db]->name,
                   np->preferences->db_list[np->preferences->selected_db]->user,
                   np->preferences->db_list[np->preferences->selected_db]->password,
                   errmsg))
        {
           lv_num = np->preferences->selected_db;
           str1 = strdup(np->preferences->db_list[lv_num]->connection);
           str2 = strtok(str1,CHDELIM);

           str = (char*)malloc(strlen(str2)+strlen(" (c)"));
           sprintf(str,"%s (c)",str2);
           dbnListView[lv_num]->setText(0,tr(str));
           lv_text = strdup(str2);
           TempDB.disconnectDB();
         }
         else    
         {
            np->preferences->selected_db = -1;
            lv_num = -1;
         }
      }
      else
      {
        lv_num= -1;
      }
    }
    dbButtonGroup = new Q3ButtonGroup(1,Qt::Horizontal,tr("Database Setting"),split );
    topLayout->addWidget(dbButtonGroup,1);

    dbButtonGroup->setColumnLayout(0, Qt::Vertical );
    dbButtonGroup->layout()->setSpacing( 6 );
    dbButtonGroup->layout()->setMargin( 11 );
    dbsetting_diaLayout = new Q3GridLayout(dbButtonGroup->layout());
    dbsetting_diaLayout->setAlignment(Qt::AlignLeft);

    QLabel *nameLabel = new QLabel( "Oracle Service Name:",dbButtonGroup );
    dbsetting_diaLayout->addWidget(nameLabel ,0 ,0, Qt::AlignLeft );

    nameLineEdit = new QLabel( dbButtonGroup, "Name" );
    dbsetting_diaLayout->addWidget( nameLineEdit,0,1, Qt::AlignLeft );
    nameLineEdit->setText("");

    QLabel *portLabel = new QLabel("Port:",dbButtonGroup );
    dbsetting_diaLayout->addWidget(portLabel, 1,0, Qt::AlignLeft);

    portLineEdit = new QLabel( dbButtonGroup, "Pt");
    dbsetting_diaLayout->addWidget(portLineEdit, 1,1, Qt::AlignLeft);
    portLineEdit->setText("");

    QLabel *ip_addrLabel = new QLabel( "IP Address:",dbButtonGroup );
    dbsetting_diaLayout->addWidget(ip_addrLabel ,2, 0, Qt::AlignLeft );

    ipLineEdit = new QLabel( dbButtonGroup, "IP Address" );
    dbsetting_diaLayout->addWidget( ipLineEdit, 2, 1, Qt::AlignLeft );
    ipLineEdit->setText("");

    QLabel *sidLabel = new QLabel( "SID:",dbButtonGroup );
    dbsetting_diaLayout->addWidget(sidLabel , 3,0, Qt::AlignLeft );

    sidLineEdit = new QLabel( dbButtonGroup, "Sid" );
    dbsetting_diaLayout->addWidget( sidLineEdit, 3, 1, Qt::AlignLeft );
    sidLineEdit->setText("");

    QLabel *usr_nameLabel = new QLabel( "User Name:",dbButtonGroup );
    dbsetting_diaLayout->addWidget(usr_nameLabel , 4,0 , Qt::AlignLeft );

    usr_nameLineEdit = new QLabel( dbButtonGroup, "Usrname" );
    dbsetting_diaLayout->addWidget( usr_nameLineEdit, 4, 1, Qt::AlignLeft );
    usr_nameLineEdit->setText("");

    QLabel *passwdLabel = new QLabel( "Password:",dbButtonGroup );
    dbsetting_diaLayout->addWidget(passwdLabel , 5, 0, Qt::AlignLeft );

    passwdLineEdit = new QLabel( dbButtonGroup, "Passwd" );
    dbsetting_diaLayout->addWidget( passwdLineEdit, 5, 1, Qt::AlignLeft);
    passwdLineEdit->setText("");
    
    //split->setOpaqueResize( true );
    //split->show();

    connect( add_btn,    SIGNAL( clicked() ), this,  SLOT( add_btn_clicked()    ));
    connect( delete_btn, SIGNAL( clicked() ), this,  SLOT( delete_btn_clicked() ));
    connect( dbListView, SIGNAL( clicked(Q3ListViewItem *) ),
                 this,  SLOT(listview_selected(Q3ListViewItem *)));
    connect( connect_btn,SIGNAL( clicked() ), this,  SLOT( connect_btn_clicked()));
    connect( export_btn, SIGNAL( clicked() ), this,  SLOT( export_btn_clicked() ));

}
/******************************************************************************************/
/**** FUNCTION: DbStDia::~DbStDia                                                      ****/
/******************************************************************************************/
DbStDia::~DbStDia()
{
}
/******************************************************************************************/
/**** FUNCTION: DbStDia::add_btn_clicked                                              ****/
/******************************************************************************************/
void DbStDia::add_btn_clicked() 
{   
      int num_db;
      int i;
       char *str;

       if (lv_num != -1)
       {
           str = strdup(np->preferences->db_list[lv_num]->connection);
           np->preferences->db_list[lv_num]->connection = strdup(lv_text);
       }
       conn_name_dia = new ConnectionNameDia(np,0,tr("Add Connection"));
       if (lv_num != -1)
           np->preferences->db_list[lv_num]->connection = strdup(str);
       if (conn_name_dia->repetition_flag == 0)
       {
       num_db = np->preferences->num_db;
       Q3ListViewItem **view_ptr = (Q3ListViewItem**)malloc(num_db*sizeof(Q3ListViewItem*));
       for (i=0; i<num_db; i++)
          view_ptr[i] = new Q3ListViewItem(dbListView);
       for (i=0;i<num_db-1;i++) {
          view_ptr[i]->setText(0, dbnListView[i]->text(0));
          delete(dbnListView[i]);
       }
       view_ptr[i] = new Q3ListViewItem(dbListView);
       view_ptr[i]->setText(0,tr(np->preferences->db_list[i]->connection));
       dbnListView = view_ptr;
       connect_btn->setDisabled(true);
       delete_btn->setDisabled(true);
       }
}
/******************************************************************************************/
/**** FUNCTION: DbStDia::delete_btn_clicked                                            ****/
/******************************************************************************************/
void DbStDia::delete_btn_clicked()
{
    int i;
    int listview_num = -1;
    int num_db = np->preferences->num_db;
    PrefClass *pref = np->preferences;
    if (num_db > 0) 
    {
       for (i=0; i<num_db; i++)
          if ( dbnListView[i]->isSelected()== TRUE )
          {
             listview_num = i;
             break;
           }

        if (listview_num != -1) {
           delete(pref->db_list[listview_num]);
           for (i=listview_num; i<num_db-1; i++)
           {  
               pref->db_list[i] = pref->db_list[i+1];
           }
           if (lv_num == listview_num)
           {
               lv_num = -1;
               //pref->lv_dbnum = -1;
               pref->selected_db = -1;
               connect_btn->setText("&Connect"); 
               connect_btn->setDisabled(true);
           }
           delete(dbnListView[listview_num]);
           for (i=listview_num; i<num_db-1; i++)
           {
               dbnListView[i] = dbnListView[i+1];
           }
           np->preferences->num_db--;
        }
    }
}

/******************************************************************************************/
/**** FUNCTION: DbStDia::connect_btn_clicked()                                         ****/
/******************************************************************************************/
void DbStDia::connect_btn_clicked()
{
    int i;
    int listview_num = -1;
    int num_db = np->preferences->num_db;
    PrefClass *pref = np->preferences;
    //int lvitem_num=-1;
    char *str;
    char *errmsg = (char*)malloc(255*sizeof(char));
    DBClass TempDB;
    if (num_db > 0)
    {
        for (i=0; i<num_db; i++)
          if ( dbnListView[i]->isSelected()== TRUE )
          {
             listview_num = i;
             break;
           }
    }

    if ( listview_num != -1 )
    {
        if (lv_num != -1)
             dbnListView[lv_num]->setText(0,lv_text);
        if (lv_num != listview_num)
        {
             /*lv_num = listview_num;
             lv_text = strdup(dbnListView[listview_num]->text(0));
             str = CVECTOR(strlen(dbnListView[listview_num]->text(0))+strlen(" (c)")); 
             sprintf(str,"%s (c)",lv_text);
             dbnListView[listview_num]->setText(0,str);
             pref->db_list[listview_num]->connection = strdup(str);
             pref->selected_db = listview_num;*/
             if (!TempDB.connectDB(pref->db_list[listview_num]->name,
                   pref->db_list[listview_num]->user,
                   pref->db_list[listview_num]->password,
                   errmsg))
             {
                 QMessageBox *prefMsgBox = new QMessageBox( tr("Error"),
                     tr("Cannot connect!"), QMessageBox::Information, 1 | QMessageBox::Default,
                     0, 0, 0, 0, TRUE);
                 prefMsgBox->setButtonText( 1, tr("Cancel") );
                 prefMsgBox->show();
                 prefMsgBox->exec();
                 delete prefMsgBox;
                 free(errmsg);
                 if (lv_num != -1)
                 {
                    lv_text = strdup(dbnListView[lv_num]->text(0));
                    str = CVECTOR(strlen(dbnListView[lv_num]->text(0))+strlen(" (c)")); 
                    sprintf(str,"%s (c)",lv_text);
                    dbnListView[lv_num]->setText(0,str);
                 }
                 return;
             }
             else TempDB.disconnectDB();

             lv_num = listview_num;
             lv_text = strdup(dbnListView[listview_num]->text(0));
             str = CVECTOR(strlen(dbnListView[listview_num]->text(0))+strlen(" (c)"));
             sprintf(str,"%s (c)",lv_text);
             dbnListView[listview_num]->setText(0,str);
             pref->db_list[listview_num]->connection = strdup(str);
             pref->selected_db = listview_num;

            /*
             pref->conn_flag = 1;
             pref->name = strdup(pref->db_list[lvitem_num]->name);
             pref->user = strdup(pref->db_list[lvitem_num]->user);
             pref->password = strdup(pref->db_list[lvitem_num]->password);*/
             connect_btn->setText(tr("&Disconnect"));
        }
        else {
             lv_num = -1;
             dbnListView[listview_num]->setText(0,lv_text);
             pref->db_list[listview_num]->connection = strdup(lv_text);
             pref->selected_db = -1;
             /*
             pref->conn_flag = 0;
             pref->name = NULL;
             pref->user = NULL;
             pref->password = NULL;*/
             listview_num = -1;
             connect_btn->setText(tr("&Connect"));
        }
        //pref->lv_dbnum = listview_num;
        //pref->lv_connection = strdup(lv_text);
     }

}
/******************************************************************************************/
/**** FUNCTION: DbStDia::export_btn_clicked()                                          ****/
/******************************************************************************************/
void DbStDia::export_btn_clicked()
{
    char * oracle_home = (char*)NULL;
    char * filepath;
    get_environment_variable("ORACLE_HOME",oracle_home);
    if (oracle_home)
    {
       filepath = CVECTOR(strlen(oracle_home)+strlen("\\network\\admin"));
       sprintf(filepath,"%s\\network\\admin",oracle_home);
    } else 
    {
       QMessageBox *prefMsgBox = new QMessageBox( tr("Error"),
           "Oracle not installed", QMessageBox::Information,
           1 | QMessageBox::Default, 0, 0, 0, 0, TRUE);
       prefMsgBox->setButtonText( 1, tr("Cancel") );
       prefMsgBox->show();
       prefMsgBox->exec();       
       return;
    }
    QString qfile = Q3FileDialog::getOpenFileName(
                          filepath,
                          "*.ora",
                          0, 
                          "select the export file",
                          "select file tnsnames.ora" );
    QFile f(qfile);
    if (!f.open(QIODevice::WriteOnly))  return;
    Q3TextStream ts( &f );
    char * chptr,*msg;
    msg = chptr = (char*)malloc(2000*sizeof(char));
    for (int i=0; i<np->preferences->num_db; i++)
    {
         chptr += sprintf(chptr, "%s =\n",np->preferences->db_list[i]->name );
         chptr += sprintf(chptr, "  (DESCRIPTION =\n");
         chptr += sprintf(chptr, "    (ADDRESS_LIST =\n");
         chptr += sprintf(chptr, "      (ADDRESS = (PROTOCOL = TCP)(HOST = ");
         chptr += sprintf(chptr, "%s",np->preferences->db_list[i]->ipaddr );
         chptr += sprintf(chptr, ")(PORT = %s))\n",np->preferences->db_list[i]->port );
         chptr += sprintf(chptr, "    )\n   (CONNECT_DATA =\n");
         chptr += sprintf(chptr, "      (SID =%s)\n",np->preferences->db_list[i]->sid);
         chptr += sprintf(chptr, "      (SERVER = DEDICATED)\n   )\n  )\n\n");
    }
    ts.operator<<(msg); 
    if (oracle_home)
    {
         free(oracle_home);
         free(filepath); 
    }
}
/******************************************************************************************/
/**** FUNCTION: DbStDia::listview_selected()                                           ****/
/******************************************************************************************/
void DbStDia::listview_selected(Q3ListViewItem *lview_item)
{
    int i;
    int flag =0;
    int dbn_num;
    PrefClass *pref = np->preferences;
    for (i=0; i<np->preferences->num_db; i++)
       if (dbnListView[i]==lview_item)
       {
          flag = 1;
          dbn_num = i;
          break;
       }
    if (flag) 
    {
        DatabasePrefClass *dbc = pref->db_list[dbn_num];
        nameLineEdit->setText(tr(dbc->name));
        ipLineEdit->setText(tr(dbc->ipaddr));
        portLineEdit->setText(tr(dbc->port));
        sidLineEdit->setText(tr(dbc->sid));
        usr_nameLineEdit->setText(tr(dbc->user));
        passwdLineEdit->setText(tr(dbc->password));
        connect_btn->setEnabled(true);
        delete_btn->setEnabled(true);
        if (dbn_num != lv_num)
            connect_btn->setText(tr("&Connect"));
        else 
            connect_btn->setText(tr("&Disconnect"));
    }
    else {
        nameLineEdit->setText("");
        ipLineEdit->setText("");
        portLineEdit->setText("");
        sidLineEdit->setText("");
        usr_nameLineEdit->setText("");
        passwdLineEdit->setText("");

        connect_btn->setDisabled(true);
        delete_btn->setDisabled(true);
   }
}
/******************************************************************************************/
#endif

/******************************************************************************************/
/**** FUNCTION: GenericSettingsDia::GenericSettingsDia                                 ****/
/******************************************************************************************/
GenericSettingsDia::GenericSettingsDia(class NetworkClass *np_param, QWidget* parent)
      : QWidget(parent)
{
    np = np_param;  

    pwr_unit_list = new ListClass<int>(4);
    pwr_unit_list->ins_elem(CConst::PWRdBm);
    pwr_unit_list->ins_elem(CConst::PWRdBW);
    pwr_unit_list->ins_elem(CConst::PWRdBuV_107);
    pwr_unit_list->ins_elem(CConst::PWRdBuV_113);

    int pwr_idx, cell_name_idx;
 
    Q3GridLayout *generic_diaLayout = new Q3GridLayout(this, 1, 1, 11, 6, "generic_diaLayout");

    /**************************************************************************************/
    /**** Language Selection                                                           ****/
    /**************************************************************************************/
    Q3ButtonGroup *lang_btngroup = new Q3ButtonGroup(1, Qt::Horizontal, tr("Language"), this);
    generic_diaLayout->addWidget(lang_btngroup, 0, 0);

    en_btn = new QRadioButton(tr("English"), lang_btngroup);
    lang_btngroup->insert(en_btn, 0);

    zh_btn = new QRadioButton(tr("Chinese"), lang_btngroup);
    lang_btngroup->insert(zh_btn, 1);

    if (np->preferences->language == CConst::en) {
        zh_btn->setChecked(false);
        en_btn->setChecked(true);
    } else {
        en_btn->setChecked(false);
        zh_btn->setChecked(true);
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Cell Size Selection                                                          ****/
    /**************************************************************************************/
    QLabel *cell_size_label = new QLabel(tr("Cell Size"), this);
    cell_size_spinbox = new QSpinBox(0, GCellClass::num_sizes-1, 1, this);

    generic_diaLayout->addWidget(cell_size_label,   1, 0);
    generic_diaLayout->addWidget(cell_size_spinbox, 1, 1);

    cell_size_spinbox->setValue(np->preferences->cell_size_idx);
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Report Cell Name Selection                                                   ****/
    /**************************************************************************************/
    QLabel *cell_name_label = new QLabel(tr("Cell Name Style in Reports"), this);
    cell_name_combobox = new QComboBox( false, this, "cell_name_combobox" );

    generic_diaLayout->addWidget(cell_name_label,    2, 0);
    generic_diaLayout->addWidget(cell_name_combobox, 2, 1);

    for (cell_name_idx=0; cell_name_idx<=np->report_cell_name_opt_list->getSize()-1; cell_name_idx++) {
        cell_name_combobox->insertItem((*(np->report_cell_name_opt_list))[cell_name_idx].getStr(), cell_name_idx);
        if (np->preferences->report_cell_name_pref == (*(np->report_cell_name_opt_list))[cell_name_idx].getInt()) {
            cell_name_combobox->setCurrentItem(cell_name_idx);
        }
    }
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Power Unit Selection                                                         ****/
    /**************************************************************************************/
    QLabel *pwr_unit_label = new QLabel(tr("Power Unit"), this);
    pwr_unit_combobox = new QComboBox( false, this, "pwr_unit_combobox" );

    generic_diaLayout->addWidget(pwr_unit_label,    3, 0);
    generic_diaLayout->addWidget(pwr_unit_combobox, 3, 1);

    char str[100];
    for (pwr_idx=0; pwr_idx<=pwr_unit_list->getSize()-1; pwr_idx++) {
        pwr_unit_to_name((*pwr_unit_list)[pwr_idx], str);
        pwr_unit_combobox->insertItem(str, pwr_idx);
        if (np->preferences->pwr_unit == (*pwr_unit_list)[pwr_idx]) {
            pwr_unit_combobox->setCurrentItem(pwr_idx);
        }
    }
    /**************************************************************************************/
}
/******************************************************************************************/
/****FUNCITON : GenericSettingsDia::~GenericSettingsDia                                ****/
/******************************************************************************************/
GenericSettingsDia::~GenericSettingsDia() {
    delete pwr_unit_list;
}
/******************************************************************************************/
/**** FUNCTION: PrefDia::PrefDia                                                       ****/
/******************************************************************************************/
PrefDia::PrefDia(FigureEditor *editor_param, QWidget* parent)
    : QDialog(parent, 0, true)
{
    setName("Preferences");
    setCaption(tr("Preferences"));
    editor = editor_param;
    np = editor->get_np();

    Q3GridLayout *pref_diaLayout = new Q3GridLayout( this, 1, 1, 11, 6, "pref_diaLayout");
    QTabWidget     *pref_tabwidget = new QTabWidget(this, tr("Preference"));
    pref_diaLayout->addMultiCellWidget( pref_tabwidget, 0,6,0,3);
    
    generic_widget    = new GenericSettingsDia(np,pref_tabwidget);
    pref_tabwidget->addTab(generic_widget,tr("Generic"));
    pref_tabwidget->showPage(generic_widget);

#if HAS_ORACLE
    database_widget= new DbStDia(np,pref_tabwidget);
    pref_tabwidget->addTab(database_widget,tr("Database"));
    pref_tabwidget->showPage(database_widget);
#endif

    QPushButton *save_btn = new QPushButton( tr("&Save"), this);
    save_btn->setMaximumWidth(100);

    QPushButton *cancel_btn = new QPushButton( tr("&Cancel"), this);
    cancel_btn->setMaximumWidth(100);

    pref_diaLayout->addWidget(save_btn,7,2);
    pref_diaLayout->addWidget(cancel_btn,7,3);
    
    connect( save_btn,   SIGNAL( clicked() ), this,  SLOT( save_btn_clicked()   ));
    connect( cancel_btn, SIGNAL( clicked() ), this,  SLOT( cancel_btn_clicked() ));
    connect( this, SIGNAL( pwr_unit_changed() ), main_window, SLOT( update_pwr_unit() ) );
 
    exec();
}
/******************************************************************************************/
/**** FUNCTION: PrefDia::~PrefDia                                                      ****/
/******************************************************************************************/
PrefDia::~PrefDia()
{
}
/******************************************************************************************/
/**** FUNCTION: PrefDia::save_btn_clicked                                              ****/
/******************************************************************************************/
void PrefDia::save_btn_clicked()
{
    int i;
    QString s;
    FILE *fp;
    PrefClass *pref= np->preferences;
    int lang_change;

    if ( !(fp = fopen(np->preferences->filename, "wb")) ) {
        sprintf(np->msg, "ERROR writing to file %s\n", np->preferences->filename);
        PRMSG(stdout, np->msg); np->error_state = 1;
    }

    if (    ( (np->preferences->language == CConst::en) && (!generic_widget->en_btn->isOn()) )
         || ( (np->preferences->language == CConst::zh) && (!generic_widget->zh_btn->isOn()) ) ) {
        lang_change = 1;
    } else {
        lang_change = 0;
    }

    fprintf(fp, "LANGUAGE: %s\n", ( (generic_widget->en_btn->isOn()) ? "en" : "zh" ) );

    if (generic_widget->cell_size_spinbox->value() != pref->cell_size_idx) {
        pref->cell_size_idx = generic_widget->cell_size_spinbox->value();
        GCellClass::setCellSize(pref->cell_size_idx);
        GCellClass::set_bitmaps(np->preferences->cell_size_idx);
        GCellClass::setCellPixmapList();
        editor->setVisibility(GConst::cellRTTI);
    }
    fprintf(fp, "CELL_SIZE_IDX: %d\n", pref->cell_size_idx);

    pref->report_cell_name_pref =
        (*(np->report_cell_name_opt_list))[generic_widget->cell_name_combobox->currentItem()].getInt();
    fprintf(fp, "REPORT_CELL_NAME_PREF: %d\n", pref->report_cell_name_pref);

    int pwr_idx = generic_widget->pwr_unit_combobox->currentItem();
    int pwr_unit = (*generic_widget->pwr_unit_list)[pwr_idx];
    if (pref->pwr_unit != pwr_unit) {
        pref->pwr_unit = pwr_unit;
        pref->pwr_offset = get_pwr_unit_offset(pwr_unit);

        char *str = CVECTOR(100);
        pwr_unit_to_name(pwr_unit, str);
        pref->pwr_str_long = strdup(str);

        for (i=0; (str[i]) && (str[i] != '_'); i++) {}
        str[i] = (char) NULL;

        pref->pwr_str_short = strdup(str);

        free(str);

        emit pwr_unit_changed();
    }
    fprintf(fp, "PWR_UNIT: %s\n", pref->pwr_str_long);

    if (pref->num_db>0) {
        fprintf(fp, "NUM_DB: %d\n", pref->num_db );
        fprintf(fp, "SELECTED_DB: %d\n", pref->selected_db );
        for (int i=0; i<pref->num_db; i++) {
            fprintf(fp, "DATABASE: \n" );
            fprintf(fp, "CONNECTION: %s\n", pref->db_list[i]->connection );
            fprintf(fp, "NAME: %s\n",       pref->db_list[i]->name );
            fprintf(fp, "PORT: %s\n",       pref->db_list[i]->port );  
            fprintf(fp, "IPADDRESS: %s\n",  pref->db_list[i]->ipaddr );
            fprintf(fp, "SID: %s\n"       , pref->db_list[i]->sid ); 
            fprintf(fp, "USERNAME: %s\n",   pref->db_list[i]->user );       
            fprintf(fp, "PASSWORD: %s\n",   pref->db_list[i]->password ); 
        }
    }

    fclose(fp);

    s = "<h3>WISIM</h3>";
    s += "<ul>";
    s +=    tr("Preferences have been saved.") + "\n";
    if (lang_change) {
        s +=    tr("Language setting has been changed.") + "\n";
        s +=    tr("Please exit and restart WISIM") + "\n";
        s +=    tr("for language change to take effect.") + "\n";
    }
    s += "</ul>";

    QMessageBox *prefMsgBox = new QMessageBox( tr("Preferences Saved"),
        s, QMessageBox::Information, 1 | QMessageBox::Default, 0, 0, 0, 0, TRUE);

    prefMsgBox->setButtonText( 1, tr("Dismiss") );
    prefMsgBox->show();
    prefMsgBox->exec();

    delete prefMsgBox;
    delete this;
}
/******************************************************************************************/
/**** FUNCTION: PrefDia::cancel_btn_clicked                                            ****/
/******************************************************************************************/
void PrefDia::cancel_btn_clicked()
{
    delete this;
}
/******************************************************************************************/


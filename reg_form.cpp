/****************************************************************************
** The wizard of registration
**
** Created: Wed Jan 12 10:36:39 2005
**  
** Auther: Wei Ben
** Modified Michael Mandell
****************************************************************************/

#include "reg_form.h"

#include <qvariant.h>
#include <qwidget.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qlineedit.h>
#include <q3textedit.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <stdlib.h>

#include "license.h"
#include "wisim.h"

#ifndef __linux__
int sendRegInfo(char *reg_file_rel, char *reg_file_full, char *subject, char *value);
#endif

/*
 *  Constructs a regForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The wizard will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
regForm::regForm( unsigned char *reg_info, int ris, QWidget* parent, \
                  const char* name, bool modal, Qt::WFlags fl ) \
    : Q3Wizard( parent, name, modal, fl ),reg_info(reg_info), ris(ris)
{
    if ( !name )
    setName( "regForm" );

    WizardPage = new QWidget( this, "WizardPage" );
    WizardPageLayout = new Q3GridLayout( WizardPage, 1, 1, 11, 6, "WizardPageLayout"); 

    userTypeGrp = new Q3ButtonGroup( WizardPage, "userTypeGrp" );

    radioButton1 = new QRadioButton( userTypeGrp, "radioButton1" );
    radioButton1->setGeometry( QRect( 20, 60, 200, 40 ) );
    radioButton1->setChecked( true );

    radioButton2 = new QRadioButton( userTypeGrp, "radioButton2" );
    radioButton2->setGeometry( QRect( 20, 130, 180, 40 ) );
    radioButton2->setEnabled( true );

    WizardPageLayout->addWidget( userTypeGrp, 1, 1 );
    spacer2 = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
    WizardPageLayout->addItem( spacer2, 0, 1 );
    spacer3 = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
    WizardPageLayout->addItem( spacer3, 2, 1 );
    spacer4 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    WizardPageLayout->addItem( spacer4, 1, 0 );
    spacer5 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    WizardPageLayout->addItem( spacer5, 1, 2 );
    addPage( WizardPage, QString("") );
    languageChange();
    resize( QSize(497, 375).expandedTo(minimumSizeHint()) );
    init();

    // signals and slots connections
    connect( userTypeGrp, SIGNAL( clicked(int) ), this, SLOT( getType(int) ) );
    connect( external_form, SIGNAL( finishCheck(bool)), this, SLOT( enableNext(bool)) );
    connect( internal_form, SIGNAL( done(bool)), this, SLOT( enableNext(bool)) );
    connect( finishButton(), SIGNAL( clicked() ), this, SLOT( sendReg() ) );
    connect( nextButton(), SIGNAL( clicked() ), this, SLOT( createReg() ) );
    connect( cancelButton(), SIGNAL( clicked() ), this, SLOT( cancel() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
regForm::~regForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void regForm::languageChange()
{
    setCaption( tr( "Welcome to register the WiSim" ) );
    userTypeGrp->setTitle( tr( "User Type" ) );
    radioButton2->setText( tr( "Internal User" ) );
    radioButton1->setText( tr( "External User" ) );
    setTitle( WizardPage, tr( "Register: User Type" ) );

    backButton()->setText(tr("&Back") + " <");
    nextButton()->setText(tr("&Next") + " >");
    finishButton()->setText( tr ("Send Register File" ) );
    cancelButton()->setText(tr("&Cancel"));
    helpButton()->setText(tr("&Help"));

}

/* 
 *  Adds the other pages to the wizard
 *
 */

void regForm::init()
{
    external_form = new ExternalForm();
    addPage( external_form, tr("Register: External Form") );
     
    internal_form = new InternalForm();
    addPage(internal_form, tr("Register: Internal Form") );
     
    last_form = new lastForm();
    addPage(last_form, tr("Register: Send Register File"));

    setHelpEnabled(external_form, false);
    setHelpEnabled(internal_form, false);
    setHelpEnabled(last_form, false);
    setHelpEnabled(WizardPage, false);   

    setNextEnabled(external_form, false);
    setNextEnabled(internal_form, false);
    

    setFinishEnabled(last_form, true);
    setAppropriate(internal_form, false);
    m_type = 0;

}

/* 
 * slot for the user type
 * 
 */

void regForm::getType( int i) 
{

  //  printf( " the user click %d\n", i);
  m_type = i;
    if ( i == 0 ) {
       setAppropriate(external_form, true);
       setAppropriate(internal_form, false);
     }
    if ( i == 1) {
       setAppropriate(external_form, false);
       setAppropriate(internal_form, true);
     }
}

/*
 *
 * enable the "next" btn of the wizard
 *
 */

void regForm::enableNext(bool yes) 
{

  if ( yes ) {
    if ( m_type == 0) {
      setNextEnabled(external_form, true);
      content.name = external_form->lnEditName->text();
      content.addr = external_form->lnEditAddr->text();
      content.phone = external_form->lnEditPhone->text();
      content.phoneOpt = external_form->lnEditPhoneOpt->text();
      content.email = external_form->lnEditEmail->text();
      content.post_code = external_form->lnEditAddr_2->text();
      
    }
    else {
      setNextEnabled(internal_form, true);
      content.name = internal_form->lnEditName->text();
      content.addr = internal_form->lnEditAddr->text();
      content.phone = internal_form->lnEditPhone->text();
      content.phoneOpt = internal_form->lnEditPhoneOpt->text();
      content.email = internal_form->lnEditEmail->text();
    }
  }
  else {
    if ( m_type == 0) {
      setNextEnabled(external_form, false);
    }
    else {
      setNextEnabled(internal_form, false);
    }
  }
}
/*
 *
 * when a user input all the info. about registration. 
 * create a registration file. This slot is responsible 
 * for the nextButton() selectively
 * 
 */
void regForm::createReg() 
{

  QString disp_str;
  QString s;
  QMessageBox* regMsgBox;
  int flag;

  if ( Q3Wizard::indexOf(Q3Wizard::currentPage()) == 3) { 
    //printf("begin to create a reg. file\n");
    
    char *name = strdup((const char *) content.name.local8Bit());
    char *company= strdup((const char *) content.addr.local8Bit());
    char *email= strdup((const char *) content.email.local8Bit());
 
    flag = gen_reg_file(reg_info, ris, name, company, email, reg_file_rel, reg_file_full);
 
    if ( !flag ) {
      printf("ERROR: Unable to write to file %s\n", reg_file_full);
      
      s = "<h3>WISIM</h3>";
      s += "<ul>";
      s +=    tr("Registration Unsuccessful") + "\n";
      s +=    tr("Unable to write to file") + " <b>" + QString::fromLocal8Bit(reg_file_full) + "</b>\n";
      s += "</ul>";
      
      regMsgBox = new QMessageBox( tr("Registration Unsuccessful"), \
                                   s, QMessageBox::Critical, 1 | QMessageBox::Default, 0, 0, 0, 0, TRUE);
      
      regMsgBox->setButtonText( 1, tr("Dismiss") );
      regMsgBox->show();
      
      regMsgBox->exec();
      
      reject();
    } 
    else {
      
      disp_str = "<h3>WISIM</h3>";
      disp_str += "<ul>";
      disp_str +=    tr("Registration file successfully created") + ".  ";
#ifdef __linux__
      disp_str +=    tr("The file") + " <b>" + QString::fromLocal8Bit(content.name) + ".creg</b> " + tr("has been generated") + ".  ";
#else
      disp_str +=    tr("The file") + " <b>" + QString::fromLocal8Bit(content.name) + ".creg</b> " + tr("has been generated on the current user's Destop") + ".  ";
#endif
      disp_str +=    tr("Please send this file to the following EMAIL address:");
      disp_str +=    "<a href=\"mailto:douboer@gmail.com\"></a> douboer@gmail.com";
      disp_str += "</ul>";  
      
      last_form->textEdit1->setText(disp_str);
      last_form->textEdit1->setReadOnly(TRUE);
      
    }
  }
}

/* 
 * the slot for finishButton(), call window's application to 
 * send the email, attached with registration file, out
 *
 */

void regForm::sendReg() 
{
  // construct the content msg
  QString subject = tr("WISIM License Application");
  QString s;

  s  = tr("WiSim License Administrator") + ",\n\n";
  s += tr("I am requesting a WiSim software license, attached is my WiSim registration data file.") + "\n\n";

  s += "--------------------------------------------------------------------\n";
  s += tr("NOTE: Please verify that the file") + " \"" + QString(reg_file_rel) + "\" " + tr("has been successfully attached to this message") + ".\n";
  s += "--------------------------------------------------------------------\n";

  s += tr("User Info: ") + "\n";
  s += "--------------------------------------------------------------------\n";
  s += tr("Name:  ") ;
  s += content.name;
  s += "\n";

  if (m_type == 0 ) {// external user  
    s += tr("Mail Address: ") ;
    s += content.addr + "\n";
    s += tr("Post Code:   ");
    s += content.post_code + "\n";

    s += tr("Primary Phone No.:  ");
    s += content.phone + "\n";
    s += tr("Option Phone No.:   ");
    s += content.phoneOpt + "\n";
  } 
  else {
    s += tr("Department:  ");
    s += content.addr + "\n";
    s += tr("Ext. No.:    ");
    s += content.phone + "\n";
    s += tr("Option Cell No.:   ");
    s += content.phoneOpt + "\n";
  }
  
  s += tr("Email Address:  ");
  s += content.email + "\n";

  char *subject_str = strdup((const char *) subject.local8Bit());
  char *msg_str = strdup((const char *) s.local8Bit());

  /*
  printf("%s\n", s.latin1());
  printf("%s\n", subject.latin1()); 
  */
#ifndef __linux__
//    ShellExecute(NULL, L"open", L"mailto:douboer@gmail.com", NULL, NULL, SW_SHOWNORMAL);
   int result = sendRegInfo(reg_file_rel, reg_file_full, subject_str, msg_str);

   // porting to QT4
   /*
   if( result > 1000  ) {
       //printf( "MAPI error\nYou need to find the Regestration Information File on the desktop and attach it manually\n" );
       ShellExecute(NULL,NULL,L"mailto:douboer@gmail.com",NULL,NULL,SW_SHOW);
   } else if( result != 0 ) {
        // WARNING: Registration Info not sent.
        //AfxMessageBox(AFX_IDP_FAILED_MAPI_SEND);
   }
   */
#endif
   //printf("%s %s\n", reg_file_rel, reg_file_full);
   free(subject_str);
   free(msg_str);
}


/*
 * cancel button's response
 *
 */
void regForm::cancel() 
{
 
  delete this;

}

/*
 * use the MAPI lib. to send the email out.
 */

#ifndef __linux__
int sendRegInfo(char *reg_file_rel, char *reg_file_full, char *subject, char *value)
{
    // porting to QT4
    /*
    HMODULE hMod = LoadLibrary(L"MAPI32.DLL");

    if (hMod == NULL)
    {
        //AfxMessageBox(AFX_IDP_FAILED_MAPI_LOAD);
        return 1001;
    }

    ULONG (PASCAL *lpfnSendMail)(ULONG, ULONG, MapiMessage*, FLAGS, ULONG);
    (FARPROC&)lpfnSendMail = GetProcAddress(hMod, "MAPISendMail");

    if (lpfnSendMail == NULL) {
        //AfxMessageBox(AFX_IDP_INVALID_MAPI_DLL);
        return 1002;
    }

    MapiFileDesc attachment = { 0,            // ulReserved, must be 0
                                0,            // no flags; this is a data file
                                (ULONG)-1,    // position not specified
                       reg_file_full,         // pathname
                       reg_file_rel,          // original filename
                        NULL};                // MapiFileTagExt unused

    // MapiRecipDesc recip={ 0, MAPI_TO, "SW Product", "douboer@gmail.com", 0, NULL };
    MapiRecipDesc recip={ 0, MAPI_TO, "douboer@gmail.com", 0, NULL };

    MapiMessage message;
    memset(&message, 0, sizeof(message));
    message.nFileCount = 1;
    message.lpFiles = &attachment;
    message.nRecipCount = 1;
    message.lpRecips = &recip;
    message.lpszSubject = subject;
    message.lpszNoteText= value;

    //CWnd* pParentWnd = CWnd::GetSafeOwner(NULL, NULL);

    int nError = lpfnSendMail(0, 0, &message, MAPI_LOGON_UI|MAPI_DIALOG , 0);

    if (nError != SUCCESS_SUCCESS && nError != MAPI_USER_ABORT
        && nError != MAPI_E_LOGIN_FAILURE)
    {
        //AfxMessageBox(AFX_IDP_FAILED_MAPI_SEND);
        return nError;
    }

    FreeLibrary(hMod);
    */
    return 0;
}
#endif



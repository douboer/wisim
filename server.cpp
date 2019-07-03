/**************************************************/
/* One Dialog and has socket server functionality */
/* Auther: Wei Ben                                */
/* Time:   Sept. 30, 2004                         */
/**************************************************/

#include "server.h"
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3VBoxLayout>
#include <QLabel>

ClientSocket::ClientSocket(int sock, QObject *parent, const char *name)
  : Q3Socket(parent, name) 
{
  line = 0;
  connect( this, SIGNAL(readyRead()),
           SLOT(readClient()) );
  connect( this, SIGNAL(connectionClosed()),
           SLOT(deleteLater()) );
  setSocket( sock );
}


void ClientSocket::readClient() 
{
  Q3TextStream ts( this );
  bool ok;
  
  int dec; 
  while ( canReadLine() ) {
    QString str = ts.readLine();
    //qDebug(str);

    dec = str.toInt( &ok, 10 ); 
    if ( ! ok) {
      emit logText( QString("'%1'\n").arg(str) );
    }
    else 
      emit progressValue( dec );
    
    line++;
  }
}

/*--------------------------------*/

SimpleServer::SimpleServer(QObject* parent) : Q3ServerSocket( 4242, 1, parent )
{
  if ( !ok() ) {
    qWarning("Failed to bind to port 4242");
    exit(1);
  }
}


void SimpleServer::newConnection(int socket) 
{
  ClientSocket *s = new ClientSocket( socket, this );
  emit newConnect( s );
}

/*---------------------------------*/
ServerInfo::ServerInfo(QWidget* parent, const char* name,  bool modal) 
  :QDialog(parent, name, modal) 

{
  SimpleServer *server = new SimpleServer( this );
  

  setCaption(tr("Convert Road Test Data"));
  resize( 400, 380);
  Q3VBoxLayout *oLayout = new Q3VBoxLayout(this, 11, 6, "whole_Form");
  QString itext = tr("Get feedback from road convert process.") + "\n";
  QLabel *lb = new QLabel( itext, this );
  lb->setAlignment( Qt::AlignHCenter );

  infoText = new Q3TextEdit( this );
  infoText->setReadOnly(true);
  infoText->setWordWrap(Q3TextEdit::NoWrap);

  progressBar = new Q3ProgressBar( 100, this );
  QPushButton *quit = new QPushButton( tr("Quit") , this );


  oLayout->addWidget(lb);
  oLayout->addWidget(infoText);
  oLayout->addWidget(progressBar);
  oLayout->addWidget(quit);
  
  connect( server, SIGNAL(newConnect(ClientSocket*)),
           SLOT(newConnect(ClientSocket*)) );
  connect( quit, SIGNAL(clicked()), this,
           SLOT(stop()) );
  exec();
  
}

void ServerInfo::newConnect(ClientSocket *s)
{
  infoText->append( tr("Beginning road test data conversion") + " ...\n" );
  infoText->append( tr("Conversation is in process") + " ...\n" );
  connect( s, SIGNAL(logText(const QString&)),
           infoText, SLOT(append(const QString&)) );

  connect( s, SIGNAL(progressValue(int)),
           progressBar, SLOT(setProgress(int)) );

  connect( s, SIGNAL(connectionClosed()),
           SLOT(connectionClosed()) );
}

void ServerInfo::connectionClosed() 
{
  int iBarResult;
  iBarResult = progressBar->progress();
  if ( ( iBarResult != -1) && (iBarResult != 100) ) {
    infoText->append( tr("WISIM road data conversion unsuccessful.") + "\n" );
    //infoText->append( tr("Read: '%1'\n").arg(iBarResult)); // for debug convinience
  }
  else 
    infoText->append( tr("Successfully finished conversion!") + "\n" );


}

void ServerInfo::stop() 
{
  
  delete this;
}


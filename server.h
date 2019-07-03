/****************************************************************************
** $Id: qt/server.cpp   3.3.1   edited May 27 2003 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
#ifndef SERVER_H
#define SERVER_H


#include <q3socket.h>
#include <q3serversocket.h>
#include <qapplication.h>
#include <q3vbox.h>
#include <q3textedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <q3textstream.h>
#include <qlayout.h>
#include <qdialog.h>
#include <q3progressbar.h>

#include <stdlib.h>


/*
  The ClientSocket class provides a socket that is connected with a client.
  For every client that connects to the server, the server creates a new
  instance of this class.
*/
class ClientSocket : public Q3Socket
{
    Q_OBJECT
public:
    ClientSocket( int sock, QObject *parent=0, const char *name=0 );

    ~ClientSocket()    {    }

signals:
    void progressValue(int);
    void logText( const QString& );

private slots:
  void readClient();


private:
    int line;
};


/*
  The SimpleServer class handles new connections to the server. For every
  client that connects, it creates a new ClientSocket -- that instance is now
  responsible for the communication with that client.
*/
class SimpleServer : public Q3ServerSocket
{
    Q_OBJECT
public:
    SimpleServer( QObject* parent=0 );

    ~SimpleServer()
    {
    }

    void newConnection( int socket );


signals:
    void newConnect( ClientSocket* );
};


/*
  The ServerInfo class provides a small GUI for the server. It also creates the
  SimpleServer and as a result the server.
*/
class ServerInfo : public QDialog
{
    Q_OBJECT
public:
    ServerInfo(QWidget* parent = 0, const char* name = "Road Test Points Convert", 
               bool modal = TRUE);
    ~ServerInfo()
    {
    }

private slots:
  void newConnect( ClientSocket *s );
  void connectionClosed();
  void stop();
 

private:
    Q3TextEdit *infoText;
    Q3ProgressBar *progressBar;
};


#endif

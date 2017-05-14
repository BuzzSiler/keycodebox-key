#include "smtp/SmtpMime"

#include <QDebug>
#include <QtNetwork/QTcpSocket>

Smtp::Smtp( const QString &from, const QString &to, const QString &subject, const QString &body )
{
    socket = new QTcpSocket( this );

    connect( socket, SIGNAL( readyRead() ), this, SLOT( readyRead() ) );
    connect( socket, SIGNAL( connected() ), this, SLOT( connected() ) );
    connect( socket, SIGNAL(error(SocketError)), this,
        SLOT(errorReceived(SocketError)));
    connect( socket, SIGNAL(stateChanged( SocketState)), this,
        SLOT(stateChanged(SocketState)));
    connect(socket, SIGNAL(disconnectedFromHost()), this,
        SLOT(disconnected()));;

    message = "To: " + to + "\n";
    message.append("From: " + from + "\n");
    message.append("Subject: " + subject + "\n");
    message.append(body);
    message.replace( QString::fromLatin1( "\n" ), QString::fromLatin1( "\r\n" ) );
    message.replace( QString::fromLatin1( "\r\n.\r\n" ),
    QString::fromLatin1( "\r\n..\r\n" ) );
    this->from = from;
    rcpt = to;
    state = Init;
    socket->connectToHost( "smtp.yourserver.com", 25);

    if(socket->waitForConnected ( 30000 )) 	{qDebug("connected"); 	}

    t = new QTextStream( socket );
}
Smtp::~Smtp()
{
    delete t;
    delete socket;
}
void Smtp::stateChanged(QTcpSocket::SocketState socketState)
{
    QString     value;
    switch(socketState)
    {
    case QTcpSocket::SocketState::LowDelayOption: // TCP_NODELAY
        value = "LowDelayOption";
        break;
    case QTcpSocket::SocketState::KeepAliveOption: // SO_KEEPALIVE
        value = "KeepAliveOption";
        break;
    case QTcpSocket::SocketState::MulticastTtlOption: // IP_MULTICAST_TTLl
        value = "MulticastTtlOption";
        break;
    case QTcpSocket::SocketState::MulticastLoopbackOption: // IP_MULTICAST_LOOPBACK
        value = "MulticaseLoopbackOption";
        break;
    case QTcpSocket::SocketState::TypeOfServiceOption: //IP_TOS
        value = "TypeOfServiceOption";
        break;
    case QTcpSocket::SocketState::SendBufferSizeSocketOption:    //SO_SNDBUF
        value = "SendBufferSizeSocketOption";
        break;
    case QTcpSocket::SocketState::ReceiveBufferSizeSocketOption:  //SO_RCVBUF
        value = "ReceiveBufferSizeSocketOption";
        break;
    }

    qDebug() << "stateChanged " << value;
}

void Smtp::errorReceived(QTcpSocket::SocketError socketError)
{
    switch(socketError)
    {
    case QTcpSocket::SocketError::UnconnectedState:
        qDebug() << "error UnconnectedState";
        break;
    case QTcpSocket::SocketError::HostLookupState:
        qDebug() << "error HostLookupState";
        break;
    case QTcpSocket::SocketError::ConnectingState:
        qDebug() << "error ConnectingState";
        break;
    case QTcpSocket::SocketError::ConnectedState:
        qDebug() << "error ConnectedState";
        break;
    case QTcpSocket::SocketError::BoundState:
        qDebug() << "error BoundState";
        break;
    case QTcpSocket::SocketError::ListeningState:
        qDebug() << "error ListeningState";
        break;
    case QTcpSocket::SocketError::ClosingState:
        qDebug() << "error ClosingState";
        break;
    };

    qDebug() << "error " <<socketError;
}

void Smtp::disconnected()
{

    qDebug() <<"disconnected";
    qDebug() << "error "  << socket->errorString();
}

void Smtp::connected()
{
//    output->append("connected");
    qDebug() << "Connected ";
}

void Smtp::readyRead()
{

     qDebug() <<"readyRead";
    // SMTP is line-oriented

    QString responseLine;
    do
    {
        responseLine = socket->readLine();
        response += responseLine;
    }
    while ( socket->canReadLine() && responseLine[3] != ' ' );

    responseLine.truncate( 3 );


    if ( state == Init && responseLine[0] == '2' )
    {
        // banner was okay, let's go on

        *t << "HELO there\r\n";
        t->flush();

        state = Mail;
    }
    else if ( state == Mail && responseLine[0] == '2' )
    {
        // HELO response was okay (well, it has to be)

        *t << "MAIL FROM: " << from << "\r\n";
        t->flush();
        state = Rcpt;
    }
    else if ( state == Rcpt && responseLine[0] == '2' )
    {

        *t << "RCPT TO: " << rcpt << "\r\n"; //r
        t->flush();
        state = Data;
    }
    else if ( state == Data && responseLine[0] == '2' )
    {

        *t << "DATA\r\n";
        t->flush();
        state = Body;
    }
    else if ( state == Body && responseLine[0] == '3' )
    {

        *t << message << "\r\n.\r\n";
        t->flush();
        state = Quit;
    }
    else if ( state == Quit && responseLine[0] == '2' )
    {

        *t << "QUIT\r\n";
        t->flush();
        // here, we just close.
        state = Close;
        emit status( tr( "Message sent" ) );
    }
    else if ( state == Close )
    {
        deleteLater();
        return;
    }
    else
    {
        // something broke.
        QMessageBox::warning( 0, tr( "Qt Mail Example" ), tr( "Unexpected reply from SMTP server:\n\n" ) + response );
        state = Close;
    }
    response = "";
}

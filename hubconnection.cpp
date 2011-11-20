#include "hubconnection.h"

HubConnection::HubConnection(QObject *parent) :
    QObject(parent)
{
    hubSocket = new QTcpSocket(this);
    connect(hubSocket, SIGNAL(readyRead()), this, SLOT(processHubMessage()));
	connect(hubSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));

	//Initialize variables - if they're not going to be set
	hubAddress = "127.0.0.1";
	hubPort = 4012;
	nick = "anon";
	password = "pass";
	version = "0";
	hubIsOnline = false;

	reconnectTimer = new QTimer(this);
	reconnectTimer->setInterval(30000); //Try to reconnect every 30 seconds
	connect(reconnectTimer, SIGNAL(timeout()), this, SLOT(reconnectTimeout()));
}

HubConnection::HubConnection(QString address, quint16 port, QString nick, QString password, QString version, QObject *parent) : QObject(parent)
{
	hubSocket = new QTcpSocket(this);
    connect(hubSocket, SIGNAL(readyRead()), this, SLOT(processHubMessage()));
	connect(hubSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));

	//Initialize variables - use set functions for type checking
	setHubAddress(address);
	setHubPort(port);
	setNick(nick);
	setPassword(password);
	setVersion(version);
	hubIsOnline = false;

	reconnectTimer = new QTimer(this);
	reconnectTimer->setInterval(30000); //Try to reconnect every 30 seconds
	connect(reconnectTimer, SIGNAL(timeout()), this, SLOT(reconnectTimeout()));
}

HubConnection::~HubConnection()
{
	//Destructor
	reconnectTimer->deleteLater();
	hubSocket->close();
	hubSocket->deleteLater();
}

void HubConnection::setHubAddress(QString addr)
{
    hubAddress = addr;
}

void HubConnection::setHubPort(quint16 port)
{
    hubPort = port;
}

void HubConnection::setNick(QString n)
{
    // TODO: check for invalid nick chars
    nick = n;
}

void HubConnection::setPassword(QString p)
{
    // TOOD: validity checks
    password = p;
}

void HubConnection::setVersion(QString v)
{
	version = v;
}

void HubConnection::sendChatMessage(QString message)
{
    QByteArray data;
    data.append("<");
    data.append(nick);
    data.append("> ");
    data.append(escapeDCProtocol(message));
    data.append("|");
    hubSocket->write(data);
}

void HubConnection::sendPrivateMessage(QString otherNick, QString message)
{
    QByteArray data;
    data.append("$To: ");
    data.append(otherNick);
    data.append(" From: ");
    data.append(nick);
    data.append(" $<");
    data.append(nick);
    data.append("> ");
    data.append(escapeDCProtocol(message));
    data.append("|");
    hubSocket->write(data);
}

QString HubConnection::escapeDCProtocol(QString msg)
{
    msg.replace(QString("$"), QString("&#36;"));
    msg.replace(QString("|"), QString("&#124;"));
    return msg;
}

QString HubConnection::unescapeDCProtocol(QString msg)
{
    msg.replace(QString("&#36;"), QString("$"));
    msg.replace(QString("&#124;"), QString("|"));
    return msg;
}

void HubConnection::processHubMessage()
{
	//Hub is considered online if it receives messages - for now
	if (!hubIsOnline)
	{
		hubIsOnline = true;
		if (reconnectTimer->isActive())
			reconnectTimer->stop();
		emit hubOnline();
	}

    while (hubSocket->bytesAvailable() > 0)
    {
        dataReceived.append(hubSocket->readAll());

		//Check if a full command was received and flag if not
        bool lastMessageHalf = false;
        if (QString(dataReceived.right(1)).compare("|") != 0)
            lastMessageHalf = true;

        QStringList msgs = QString(dataReceived).split("|");
        dataReceived.clear();

        QStringListIterator it(msgs);
        while (it.hasNext())
        {
            QString msg = it.next();
			//If a full command wasn't received, leave the rest of the data in the cache for the next read
            if (lastMessageHalf && !it.hasNext())
            {
                dataReceived.append(msg);
                continue;
            }

            if (msg.length() == 0)
                continue;

			//If command
            if (msg.mid(0, 1).compare("$") == 0)
            {
                QByteArray sendData;

				//===== MY INFO COMMAND =====
                if (msg.mid(0, 7).compare("$MyINFO") == 0)
                {
                    // $MyINFO $ALL nick description<ArpmanetDC V:0.1,M:A, etc
                    int pos1 = msg.indexOf(" ", 13);
                    QString nickname = msg.mid(13, pos1 - 13);
                    int pos2 = msg.indexOf("<", pos1 + 1);
                    QString description = msg.mid(pos1 + 1, pos2 - pos1 - 1);
                    pos1 = msg.indexOf("M:", pos2 + 1);
                    QString mode = msg.mid(pos1 + 2, 1);
                    emit receivedMyINFO(nickname, description, mode);
                }

				//===== USER LEFT =====
                else if (msg.mid(0, 5).compare("$Quit") == 0)
                {
                    emit userLoggedOut(msg.mid(6));
                }

				//===== AUTHENTICATE CAPABILITIES =====
                else if (msg.mid(0, 5).compare("$Lock") == 0)
                {
                    sendData.append("$Supports NoGetINFO NoHello TTHSearch |$Key ABCABCABC|$ValidateNick ");
                    sendData.append(nick);
                    sendData.append("|");
                    hubSocket->write(sendData);
                }

				//===== AUTHENTICATE USER =====
                else if (msg.mid(0, 6).compare("$Hello") == 0)
                {
                    if (msg.mid(7).compare(nick) == 0)
                    {
                        sendData.append("$Version 1,0091|$GetNickList|$MyINFO $ALL ");
                        sendData.append(nick);
                        sendData.append(" [L:2.00 MiB/s]<ArpmanetDC V:");
						sendData.append(version);
						sendData.append(",M:A,H:");
                        if (registeredUser)
                            sendData.append("0/1/0");
                        else
                            sendData.append("1/0/0");
                        sendData.append(",S:5>$ $100.00 KiB/s$$0$|");
                        hubSocket->write(sendData);
                    }
                    else
                    {
                        emit(userLoggedIn(msg.mid(7)));
                    }
                }

				//===== AUTHENTICATE PASSWORD =====
                else if (msg.mid(0, 8).compare("$GetPass") == 0)
                {
                    sendData.append("$MyPass ");
                    sendData.append(password);
                    sendData.append("|");
                    hubSocket->write(sendData);
                    registeredUser = true;
                }

				//===== PRIVATE MESSAGE =====
                else if (msg.mid(0, 4).compare("$To:") == 0)
                {
                    QString str = tr("%1 From: ").arg(nick);
                    int pos1 = msg.indexOf(str);
                    int pos2 = msg.indexOf(" ", pos1 + str.size());
                    QString otherNick = msg.mid(pos1 + str.size(), pos2 - pos1 - str.size());
                    QString message = msg.mid(pos2 + 2);
                    emit receivedPrivateMessage(otherNick, escapeDCProtocol(message));  // TODO: nie seker of indekse en offsets reg is nie, check!
                }

				//===== NICKNAME LIST =====
                else if (msg.mid(0,9).compare("$NickList") == 0)
                {
                    msg = msg.mid(msg.indexOf("$$")+2);
                    QStringList nicks = msg.split("$$");
                    emit receivedNickList(nicks);
                }

				//===== OP LIST =====
                else if (msg.mid(0,7).compare("$OpList") == 0)
                {
                    msg = msg.mid(msg.indexOf("$$")+2);
                    QStringList nicks = msg.split("$$");
                    emit receivedOpList(nicks);
                }
            }
            else
				//===== MAIN CHAT MESSAGE =====
                emit receivedChatMessage(unescapeDCProtocol(msg));

        }
    }
}

void HubConnection::connectHub()
{
	//Make sure there is something to connect to
	if (!hubAddress.isEmpty() && hubPort != 0)
	{
		//Check if not already connecting - in that case just wait
		if (hubSocket->state() != QAbstractSocket::ConnectingState)
		{
			registeredUser = false;
			hubSocket->connectToHost(hubAddress, hubPort);
		}
	}
	else
	{
		//If address/port is not valid, stop reconnection process cause it's never going to work
		if (hubIsOnline)
		{
			hubIsOnline = false;
			emit hubOffline();
		}
		if (reconnectTimer->isActive())
			reconnectTimer->stop();
	}
}

void HubConnection::socketError(QAbstractSocket::SocketError error)
{
	//Parse error
	QString errorString;

	switch(error)
	{
		case QAbstractSocket::ConnectionRefusedError:
			errorString = "Connection refused by server. Retrying...";
			break;

		case QAbstractSocket::RemoteHostClosedError:
			errorString = "Remote host forcibly closed the connection. Retrying...";
			break;

		case QAbstractSocket::NetworkError:
			errorString = "Network error - Retrying...";
			break;

		case QAbstractSocket::HostNotFoundError:
			errorString = "Remote host could not be found. Retrying...";
			break;

		default:
			errorString = tr("Error occurred: %1").arg(hubSocket->errorString());
			break;
	}
		
	//Only try reconnecting if there was a connection error - otherwise just show there was an error
	if (hubSocket->state() == QAbstractSocket::UnconnectedState || hubSocket->state() == QAbstractSocket::ClosingState)
	{
		//Try again if it was a connection error
		if (!reconnectTimer->isActive())
			reconnectTimer->start();

		//If hub was online, change status to offline
		if (hubIsOnline)
		{
			hubIsOnline = false;
			emit hubOffline();
		}
	}	

	emit hubError(errorString);
}

void HubConnection::reconnectTimeout()
{
	//Retry connection
	connectHub();
}
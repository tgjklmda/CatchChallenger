#ifndef CLIENTNETWORKWRITE_H
#define CLIENTNETWORKWRITE_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>

class ClientNetworkWrite : public QObject
{
    Q_OBJECT
public:
	explicit ClientNetworkWrite();
	~ClientNetworkWrite();
	void setSocket(QTcpSocket * socket);
public slots:
	void sendPacket(const QByteArray &data);
	//normal slots
	void askIfIsReadyToStop();
	void stop();
private:
	QTcpSocket * socket;
	QByteArray block;
	qint64 byteWriten;
signals:
	void isReadyToStop();
	void fake_send_data(const QByteArray &data);
	void error(const QString &error);
	void message(const QString &message);
};

#endif // CLIENTNETWORKWRITE_H
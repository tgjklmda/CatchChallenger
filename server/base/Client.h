#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QTimer>

#include "../../general/base/DebugClass.h"
#include "ServerStructures.h"
#include "ClientBroadCast.h"
#include "ClientLocalBroadcast.h"
#include "ClientHeavyLoad.h"
#include "ClientMapManagement/ClientMapManagement.h"
#include "ClientNetworkRead.h"
#include "ClientNetworkWrite.h"
#include "LocalClientHandler.h"
#include "EventThreader.h"
#include "BroadCastWithoutSender.h"
#include "../../general/base/GeneralStructures.h"
#include "../VariableServer.h"
#include "ClientMapManagement/MapVisibilityAlgorithm_Simple.h"
#include "ClientMapManagement/MapVisibilityAlgorithm_None.h"

#ifndef CATCHCHALLENGER_CLIENT_H
#define CATCHCHALLENGER_CLIENT_H

namespace CatchChallenger {
class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(ConnectedSocket *socket, bool isFake, ClientMapManagement *clientMapManagement);
    ~Client();
    //to get some info
    QString getPseudo();
private:
    bool ask_is_ready_to_stop;
    //-------------------
    Player_internal_informations player_informations;
    ClientBroadCast *clientBroadCast;
    ClientHeavyLoad *clientHeavyLoad;
    ClientMapManagement *clientMapManagement;
    ClientNetworkRead *clientNetworkRead;
    ClientNetworkWrite *clientNetworkWrite;
    LocalClientHandler *localClientHandler;
    ClientLocalBroadcast *clientLocalBroadcast;

    //socket related
    ConnectedSocket *socket;
    QString remote_ip;
    quint16 port;

    quint8 stopped_object;
private slots:
    //socket related
    void connectionError(QAbstractSocket::SocketError);
    //normal management related
    void errorOutput(QString errorString);
    void kicked();
    void normalOutput(QString message);
    //internal management related
    void send_player_informations();
    //remove and stop related
    void disconnectNextStep();
signals:
    //remove and stop related
    void askIfIsReadyToStop();
    void isReadyToDelete();

    //to async the message
    void send_fakeLogin(quint32 last_fake_player_id,quint16 x,quint16 y,Map_server_MapVisibility_simple *map,Orientation orientation,QString skin);
    void fake_send_data(const QByteArray &data);
    void fake_send_received_data(const QByteArray &data);
    void try_ask_stop();
public slots:
    void disconnectClient();
    /// \warning it need be complete protocol trame
    void fake_receive_data(QByteArray data);
};
}

#endif // CLIENT_H

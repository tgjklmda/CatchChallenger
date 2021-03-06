#include "ClientMapManagement/MapBasicMove.h"

#ifndef CATCHCHALLENGER_CLIENTLOCALBROADCAST_H
#define CATCHCHALLENGER_CLIENTLOCALBROADCAST_H

#include <QObject>
#include <QList>
#include <QString>
#include <QTimer>
#include <QHash>
#include <QHashIterator>
#include <QSqlQuery>
#include <QSqlError>

#include "../../general/base/DebugClass.h"
#include "ServerStructures.h"
#include "EventThreader.h"
#include "../VariableServer.h"
#include "MapServer.h"
#include "SqlFunction.h"

/** \brief broadcast at the local map */

namespace CatchChallenger {
class ClientLocalBroadcast : public MapBasicMove
{
    Q_OBJECT
public:
    explicit ClientLocalBroadcast();
    virtual ~ClientLocalBroadcast();
public slots:
    //chat
    void sendLocalChatText(const QString &text);
    void receiveChatText(const QString &text, const Player_internal_informations *sender_informations);
    //map move
    bool singleMove(const Direction &direction);
    virtual void put_on_the_map(Map *map,const /*COORD_TYPE*/quint8 &x,const /*COORD_TYPE*/quint8 &y,const Orientation &orientation);
    virtual void teleportValidatedTo(Map *map,const /*COORD_TYPE*/quint8 &x,const /*COORD_TYPE*/quint8 &y,const Orientation &orientation);
    //seed
    void seedValidated();
    virtual void plantSeed(const quint8 &query_id,const quint8 &plant_id);
    virtual void collectPlant(const quint8 &query_id);
protected:
    virtual void extraStop();
    virtual void insertClient(Map *map);
    virtual void removeClient(Map *map,const bool &withDestroy=false);
    virtual void sendNearPlant();
    virtual void removeNearPlant();
    virtual void receiveSeed(const MapServerCrafting::PlantOnMap &plantOnMap,const quint64 &current_time);
    virtual void removeSeed(const MapServerCrafting::PlantOnMap &plantOnMap);

    struct PlantInWaiting
    {
        quint8 query_id;
        quint8 plant_id;
        Map *map;
        quint8 x,y;
    };
    QList<PlantInWaiting> plant_list_in_waiting;
signals:
    //send reply
    void postReply(const quint8 &queryNumber,const QByteArray &data);
    //seed
    void useSeed(const quint8 &plant_id);
    void addObjectAndSend(const quint32 &item,const quint32 &quantity=1);
    //db
    void dbQuery(const QString &sqlQuery);
};
}

#endif // CATCHCHALLENGER_CLIENTLOCALBROADCAST_H

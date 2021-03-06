#ifndef CATCHCHALLENGER_MAP_CLIENT_H
#define CATCHCHALLENGER_MAP_CLIENT_H

#include "../../general/base/Map.h"
#include "../../general/base/GeneralStructures.h"
#include "../../general/libtiled/mapobject.h"
#include "ClientStructures.h"
#include "DisplayStructures.h"

#include <QString>
#include <QList>
#include <QHash>
#include <QDomElement>

namespace CatchChallenger {
class Map_client : public Map
{
public:
    QList<Map_semi_teleport> teleport_semi;
    QList<Map_to_send::Map_Point> rescue_points;
    QList<Map_to_send::Map_Point> bot_spawn_points;

    Map_semi_border border_semi;

    Map_client();

    struct Plant
    {
        Tiled::MapObject * mapObject;
        quint8 x,y;
        quint8 plant_id;
        quint64 mature_at;
    };
    QList<Plant> plantList;
    QHash<QPair<quint8,quint8>,Bot> bots;
    QHash<QPair<quint8,quint8>,BotDisplay> botsDisplay;
};
}

#endif // MAP_SERVER_H

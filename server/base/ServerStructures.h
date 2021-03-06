#ifndef CATCHCHALLENGER_SERVER_STRUCTURES_H
#define CATCHCHALLENGER_SERVER_STRUCTURES_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QString>
#include <QSqlDatabase>
#include <QTimer>
#include <QMutex>
#include <QHash>
#include <QVariant>
#include <QSet>
#include <QSqlQuery>
#include <QDataStream>
#include <QMultiHash>

#include "../../general/base/GeneralStructures.h"
#include "../../general/base/ConnectedSocket.h"
#include "PlayerUpdater.h"

namespace CatchChallenger {
class EventThreader;
class Map_custom;
class Map;
class ClientBroadCast;
class ClientMapManagement;
class FakeBot;
class PlayerUpdater;
class Map_server_MapVisibility_simple;

struct Map_player_info
{
    Map *map;
    int x,y;
    QString skin;
};

enum MapVisibilityAlgorithm
{
    MapVisibilityAlgorithm_simple,
    MapVisibilityAlgorithm_none
};

struct Player_internal_informations
{
    Player_private_and_public_informations public_and_private_informations;
    bool isFake;
    bool is_logged;
    quint32 id;
    QByteArray rawPseudo;
    volatile bool isConnected;
    struct Rescue
    {
        Map* map;
        COORD_TYPE x;
        COORD_TYPE y;
        Orientation orientation;
    };
    Rescue rescue;
};

struct ServerSettings
{
    //the listen, implicit on the client
    quint16 server_port;
    QString server_ip;

    //settings of the server shared with the client
    CommmonServerSettings commmonServerSettings;

    //rates
    qreal rates_xp_premium;
    qreal rates_gold_premium;
    qreal rates_shiny_premium;

    struct Database
    {
        enum Type
        {
            DatabaseType_Mysql,
            DatabaseType_SQLite
        };
        Type type;

        enum FightSync
        {
            FightSync_AtEachTurn=0x00,
            FightSync_AtTheEndOfBattle=0x01,//or at the object usage
            FightSync_AtTheDisconnexion=0x02
        };
        FightSync fightSync;
        bool positionTeleportSync;
        quint32 secondToPositionSync;//0 is disabled

        struct Mysql
        {
            //mysql settings
            QString host;
            QString db;
            QString login;
            QString pass;
        };
        Mysql mysql;
        struct SQLite
        {
            QString file;
        };
        SQLite sqlite;
    };
    Database database;

    //connection
    quint16 max_players;
    bool tolerantMode;

    //visibility algorithm
    struct MapVisibility
    {
        MapVisibilityAlgorithm mapVisibilityAlgorithm;

        struct MapVisibility_simple
        {
            quint16 max;
            quint16 reshow;
        };
        MapVisibility_simple simple;
    };
    MapVisibility mapVisibility;
};

struct Plant
{
    quint32 itemUsed;
    quint32 mature_seconds;
    //float quantity;//splited into 2 integer
    quint16 fix_quantity;
    quint16 random_quantity;
};

struct Item
{
    quint32 price;
};

struct MonsterDrops
{
    quint32 item;
    quint32 quantity_min;
    quint32 quantity_max;
    quint32 luck;
};

struct ServerPrivateVariables
{
    //bd
    QSqlDatabase *db;//use pointer here to init in correct thread
    QString db_type_string;

    //datapack
    QString datapack_basePath;
    QString datapack_mapPath;
    QRegExp datapack_rightFileName;
    QHash<quint32,Item> items;
    QHash<QString,quint64> filesList;
    QHash<quint8,Plant> plants;
    QHash<quint32,CrafingRecipe> crafingRecipes;
    QHash<quint32,Shop> shops;
    QHash<quint32,quint32> itemToCrafingRecipes;

    //fight
    QHash<quint32,Monster> monsters;
    QMultiHash<quint32,MonsterDrops> monsterDrops;
    QHash<quint32,Monster::Skill> monsterSkills;
    QHash<quint32,Monster::Buff> monsterBuffs;

    //general data
    QList<EventThreader *> eventThreaderList;
    QTimer *timer_player_map;
    bool stopIt;

    //interconnected thread
    //QMutex clientBroadCastListMutex;

    //datapack
    QHash<QString,quint8> skinList;

    //map
    QHash<QString,Map *> map_list;
    QTimer timer_to_send_insert_move_remove;/// \todo put on timer by thread without Qt::QueuedConnection to improve the performance
    QTimer positionSync;/// \todo put into the local thread to drop Qt::QueuedConnection and improve the performance
    qint8 sizeofInsertRequest;

    //connection
    quint16 connected_players;
    PlayerUpdater player_updater;
    QSet<quint32> connected_players_id_list;

    //bot
    struct BotSpawn
    {
        QString map;
        COORD_TYPE x;
        COORD_TYPE y;
    };
    QList<BotSpawn> botSpawn;
    QSet<FakeBot *> fakeBotList;
    QSet<QFakeSocket *> botSockets;
    quint32 number_of_bots_logged;
    int botSpawnIndex;

    //xp rate at form for level to xp: a*exp(x*b+c)+d
    struct Xp
    {
        quint32 a,b,c,d;
    };
    //xp rate at form for level to xp: a*exp(x*b+c)+d
    struct Sp
    {
        quint32 a,b,c,d;
    };
};



}

#endif // STRUCTURES_SERVER_H

#include "MapVisibilityAlgorithm_Simple.h"
#include "../GlobalServerData.h"
#include "../../VariableServer.h"

using namespace CatchChallenger;

int MapVisibilityAlgorithm_Simple::index;
int MapVisibilityAlgorithm_Simple::loop_size;
MapVisibilityAlgorithm_Simple *MapVisibilityAlgorithm_Simple::current_client;

//temp variable for purge buffer
QByteArray MapVisibilityAlgorithm_Simple::purgeBuffer_outputData;
int MapVisibilityAlgorithm_Simple::purgeBuffer_index;
int MapVisibilityAlgorithm_Simple::purgeBuffer_list_size;
int MapVisibilityAlgorithm_Simple::purgeBuffer_list_size_internal;
int MapVisibilityAlgorithm_Simple::purgeBuffer_indexMovement;
map_management_move MapVisibilityAlgorithm_Simple::purgeBuffer_move;
QHash<SIMPLIFIED_PLAYER_ID_TYPE, QList<map_management_movement> >::const_iterator MapVisibilityAlgorithm_Simple::i_move;
QHash<SIMPLIFIED_PLAYER_ID_TYPE, QList<map_management_movement> >::const_iterator MapVisibilityAlgorithm_Simple::i_move_end;
QHash<SIMPLIFIED_PLAYER_ID_TYPE, MapVisibilityAlgorithm_Simple *>::const_iterator MapVisibilityAlgorithm_Simple::i_insert;
QHash<SIMPLIFIED_PLAYER_ID_TYPE, MapVisibilityAlgorithm_Simple *>::const_iterator MapVisibilityAlgorithm_Simple::i_insert_end;
QSet<SIMPLIFIED_PLAYER_ID_TYPE>::const_iterator MapVisibilityAlgorithm_Simple::i_remove;
QSet<SIMPLIFIED_PLAYER_ID_TYPE>::const_iterator MapVisibilityAlgorithm_Simple::i_remove_end;
Map* MapVisibilityAlgorithm_Simple::old_map;
Map* MapVisibilityAlgorithm_Simple::new_map;
bool MapVisibilityAlgorithm_Simple::mapHaveChanged;

//temp variable to move on the map
map_management_movement MapVisibilityAlgorithm_Simple::moveClient_tempMov;

MapVisibilityAlgorithm_Simple::MapVisibilityAlgorithm_Simple()
{
    #ifdef CATCHCHALLENGER_SERVER_MAP_DROP_BLOCKED_MOVE
    previousMovedUnitBlocked=0;
    #endif
}

MapVisibilityAlgorithm_Simple::~MapVisibilityAlgorithm_Simple()
{
}

void MapVisibilityAlgorithm_Simple::insertClient()
{
    loop_size=static_cast<Map_server_MapVisibility_simple*>(map)->clients.size();
    if(likely(loop_size<=GlobalServerData::serverSettings.mapVisibility.simple.max))
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        emit message(QString("insertClient() insert the client, into: %1 (%2,%3)").arg(map->map_file).arg(x).arg(y));
        #endif
        //insert the new client
        index=0;
        while(index<loop_size)
        {
            current_client=static_cast<Map_server_MapVisibility_simple*>(map)->clients.at(index);
            current_client->insertAnotherClient(player_informations->public_and_private_informations.public_informations.simplifiedId,this);
            this->insertAnotherClient(current_client->player_informations->public_and_private_informations.public_informations.simplifiedId,current_client);
            index++;
        }
    }
    else
    {
        if(unlikely(static_cast<Map_server_MapVisibility_simple*>(map)->show))
        {
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            emit message(QString("insertClient() too many client, hide now, into: %1").arg(map->map_file));
            #endif
            static_cast<Map_server_MapVisibility_simple*>(map)->show=false;
            //drop all show client because it have excess the limit
            //drop on all client
            index=0;
            while(index<loop_size)
            {
                static_cast<Map_server_MapVisibility_simple*>(map)->clients.at(index)->dropAllClients();
                index++;
            }
        }
        else
        {
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            emit message(QString("insertClient() already too many client, into: %1").arg(map->map_file));
            #endif
        }
    }
    //auto insert to know where it have spawn, now in charge of ClientLocalCalcule
    //insertAnotherClient(player_id,current_map,x,y,last_direction,speed);
}

void MapVisibilityAlgorithm_Simple::moveClient(const quint8 &movedUnit,const Direction &direction)
{
    loop_size=static_cast<Map_server_MapVisibility_simple*>(map)->clients.size();
    if(unlikely(mapHaveChanged))
    {
        #ifdef DEBUG_MESSAGE_CLIENT_MOVE
        emit message(QString("map have change, direction: %4: (%1,%2): %3, send at %5 player(s)").arg(x).arg(y).arg(player_informations->public_and_private_informations.public_informations.simplifiedId).arg(MoveOnTheMap::directionToString(direction)).arg(loop_size-1));
        #endif
        if(likely(loop_size<=GlobalServerData::serverSettings.mapVisibility.simple.max))
        {
            //insert the new client, do into insertClient(), call by singleMove()
        }
        else
        {
            //drop all show client because it have excess the limit, do into removeClient(), call by singleMove()
        }
    }
    else
    {
        //here to know how player is affected
        #ifdef DEBUG_MESSAGE_CLIENT_MOVE
        emit message(QString("after %4: (%1,%2): %3, send at %5 player(s)").arg(x).arg(y).arg(player_informations->public_and_private_informations.public_informations.simplifiedId).arg(MoveOnTheMap::directionToString(direction)).arg(loop_size-1));
        #endif

        //normal operation
        if(likely(loop_size<=GlobalServerData::serverSettings.mapVisibility.simple.max))
        {
            index=0;
            while(index<loop_size)
            {
                current_client=static_cast<Map_server_MapVisibility_simple*>(map)->clients.at(index);
                if(likely(current_client!=this))
                     current_client->moveAnotherClientWithMap(player_informations->public_and_private_informations.public_informations.simplifiedId,this,movedUnit,direction);
                index++;
            }
        }
        else //all client is dropped due to over load on the map
        {
        }
    }
}

//drop all clients
void MapVisibilityAlgorithm_Simple::dropAllClients()
{
    to_send_insert.clear();
    to_send_move.clear();
    to_send_remove.clear();
    to_send_over_move.clear();

    ClientMapManagement::dropAllClients();
}

void MapVisibilityAlgorithm_Simple::removeClient()
{
    loop_size=static_cast<Map_server_MapVisibility_simple*>(map)->clients.size();
    if(unlikely(loop_size==(GlobalServerData::serverSettings.mapVisibility.simple.reshow) && static_cast<Map_server_MapVisibility_simple*>(map)->show==false))
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        emit message(QString("removeClient() client of the map is now under the limit, reinsert all into: %1").arg(map->map_file));
        #endif
        static_cast<Map_server_MapVisibility_simple*>(map)->show=true;
        //insert all the client because it start to be visible
        index=0;
        while(index<loop_size)
        {
            static_cast<Map_server_MapVisibility_simple*>(map)->clients.at(index)->reinsertAllClient();
            index++;
        }
    }
    //nothing removed because all clients are already hide
    else if(unlikely(loop_size>(GlobalServerData::serverSettings.mapVisibility.simple.max+1)))
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        emit message(QString("removeClient() do nothing because client hiden, into: %1").arg(map->map_file));
        #endif
    }
    else //normal working
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        emit message(QString("removeClient() normal work, just remove from client on: %1").arg(map->map_file));
        #endif
        index=0;
        while(index<loop_size)
        {
            current_client=static_cast<Map_server_MapVisibility_simple*>(map)->clients.at(index);
            current_client->removeAnotherClient(player_informations->public_and_private_informations.public_informations.simplifiedId);
            this->removeAnotherClient(current_client->player_informations->public_and_private_informations.public_informations.simplifiedId);
            index++;
        }
    }
}

void MapVisibilityAlgorithm_Simple::mapVisiblity_unloadFromTheMap()
{
    removeClient();
}

void MapVisibilityAlgorithm_Simple::reinsertAllClient()
{
    #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
    emit message(QString("reinsertAllClient() %1)").arg(player_informations->public_and_private_informations.public_informations.simplifiedId));
    #endif
    index=0;
    while(index<loop_size)
    {
        current_client=static_cast<Map_server_MapVisibility_simple*>(map)->clients.at(index);
        if(likely(current_client!=this))
        {
            current_client->insertAnotherClient(player_informations->public_and_private_informations.public_informations.simplifiedId,this);
            this->insertAnotherClient(current_client->player_informations->public_and_private_informations.public_informations.simplifiedId,current_client);
        }
        index++;
    }
}

#ifdef CATCHCHALLENGER_SERVER_VISIBILITY_CLEAR
//remove the move/remove
void MapVisibilityAlgorithm_Simple::insertAnotherClient(const SIMPLIFIED_PLAYER_ID_TYPE &player_id,MapVisibilityAlgorithm_Simple *the_another_player)
{
    to_send_remove.remove(player_id);
    to_send_move.remove(player_id);

    #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
    emit message(QString("insertAnotherClient(%1,%2,%3,%4)").arg(player_id).arg(the_another_player->map->map_file).arg(the_another_player->x).arg(the_another_player->y));
    #endif
    to_send_insert[player_id]=the_another_player;
}
#endif

void MapVisibilityAlgorithm_Simple::moveAnotherClientWithMap(const SIMPLIFIED_PLAYER_ID_TYPE &player_id,MapVisibilityAlgorithm_Simple *the_another_player,const quint8 &movedUnit,const Direction &direction)
{
    #ifdef CATCHCHALLENGER_SERVER_MAP_DROP_OVER_MOVE
    //already into over move
    if(to_send_insert.contains(player_id) || to_send_over_move.contains(player_id))
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
        emit message(QString("moveAnotherClientWithMap(%1,%2,%3) to the player: %4, already into over move").arg(player_id).arg(movedUnit).arg(MoveOnTheMap::directionToString(direction)).arg(player_informations->public_and_private_informations.public_informations.simplifiedId));
        #endif
        //to_send_map_management_remove.remove(player_id); -> what?
        return;//quit now
    }
    //go into over move
    else if(unlikely(
                (to_send_move[player_id].size()*(sizeof(quint8)+sizeof(quint8))+sizeof(quint8))//the size of one move
                >=
                    //size of on insert
                    GlobalServerData::serverPrivateVariables.sizeofInsertRequest+player_informations->rawPseudo.size()
                ))
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
        emit message(QString("moveAnotherClientWithMap(%1,%2,%3) to the player: %4, go into over move").arg(player_id).arg(movedUnit).arg(MoveOnTheMap::directionToString(direction)).arg(player_informations->public_and_private_informations.public_informations.simplifiedId));
        #endif
        to_send_move.remove(player_id);
        to_send_over_move[player_id]=the_another_player;
        return;
    }
    #endif
    #ifdef CATCHCHALLENGER_SERVER_MAP_DROP_STOP_MOVE
    if(to_send_move.contains(player_id) && !to_send_move[player_id].isEmpty())
    {
        switch(to_send_move[player_id].last().direction)
        {
            case Direction_look_at_top:
            case Direction_look_at_right:
            case Direction_look_at_bottom:
            case Direction_look_at_left:
                #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
                emit message(QString("moveAnotherClientWithMap(%1,%2,%3) to the player: %4, compressed move").arg(player_id).arg(to_send_move[player_id].last().movedUnit).arg(MoveOnTheMap::directionToString(direction)).arg(player_informations->public_and_private_informations.public_informations.simplifiedId));
                #endif
                to_send_move[player_id].last().direction=direction;
            return;
            default:
            break;
        }
    }
    #endif
    #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
    emit message(QString("moveAnotherClientWithMap(%1,%2,%3) to the player: %4, normal move").arg(player_id).arg(movedUnit).arg(MoveOnTheMap::directionToString(direction)).arg(player_informations->public_and_private_informations.public_informations.simplifiedId));
    #endif
    moveClient_tempMov.movedUnit=movedUnit;
    moveClient_tempMov.direction=direction;
    to_send_move[player_id] << moveClient_tempMov;
}

#ifdef CATCHCHALLENGER_SERVER_VISIBILITY_CLEAR
//remove the move/insert
void MapVisibilityAlgorithm_Simple::removeAnotherClient(const SIMPLIFIED_PLAYER_ID_TYPE &player_id)
{
    #ifdef CATCHCHALLENGER_SERVER_EXTRA_CHECK
    if(unlikely(to_send_remove.contains(player_id)))
    {
        emit message("removeAnotherClient() try dual remove");
        return;
    }
    #endif

    to_send_insert.remove(player_id);
    to_send_move.remove(player_id);
    #ifdef CATCHCHALLENGER_SERVER_MAP_DROP_OVER_MOVE
    to_send_over_move.remove(player_id);
    #endif

    #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
    emit message(QString("removeAnotherClient(%1)").arg(player_id));
    #endif

    /* Do into the upper class, like MapVisibilityAlgorithm_Simple
     * #ifdef CATCHCHALLENGER_SERVER_VISIBILITY_CLEAR
    //remove the move/insert
    to_send_map_management_insert.remove(player_id);
    to_send_map_management_move.remove(player_id);
    #endif */
    to_send_remove << player_id;
}
#endif

void MapVisibilityAlgorithm_Simple::extraStop()
{
    unloadFromTheMap();//product remove on the map

    to_send_insert.clear();
    to_send_move.clear();
    to_send_remove.clear();
}

void MapVisibilityAlgorithm_Simple::purgeBuffer()
{
    send_insert();
    send_move();
    send_remove();
    #if defined(CATCHCHALLENGER_SERVER_VISIBILITY_CLEAR) && defined(CATCHCHALLENGER_SERVER_MAP_DROP_OVER_MOVE)
    send_reinsert();
    #endif
}

//for the purge buffer
void MapVisibilityAlgorithm_Simple::send_insert()
{
    if(to_send_insert.size()==0)
        return;
    #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
    emit message(QString("send_insert() of player: %4").arg(player_informations->public_and_private_informations.public_informations.simplifiedId));
    #endif

    purgeBuffer_outputData.clear();
    QDataStream out(&purgeBuffer_outputData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_4);

    //////////////////////////// insert //////////////////////////
    /* can be only this map with this algo, then 1 map */
    out << (quint8)0x01;
    if(GlobalServerData::serverPrivateVariables.map_list.size()<=255)
        out << (quint8)map->id;
    else if(GlobalServerData::serverPrivateVariables.map_list.size()<=65535)
        out << (quint16)map->id;
    else
        out << (quint32)map->id;
    if(GlobalServerData::serverSettings.max_players<=255)
        out << (quint8)to_send_insert.size();
    else
        out << (quint16)to_send_insert.size();

    i_insert = to_send_insert.constBegin();
    i_insert_end = to_send_insert.constEnd();
    while (i_insert != i_insert_end)
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
        emit message(
            QString("insert player_id: %1, mapName %2, x: %3, y: %4,direction: %5, for player: %6, direction | type: %7, rawPseudo: %8, skinId: %9")
            .arg(i_insert.key())
            .arg(i_insert.value()->map->map_file)
            .arg(i_insert.value()->x)
            .arg(i_insert.value()->y)
            .arg(MoveOnTheMap::directionToString(i_insert.value()->last_direction))
            .arg(player_informations->public_and_private_informations.public_informations.simplifiedId)
            .arg(((quint8)i_insert.value()->last_direction | (quint8)i_insert.value()->player_informations->public_and_private_informations.public_informations.type))
            .arg(QString(i_insert.value()->player_informations->rawPseudo.toHex()))
            .arg(i_insert.value()->player_informations->public_and_private_informations.public_informations.skinId)
             );
        #endif
        if(GlobalServerData::serverSettings.max_players<=255)
            out << (quint8)i_insert.key();
        else
            out << (quint16)i_insert.key();
        out << (COORD_TYPE)i_insert.value()->x;
        out << (COORD_TYPE)i_insert.value()->y;
        out << (quint8)((quint8)i_insert.value()->getLastDirection() | (quint8)i_insert.value()->player_informations->public_and_private_informations.public_informations.type);
        out << i_insert.value()->player_informations->public_and_private_informations.public_informations.speed;
        //clan
        out << i_insert.value()->player_informations->public_and_private_informations.public_informations.clan;
        //pseudo
        purgeBuffer_outputData+=i_insert.value()->player_informations->rawPseudo;
        out.device()->seek(out.device()->pos()+i_insert.value()->player_informations->rawPseudo.size());
        //skin
        out << i_insert.value()->player_informations->public_and_private_informations.public_informations.skinId;

        ++i_insert;
    }
    to_send_insert.clear();

    emit sendPacket(0xC0,purgeBuffer_outputData);
}

void MapVisibilityAlgorithm_Simple::send_move()
{
    if(to_send_move.size()==0)
        return;
    #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
    emit message(QString("send_move() of player: %4").arg(player_informations->public_and_private_informations.public_informations.simplifiedId));
    #endif

    purgeBuffer_outputData.clear();
    QDataStream out(&purgeBuffer_outputData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_4);

    //////////////////////////// move //////////////////////////
    purgeBuffer_list_size_internal=0;
    purgeBuffer_indexMovement=0;
    i_move = to_send_move.constBegin();
    i_move_end = to_send_move.constEnd();
    if(GlobalServerData::serverSettings.max_players<=255)
        out << (quint8)to_send_move.size();
    else
        out << (quint16)to_send_move.size();
    while (i_move != i_move_end)
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
        emit message(
            QString("move player_id: %1, for player: %2")
            .arg(i_move.key())
            .arg(player_informations->public_and_private_informations.public_informations.simplifiedId)
             );
        #endif
        if(GlobalServerData::serverSettings.max_players<=255)
            out << (quint8)i_move.key();
        else
            out << (quint16)i_move.key();

        purgeBuffer_list_size_internal=i_move.value().size();
        out << (quint8)purgeBuffer_list_size_internal;
        purgeBuffer_indexMovement=0;
        while(purgeBuffer_indexMovement<purgeBuffer_list_size_internal)
        {
            out << i_move.value().at(purgeBuffer_indexMovement).movedUnit;
            out << (quint8)i_move.value().at(purgeBuffer_indexMovement).direction;
            purgeBuffer_indexMovement++;
        }
        ++i_move;
    }
    to_send_move.clear();

    emit sendPacket(0xC7,purgeBuffer_outputData);
}

void MapVisibilityAlgorithm_Simple::send_remove()
{
    if(to_send_remove.size()==0)
        return;
    #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
    emit message(QString("send_remove() of player: %4").arg(player_informations->public_and_private_informations.public_informations.simplifiedId));
    #endif

    purgeBuffer_outputData.clear();
    QDataStream out(&purgeBuffer_outputData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_4);

    //////////////////////////// remove //////////////////////////
    i_remove = to_send_remove.constBegin();
    i_remove_end = to_send_remove.constEnd();
    if(GlobalServerData::serverSettings.max_players<=255)
        out << (quint8)to_send_remove.size();
    else
        out << (quint16)to_send_remove.size();
    while (i_remove != i_remove_end)
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
        emit message(
            QString("player_id to remove: %1, for player: %2")
            .arg((quint32)(*i_remove))
            .arg(player_informations->public_and_private_informations.public_informations.simplifiedId)
             );
        #endif
        if(GlobalServerData::serverSettings.max_players<=255)
            out << (quint8)*i_remove;
        else
            out << (quint16)*i_remove;
        ++i_remove;
    }
    to_send_remove.clear();

    emit sendPacket(0xC8,purgeBuffer_outputData);
}

#if defined(CATCHCHALLENGER_SERVER_VISIBILITY_CLEAR) && defined(CATCHCHALLENGER_SERVER_MAP_DROP_OVER_MOVE)
void MapVisibilityAlgorithm_Simple::send_reinsert()
{
    if(to_send_over_move.size()==0)
        return;
    #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
    emit message(QString("send_reinsert() of player: %4").arg(player_informations->public_and_private_informations.public_informations.simplifiedId));
    #endif

    purgeBuffer_outputData.clear();
    QDataStream out(&purgeBuffer_outputData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_4);

    //////////////////////////// re-insert //////////////////////////
    if(GlobalServerData::serverSettings.max_players<=255)
        out << (quint8)to_send_over_move.size();
    else
        out << (quint16)to_send_over_move.size();

    i_insert = to_send_over_move.constBegin();
    i_insert_end = to_send_over_move.constEnd();
    while (i_insert != i_insert_end)
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
        emit message(
            QString("reinsert player_id: %1, x: %2, y: %3,direction: %4, for player: %5")
            .arg(i_insert.key())
            .arg(i_insert.value()->x)
            .arg(i_insert.value()->y)
            .arg(MoveOnTheMap::directionToString(i_insert.value()->last_direction))
            .arg(player_informations->public_and_private_informations.public_informations.simplifiedId)
             );
        #endif
        if(GlobalServerData::serverSettings.max_players<=255)
            out << (quint8)i_insert.key();
        else
            out << (quint16)i_insert.key();
        out << i_insert.value()->x;
        out << i_insert.value()->y;
        out << (quint8)i_insert.value()->getLastDirection();

        ++i_insert;
    }
}
#endif

bool MapVisibilityAlgorithm_Simple::singleMove(const Direction &direction)
{
    if(!MoveOnTheMap::canGoTo(direction,*map,x,y,false))//check of colision disabled because do into LocalClientHandler
        return false;
    old_map=map;
    new_map=map;
    MoveOnTheMap::move(direction,&new_map,&x,&y);
    if(old_map!=new_map)
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        emit message(QString("singleMove() have change from old map: %1, to the new map: %2").arg(old_map->map_file).arg(new_map->map_file));
        #endif
        mapHaveChanged=true;
        map=old_map;
        unloadFromTheMap();
        map=static_cast<Map_server_MapVisibility_simple*>(new_map);
        loadOnTheMap();
    }
    return true;
}

void MapVisibilityAlgorithm_Simple::loadOnTheMap()
{
    insertClient();
    #ifdef CATCHCHALLENGER_SERVER_EXTRA_CHECK
    if(static_cast<Map_server_MapVisibility_simple*>(map)->clients.contains(this))
    {
        emit message("loadOnTheMap() try dual insert into the player list");
        return;
    }
    #endif
    static_cast<Map_server_MapVisibility_simple*>(map)->clients << this;
}

void MapVisibilityAlgorithm_Simple::unloadFromTheMap()
{
    #ifdef CATCHCHALLENGER_SERVER_EXTRA_CHECK
    if(!static_cast<Map_server_MapVisibility_simple*>(map)->clients.contains(this))
    {
        emit message("unloadFromTheMap() try remove of the player list, but not found");
        return;
    }
    #endif
    static_cast<Map_server_MapVisibility_simple*>(map)->clients.removeOne(this);
    mapVisiblity_unloadFromTheMap();
}

//map slots, transmited by the current ClientNetworkRead
void MapVisibilityAlgorithm_Simple::put_on_the_map(Map *map,const /*COORD_TYPE*/quint8 &x,const /*COORD_TYPE*/quint8 &y,const Orientation &orientation)
{
    MapBasicMove::put_on_the_map(map,x,y,orientation);
    loadOnTheMap();
}

bool MapVisibilityAlgorithm_Simple::moveThePlayer(const quint8 &previousMovedUnit,const Direction &direction)
{
    mapHaveChanged=false;
    //do on server part, because the client send when is blocked to sync the position
    #ifdef CATCHCHALLENGER_SERVER_MAP_DROP_BLOCKED_MOVE
    if(previousMovedUnitBlocked>0)
    {
        if(previousMovedUnit==0)
        {
            if(!MapBasicMove::moveThePlayer(previousMovedUnit,direction))
            {
                previousMovedUnitBlocked=0;
                return false;
            }
            //send the move to the other client
            moveClient(previousMovedUnitBlocked,direction);
            previousMovedUnitBlocked=0;
            return true;
        }
        else
        {
            emit error(QString("previousMovedUnitBlocked>0 but previousMovedUnit!=0"));
            return false;
        }
    }
    Direction temp_last_direction=last_direction;
    switch(last_direction)
    {
        case Direction_move_at_top:
            //move the player on the server map
            if(!MapBasicMove::moveThePlayer(previousMovedUnit,direction))
                return false;
            if(direction==Direction_look_at_top && !MoveOnTheMap::canGoTo(temp_last_direction,*map,x,y,true))
            {
                //blocked into the wall
                previousMovedUnitBlocked=previousMovedUnit;
                return true;
            }
        break;
        case Direction_move_at_right:
            //move the player on the server map
            if(!MapBasicMove::moveThePlayer(previousMovedUnit,direction))
                return false;
            if(direction==Direction_look_at_right && !MoveOnTheMap::canGoTo(temp_last_direction,*map,x,y,true))
            {
                //blocked into the wall
                previousMovedUnitBlocked=previousMovedUnit;
                return true;
            }
        break;
        case Direction_move_at_bottom:
            //move the player on the server map
            if(!MapBasicMove::moveThePlayer(previousMovedUnit,direction))
                return false;
            if(direction==Direction_look_at_bottom && !MoveOnTheMap::canGoTo(temp_last_direction,*map,x,y,true))
            {
                //blocked into the wall
                previousMovedUnitBlocked=previousMovedUnit;
                return true;
            }
        break;
        case Direction_move_at_left:
            //move the player on the server map
            if(!MapBasicMove::moveThePlayer(previousMovedUnit,direction))
                return false;
            if(direction==Direction_look_at_left && !MoveOnTheMap::canGoTo(temp_last_direction,*map,x,y,true))
            {
                //blocked into the wall
                previousMovedUnitBlocked=previousMovedUnit;
                return true;
            }
        break;
        case Direction_move_at_top:
        case Direction_move_at_right:
        case Direction_move_at_bottom:
        case Direction_move_at_left:
            //move the player on the server map
            if(!MapBasicMove::moveThePlayer(previousMovedUnit,direction))
                return false;
        break;
        default:
            emit error(QString("moveThePlayer(): direction not managed"));
            return false;
        break;
    }
    #else
    //move the player on the server map
    if(!MapBasicMove::moveThePlayer(previousMovedUnit,direction))
        return false;
    #endif
    //send the move to the other client
    moveClient(previousMovedUnit,direction);
    return true;
}

void MapVisibilityAlgorithm_Simple::teleportValidatedTo(Map *map,const COORD_TYPE &x,const COORD_TYPE &y,const Orientation &orientation)
{
    bool mapChange=(this->map!=map);
    if(mapChange)
        unloadFromTheMap();
    MapBasicMove::teleportValidatedTo(map,x,y,orientation);
    if(mapChange)
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        emit message(QString("have changed of map for teleportation, old map: %1, new map: %2").arg(this->map->map_file).arg(map->map_file));
        #endif
        this->map=static_cast<Map_server_MapVisibility_simple*>(map);
        loadOnTheMap();
    }
}

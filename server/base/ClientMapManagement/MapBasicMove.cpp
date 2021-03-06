#include "MapBasicMove.h"
#include "../MapServer.h"
#include "../../../general/base/GeneralVariable.h"
#include "../../../general/base/MoveOnTheMap.h"

using namespace CatchChallenger;

/** \todo do client near list for the local player
  the list is limited to 50
  if is greater, then truncate to have the near player, truncate to have all near player grouped by distance where a group not do the list greater
  each Xs update the local player list
*/
/** Never reserve the list, because it have square memory usage, and use more cpu */

MapBasicMove::MapBasicMove()
{
    map=NULL;
    player_informations=NULL;
}

MapBasicMove::~MapBasicMove()
{
}

void MapBasicMove::setVariable(Player_internal_informations *player_informations)
{
    this->player_informations=player_informations;
}

void MapBasicMove::askIfIsReadyToStop()
{
    if(map==NULL)
    {
        emit isReadyToStop();
        return;
    }
    extraStop();

    map=NULL;
    emit isReadyToStop();
}

Direction MapBasicMove::getLastDirection()
{
    return last_direction;
}

Map* MapBasicMove::getMap()
{
    return map;
}

COORD_TYPE MapBasicMove::getX()
{
    return x;
}

COORD_TYPE MapBasicMove::getY()
{
    return y;
}

void MapBasicMove::extraStop()
{
}

void MapBasicMove::put_on_the_map(Map *map,const /*COORD_TYPE*/quint8 &x,const /*COORD_TYPE*/quint8 &y,const Orientation &orientation)
{
    //store the starting informations
    last_direction=static_cast<Direction>(orientation);

    //store the current information about player on the map
    this->x=x;
    this->y=y;
    this->map=map;

    #ifdef CATCHCHALLENGER_SERVER_EXTRA_CHECK
    if(this->x>(map->width-1))
    {
        emit message(QString("put_on_the_map(): Wrong x: %1").arg(x));
        this->x=map->width-1;
    }
    if(this->y>(map->height-1))
    {
        emit message(QString("put_on_the_map(): Wrong y: %1").arg(y));
        this->y=map->height-1;
    }
    #endif
}

void MapBasicMove::teleportValidatedTo(Map *map,const /*COORD_TYPE*/quint8 &x,const /*COORD_TYPE*/quint8 &y,const Orientation &orientation)
{
    MapBasicMove::put_on_the_map(map,x,y,orientation);
}

bool MapBasicMove::moveThePlayer(const quint8 &previousMovedUnit,const Direction &direction)
{
    /** \warning Don't put emit here, because call by LocalClientHandler, visiblity algo, LocalBroadcast */

    quint8 moveThePlayer_index_move=0;
    if(unlikely(last_direction==direction))
    {
        emit error(QString("Previous action is same direction: %1").arg(last_direction));
        return false;
    }
    switch(last_direction)
    {
        case Direction_move_at_top:
        {
            CatchChallenger::ParsedLayerLedges ledge;
            /* can be moving by grouping
            if(unlikely(previousMovedUnit==0 || previousMovedUnit==255))
            {
                emit error(QString("Direction_move_at_top: Previous action is moving: %1").arg(last_direction));
                return false;
            }*/
            while(moveThePlayer_index_move<previousMovedUnit)
            {
                do
                {
                    if(!singleMove(Direction_move_at_top))
                        return false;
                    ledge=MoveOnTheMap::getLedge(*map,x,y);
                } while(ledge==ParsedLayerLedges_LedgesTop);
                if(ledge!=ParsedLayerLedges_NoLedges)
                {
                    emit error(QString("Try pass on wrong ledge, direction: %1, ledge: %2").arg(last_direction).arg(ledge));
                    return false;
                }
                moveThePlayer_index_move++;
            }
        }
        break;
        case Direction_look_at_top:
            /* can be look into other direction
            if(unlikely(previousMovedUnit>0))
            {
                emit error(QString("Direction_look_at_top: Previous action is not moving: %1").arg(last_direction));
                return false;
            }*/
        break;
        case Direction_move_at_right:
        {
            CatchChallenger::ParsedLayerLedges ledge;
            /* can be moving by grouping
            if(unlikely(previousMovedUnit==0 || previousMovedUnit==255))
            {
                emit error(QString("Direction_move_at_right: Previous action is moving: %1").arg(last_direction));
                return false;
            }*/
            while(moveThePlayer_index_move<previousMovedUnit)
            {
                do
                {
                    if(!singleMove(Direction_move_at_right))
                        return false;
                    ledge=MoveOnTheMap::getLedge(*map,x,y);
                } while(ledge==ParsedLayerLedges_LedgesRight);
                if(ledge!=ParsedLayerLedges_NoLedges)
                {
                    emit error(QString("Try pass on wrong ledge, direction: %1, ledge: %2").arg(last_direction).arg(ledge));
                    return false;
                }
                moveThePlayer_index_move++;
            }
        }
        break;
        case Direction_look_at_right:
            /* can be look into other direction
            if(unlikely(previousMovedUnit>0))
            {
                emit error(QString("Direction_look_at_right: Previous action is not moving: %1").arg(last_direction));
                return false;
            }*/
        break;
        case Direction_move_at_bottom:
        {
            CatchChallenger::ParsedLayerLedges ledge;
            /* can be moving by grouping
            if(unlikely(previousMovedUnit==0 || previousMovedUnit==255))
            {
                emit error(QString("Direction_move_at_bottom: Previous action is moving: %1").arg(last_direction));
                return false;
            }*/
            while(moveThePlayer_index_move<previousMovedUnit)
            {
                do
                {
                    if(!singleMove(Direction_move_at_bottom))
                        return false;
                    ledge=MoveOnTheMap::getLedge(*map,x,y);
                } while(ledge==ParsedLayerLedges_LedgesBottom);
                if(ledge!=ParsedLayerLedges_NoLedges)
                {
                    emit error(QString("Try pass on wrong ledge, direction: %1, ledge: %2").arg(last_direction).arg(ledge));
                    return false;
                }
                moveThePlayer_index_move++;
            }
        }
        break;
        case Direction_look_at_bottom:
            /* can be look into other direction
            if(unlikely(previousMovedUnit>0))
            {
                emit error(QString("Direction_look_at_bottom: Previous action is not moving: %1").arg(last_direction));
                return false;
            }*/
        break;
        case Direction_move_at_left:
        {
            CatchChallenger::ParsedLayerLedges ledge;
            /* can be moving by grouping
            if(unlikely(previousMovedUnit==0 || previousMovedUnit==255))
            {
                emit error(QString("Direction_move_at_left: Previous action is moving: %1").arg(last_direction));
                return false;
            }*/
            while(moveThePlayer_index_move<previousMovedUnit)
            {
                do
                {
                    if(!singleMove(Direction_move_at_left))
                        return false;
                    ledge=MoveOnTheMap::getLedge(*map,x,y);
                } while(ledge==ParsedLayerLedges_LedgesLeft);
                if(ledge!=ParsedLayerLedges_NoLedges)
                {
                    emit error(QString("Try pass on wrong ledge, direction: %1, ledge: %2").arg(last_direction).arg(ledge));
                    return false;
                }
                moveThePlayer_index_move++;
            }
        }
        break;
        case Direction_look_at_left:
            /* can be look into other direction
            if(unlikely(previousMovedUnit>0))
            {
                emit error(QString("Direction_look_at_left: Previous action is not moving: %1").arg(last_direction));
                return false;
            }*/
        break;
        default:
            emit error(QString("moveThePlayer(): direction not managed"));
            return false;
        break;
    }
    last_direction=direction;
    return true;
}

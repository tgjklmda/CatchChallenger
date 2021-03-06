#include "MapVisualiserPlayer.h"

#include "../../../general/base/MoveOnTheMap.h"
#include "../interface/DatapackClientLoader.h"

#include <qmath.h>

/* why send the look at because blocked into the wall?
to be sync if connexion is stop, but use more bandwith
To not send: store "is blocked but direction not send", cautch the close event, at close: if "is blocked but direction not send" then send it
*/

MapVisualiserPlayer::MapVisualiserPlayer(const bool &centerOnPlayer,const bool &debugTags,const bool &useCache,const bool &OpenGL) :
    MapVisualiser(debugTags,useCache,OpenGL)
{
    inMove=false;
    x=0;
    y=0;

    keyAccepted << Qt::Key_Left << Qt::Key_Right << Qt::Key_Up << Qt::Key_Down << Qt::Key_Return;

    lookToMove.setInterval(200);
    lookToMove.setSingleShot(true);
    connect(&lookToMove,SIGNAL(timeout()),this,SLOT(transformLookToMove()));

    moveTimer.setInterval(250/5);
    moveTimer.setSingleShot(true);
    connect(&moveTimer,SIGNAL(timeout()),this,SLOT(moveStepSlot()));

    this->centerOnPlayer=centerOnPlayer;

    if(centerOnPlayer)
    {
        setSceneRect(-2000,-2000,4000,4000);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    stepAlternance=false;
    animationTileset=new Tiled::Tileset("animation",16,16);
    nextCurrentObject=new Tiled::MapObject();
    grassCurrentObject=new Tiled::MapObject();
    haveGrassCurrentObject=false;
    haveNextCurrentObject=false;

    playerMapObject = new Tiled::MapObject();
    playerTileset = new Tiled::Tileset("player",16,24);

    current_map=NULL;
}

MapVisualiserPlayer::~MapVisualiserPlayer()
{
    delete animationTileset;
    delete nextCurrentObject;
    delete grassCurrentObject;
    delete playerMapObject;
    delete playerTileset;
}

void MapVisualiserPlayer::keyPressEvent(QKeyEvent * event)
{
    if(current_map==NULL)
        return;

    //ignore the no arrow key
    if(!keyAccepted.contains(event->key()))
    {
        event->ignore();
        return;
    }

    //ignore the repeated event
    if(event->isAutoRepeat())
        return;

    //add to pressed key list
    keyPressed << event->key();

    //apply the key
    keyPressParse();
}

void MapVisualiserPlayer::keyPressParse()
{
    //ignore is already in move
    if(inMove)
        return;

    if(keyPressed.size()==1 && keyPressed.contains(Qt::Key_Return))
    {
        keyPressed.remove(Qt::Key_Return);
        parseAction();
        return;
    }

    if(keyPressed.contains(Qt::Key_Left))
    {
        //already turned on this direction, then try move into this direction
        if(direction==CatchChallenger::Direction_look_at_left)
        {
            if(!canGoTo(CatchChallenger::Direction_move_at_left,current_map->logicalMap,x,y,true))
                return;//Can't do at the left!
            //the first step
            direction=CatchChallenger::Direction_move_at_left;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        //look in this direction
        else
        {
            playerMapObject->setTile(playerTileset->tileAt(10));
            direction=CatchChallenger::Direction_look_at_left;
            lookToMove.start();
            emit send_player_direction(direction);
            parseStop();
        }
    }
    else if(keyPressed.contains(Qt::Key_Right))
    {
        //already turned on this direction, then try move into this direction
        if(direction==CatchChallenger::Direction_look_at_right)
        {
            if(!canGoTo(CatchChallenger::Direction_move_at_right,current_map->logicalMap,x,y,true))
                return;//Can't do at the right!
            //the first step
            direction=CatchChallenger::Direction_move_at_right;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        //look in this direction
        else
        {
            playerMapObject->setTile(playerTileset->tileAt(4));
            direction=CatchChallenger::Direction_look_at_right;
            lookToMove.start();
            emit send_player_direction(direction);
            parseStop();
        }
    }
    else if(keyPressed.contains(Qt::Key_Up))
    {
        //already turned on this direction, then try move into this direction
        if(direction==CatchChallenger::Direction_look_at_top)
        {
            if(!canGoTo(CatchChallenger::Direction_move_at_top,current_map->logicalMap,x,y,true))
                return;//Can't do at the top!
            //the first step
            direction=CatchChallenger::Direction_move_at_top;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        //look in this direction
        else
        {
            playerMapObject->setTile(playerTileset->tileAt(1));
            direction=CatchChallenger::Direction_look_at_top;
            lookToMove.start();
            emit send_player_direction(direction);
            parseStop();
        }
    }
    else if(keyPressed.contains(Qt::Key_Down))
    {
        //already turned on this direction, then try move into this direction
        if(direction==CatchChallenger::Direction_look_at_bottom)
        {
            if(!canGoTo(CatchChallenger::Direction_move_at_bottom,current_map->logicalMap,x,y,true))
                return;//Can't do at the bottom!
            //the first step
            direction=CatchChallenger::Direction_move_at_bottom;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        //look in this direction
        else
        {
            playerMapObject->setTile(playerTileset->tileAt(7));
            direction=CatchChallenger::Direction_look_at_bottom;
            lookToMove.start();
            emit send_player_direction(direction);
            parseStop();
        }
    }
}

void MapVisualiserPlayer::moveStepSlot()
{
    int baseTile=1;
    //move the player for intermediate step and define the base tile (define the stopped step with direction)
    switch(direction)
    {
        case CatchChallenger::Direction_move_at_left:
        baseTile=10;
        switch(moveStep)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            playerMapObject->setX(playerMapObject->x()-0.20);
            break;
        }
        break;
        case CatchChallenger::Direction_move_at_right:
        baseTile=4;
        switch(moveStep)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            playerMapObject->setX(playerMapObject->x()+0.20);
            break;
        }
        break;
        case CatchChallenger::Direction_move_at_top:
        baseTile=1;
        switch(moveStep)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            playerMapObject->setY(playerMapObject->y()-0.20);
            break;
        }
        break;
        case CatchChallenger::Direction_move_at_bottom:
        baseTile=7;
        switch(moveStep)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            playerMapObject->setY(playerMapObject->y()+0.20);
            break;
        }
        break;
        default:
        qDebug() << QString("moveStepSlot(): moveStep: %1, wrong direction").arg(moveStep);
        return;
    }

    //apply the right step of the base step defined previously by the direction
    switch(moveStep%5)
    {
        //stopped step
        case 0:
        playerMapObject->setTile(playerTileset->tileAt(baseTile+0));
        break;
        case 1:
        MapObjectItem::objectLink[playerMapObject]->setZValue(qCeil(playerMapObject->y()));
        break;
        //transition step
        case 2:
        if(stepAlternance)
            playerMapObject->setTile(playerTileset->tileAt(baseTile-1));
        else
            playerMapObject->setTile(playerTileset->tileAt(baseTile+1));
        stepAlternance=!stepAlternance;
        break;
        //stopped step
        case 4:
        playerMapObject->setTile(playerTileset->tileAt(baseTile+0));
        break;
    }

    if(centerOnPlayer)
        centerOn(MapObjectItem::objectLink[playerMapObject]);
    loadGrassTile();

    moveStep++;

    //if have finish the step
    if(moveStep>5)
    {
        CatchChallenger::Map * old_map=&current_map->logicalMap;
        CatchChallenger::Map * map=&current_map->logicalMap;
        //set the final value (direction, position, ...)
        switch(direction)
        {
            case CatchChallenger::Direction_move_at_left:
                CatchChallenger::MoveOnTheMap::move(direction,&map,&x,&y);
                direction=CatchChallenger::Direction_look_at_left;
            break;
            case CatchChallenger::Direction_move_at_right:
                CatchChallenger::MoveOnTheMap::move(direction,&map,&x,&y);
                direction=CatchChallenger::Direction_look_at_right;
            break;
            case CatchChallenger::Direction_move_at_top:
                CatchChallenger::MoveOnTheMap::move(direction,&map,&x,&y);
                direction=CatchChallenger::Direction_look_at_top;
            break;
            case CatchChallenger::Direction_move_at_bottom:
                CatchChallenger::MoveOnTheMap::move(direction,&map,&x,&y);
                direction=CatchChallenger::Direction_look_at_bottom;
            break;
            default:
                qDebug() << QString("moveStepSlot(): moveStep: %1, wrong direction (%2) when moveStep>2").arg(moveStep).arg(direction);
            return;
        }
        //if the map have changed
        if(old_map!=map)
        {
            loadOtherMap(map->map_file);
            if(!all_map.contains(map->map_file))
                qDebug() << QString("map changed not located: %1").arg(map->map_file);
            else
            {
                unloadPlayerFromCurrentMap();
                all_map[current_map->logicalMap.map_file]=current_map;
                current_map=all_map[map->map_file];
                mapUsed=loadMap(current_map,true);
                removeUnusedMap();
                loadPlayerFromCurrentMap();
            }
        }
        //move to the final position (integer), y+1 because the tile lib start y to 1, not 0
        playerMapObject->setPosition(QPoint(x,y+1));
        MapObjectItem::objectLink[playerMapObject]->setZValue(y);
        if(centerOnPlayer)
        {
            //playerMapObject->set
            centerOn(MapObjectItem::objectLink[playerMapObject]);
        }
        stopGrassAnimation();

        if(haveStopTileAction())
            return;

        if(CatchChallenger::MoveOnTheMap::getLedge(*map,x,y)!=CatchChallenger::ParsedLayerLedges_NoLedges)
        {
            switch(direction)
            {
                case CatchChallenger::Direction_look_at_left:
                    direction=CatchChallenger::Direction_move_at_left;
                break;
                case CatchChallenger::Direction_look_at_right:
                    direction=CatchChallenger::Direction_move_at_right;
                break;
                case CatchChallenger::Direction_look_at_top:
                    direction=CatchChallenger::Direction_move_at_top;
                break;
                case CatchChallenger::Direction_look_at_bottom:
                    direction=CatchChallenger::Direction_move_at_bottom;
                break;
                default:
                    qDebug() << QString("moveStepSlot(): direction: %1, wrong direction").arg(direction);
                return;
            }
            moveStep=0;
            moveTimer.start();
            startGrassAnimation(direction);
            return;
        }

        //check if one arrow key is pressed to continue to move into this direction
        if(keyPressed.contains(Qt::Key_Left))
        {
            //can't go into this direction, then just look into this direction
            if(!canGoTo(CatchChallenger::Direction_move_at_left,current_map->logicalMap,x,y,true))
            {
                keyPressed.remove(Qt::Key_Left);
                direction=CatchChallenger::Direction_look_at_left;
                playerMapObject->setTile(playerTileset->tileAt(10));
                inMove=false;
                emit send_player_direction(direction);//see the top note
                parseStop();
            }
            //if can go, then do the move
            else
            {
                direction=CatchChallenger::Direction_move_at_left;
                moveStep=0;
                moveStepSlot();
                emit send_player_direction(direction);
                startGrassAnimation(direction);
            }
        }
        else if(keyPressed.contains(Qt::Key_Right))
        {
            //can't go into this direction, then just look into this direction
            if(!canGoTo(CatchChallenger::Direction_move_at_right,current_map->logicalMap,x,y,true))
            {
                keyPressed.remove(Qt::Key_Right);
                direction=CatchChallenger::Direction_look_at_right;
                playerMapObject->setTile(playerTileset->tileAt(4));
                inMove=false;
                emit send_player_direction(direction);//see the top note
                parseStop();
            }
            //if can go, then do the move
            else
            {
                direction=CatchChallenger::Direction_move_at_right;
                moveStep=0;
                moveStepSlot();
                emit send_player_direction(direction);
                startGrassAnimation(direction);
            }
        }
        else if(keyPressed.contains(Qt::Key_Up))
        {
            //can't go into this direction, then just look into this direction
            if(!canGoTo(CatchChallenger::Direction_move_at_top,current_map->logicalMap,x,y,true))
            {
                keyPressed.remove(Qt::Key_Up);
                direction=CatchChallenger::Direction_look_at_top;
                playerMapObject->setTile(playerTileset->tileAt(1));
                inMove=false;
                emit send_player_direction(direction);//see the top note
                parseStop();
            }
            //if can go, then do the move
            else
            {
                direction=CatchChallenger::Direction_move_at_top;
                moveStep=0;
                moveStepSlot();
                emit send_player_direction(direction);
                startGrassAnimation(direction);
            }
        }
        else if(keyPressed.contains(Qt::Key_Down))
        {
            //can't go into this direction, then just look into this direction
            if(!canGoTo(CatchChallenger::Direction_move_at_bottom,current_map->logicalMap,x,y,true))
            {
                keyPressed.remove(Qt::Key_Down);
                direction=CatchChallenger::Direction_look_at_bottom;
                playerMapObject->setTile(playerTileset->tileAt(7));
                inMove=false;
                emit send_player_direction(direction);//see the top note
                parseStop();
            }
            //if can go, then do the move
            else
            {
                direction=CatchChallenger::Direction_move_at_bottom;
                moveStep=0;
                moveStepSlot();
                emit send_player_direction(direction);
                startGrassAnimation(direction);
            }
        }
        //now stop walking, no more arrow key is pressed
        else
        {
            inMove=false;
            emit send_player_direction(direction);
            parseStop();
        }
    }
    else
        moveTimer.start();
}

bool MapVisualiserPlayer::haveStopTileAction()
{
    return false;
}

void MapVisualiserPlayer::parseStop()
{
    CatchChallenger::Map * map=&current_map->logicalMap;
    quint8 x=this->x;
    quint8 y=this->y;
    switch(direction)
    {
        case CatchChallenger::Direction_look_at_left:
        if(CatchChallenger::MoveOnTheMap::canGoTo(CatchChallenger::Direction_move_at_left,*map,x,y,false))
        {
            if(!CatchChallenger::MoveOnTheMap::move(CatchChallenger::Direction_move_at_left,&map,&x,&y,false))
                qDebug() << QString("can't go at left at map %1 (%2,%3) when move have been checked").arg(map->map_file).arg(x).arg(y);
            else
                emit stopped_in_front_of(static_cast<CatchChallenger::Map_client *>(map),x,y);
        }
        break;
        case CatchChallenger::Direction_look_at_right:
        if(CatchChallenger::MoveOnTheMap::canGoTo(CatchChallenger::Direction_move_at_right,*map,x,y,false))
        {
            if(!CatchChallenger::MoveOnTheMap::move(CatchChallenger::Direction_move_at_right,&map,&x,&y,false))
                qDebug() << QString("can't go at right at map %1 (%2,%3) when move have been checked").arg(map->map_file).arg(x).arg(y);
            else
                emit stopped_in_front_of(static_cast<CatchChallenger::Map_client *>(map),x,y);
        }
        break;
        case CatchChallenger::Direction_look_at_top:
        if(CatchChallenger::MoveOnTheMap::canGoTo(CatchChallenger::Direction_move_at_top,*map,x,y,false))
        {
            if(!CatchChallenger::MoveOnTheMap::move(CatchChallenger::Direction_move_at_top,&map,&x,&y,false))
                qDebug() << QString("can't go at top at map %1 (%2,%3) when move have been checked").arg(map->map_file).arg(x).arg(y);
            else
                emit stopped_in_front_of(static_cast<CatchChallenger::Map_client *>(map),x,y);
        }
        break;
        case CatchChallenger::Direction_look_at_bottom:
        if(CatchChallenger::MoveOnTheMap::canGoTo(CatchChallenger::Direction_move_at_bottom,*map,x,y,false))
        {
            if(!CatchChallenger::MoveOnTheMap::move(CatchChallenger::Direction_move_at_bottom,&map,&x,&y,false))
                qDebug() << QString("can't go at bottom at map %1 (%2,%3) when move have been checked").arg(map->map_file).arg(x).arg(y);
            else
                emit stopped_in_front_of(static_cast<CatchChallenger::Map_client *>(map),x,y);
        }
        break;
        default:
        break;
    }
}

void MapVisualiserPlayer::parseAction()
{
    CatchChallenger::Map * map=&current_map->logicalMap;
    quint8 x=this->x;
    quint8 y=this->y;
    switch(direction)
    {
        case CatchChallenger::Direction_look_at_left:
        if(CatchChallenger::MoveOnTheMap::canGoTo(CatchChallenger::Direction_move_at_left,*map,x,y,false))
        {
            if(!CatchChallenger::MoveOnTheMap::move(CatchChallenger::Direction_move_at_left,&map,&x,&y,false))
                qDebug() << QString("can't go at left at map %1 (%2,%3) when move have been checked").arg(map->map_file).arg(x).arg(y);
            else
                emit actionOn(static_cast<CatchChallenger::Map_client *>(map),x,y);
        }
        break;
        case CatchChallenger::Direction_look_at_right:
        if(CatchChallenger::MoveOnTheMap::canGoTo(CatchChallenger::Direction_move_at_right,*map,x,y,false))
        {
            if(!CatchChallenger::MoveOnTheMap::move(CatchChallenger::Direction_move_at_right,&map,&x,&y,false))
                qDebug() << QString("can't go at right at map %1 (%2,%3) when move have been checked").arg(map->map_file).arg(x).arg(y);
            else
                emit actionOn(static_cast<CatchChallenger::Map_client *>(map),x,y);
        }
        break;
        case CatchChallenger::Direction_look_at_top:
        if(CatchChallenger::MoveOnTheMap::canGoTo(CatchChallenger::Direction_move_at_top,*map,x,y,false))
        {
            if(!CatchChallenger::MoveOnTheMap::move(CatchChallenger::Direction_move_at_top,&map,&x,&y,false))
                qDebug() << QString("can't go at bottom at map %1 (%2,%3) when move have been checked").arg(map->map_file).arg(x).arg(y);
            else
                emit actionOn(static_cast<CatchChallenger::Map_client *>(map),x,y);
        }
        break;
        case CatchChallenger::Direction_look_at_bottom:
        if(CatchChallenger::MoveOnTheMap::canGoTo(CatchChallenger::Direction_move_at_bottom,*map,x,y,false))
        {
            if(!CatchChallenger::MoveOnTheMap::move(CatchChallenger::Direction_move_at_bottom,&map,&x,&y,false))
                qDebug() << QString("can't go at top at map %1 (%2,%3) when move have been checked").arg(map->map_file).arg(x).arg(y);
            else
                emit actionOn(static_cast<CatchChallenger::Map_client *>(map),x,y);
        }
        break;
        default:
        break;
    }
}

//have look into another direction, if the key remain pressed, apply like move
void MapVisualiserPlayer::transformLookToMove()
{
    if(inMove)
        return;

    switch(direction)
    {
        case CatchChallenger::Direction_look_at_left:
        if(keyPressed.contains(Qt::Key_Left) && canGoTo(CatchChallenger::Direction_move_at_left,current_map->logicalMap,x,y,true))
        {
            direction=CatchChallenger::Direction_move_at_left;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        break;
        case CatchChallenger::Direction_look_at_right:
        if(keyPressed.contains(Qt::Key_Right) && canGoTo(CatchChallenger::Direction_move_at_right,current_map->logicalMap,x,y,true))
        {
            direction=CatchChallenger::Direction_move_at_right;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        break;
        case CatchChallenger::Direction_look_at_top:
        if(keyPressed.contains(Qt::Key_Up) && canGoTo(CatchChallenger::Direction_move_at_top,current_map->logicalMap,x,y,true))
        {
            direction=CatchChallenger::Direction_move_at_top;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        break;
        case CatchChallenger::Direction_look_at_bottom:
        if(keyPressed.contains(Qt::Key_Down) && canGoTo(CatchChallenger::Direction_move_at_bottom,current_map->logicalMap,x,y,true))
        {
            direction=CatchChallenger::Direction_move_at_bottom;
            inMove=true;
            moveStep=1;
            moveStepSlot();
            emit send_player_direction(direction);
            startGrassAnimation(direction);
        }
        break;
        default:
        qDebug() << QString("transformLookToMove(): wrong direction");
        return;
    }
}

void MapVisualiserPlayer::keyReleaseEvent(QKeyEvent * event)
{
    if(current_map==NULL)
        return;

    //ignore the no arrow key
    if(!keyAccepted.contains(event->key()))
    {
        event->ignore();
        return;
    }

    //ignore the repeated event
    if(event->isAutoRepeat())
        return;

    //remove from the key list pressed
    keyPressed.remove(event->key());

    if(keyPressed.size()>0)//another key pressed, repeat
        keyPressParse();
}

QString MapVisualiserPlayer::lastLocation() const
{
    return mLastLocation;
}

CatchChallenger::Direction MapVisualiserPlayer::getDirection()
{
    return direction;
}

void MapVisualiserPlayer::setAnimationTilset(QString animationTilset)
{
    animationTileset->loadFromImage(QImage(":/images/player_default/animation.png"),":/images/player_default/animation.png");
    if(QFile::exists(animationTilset))
        if(!animationTileset->loadFromImage(QImage(animationTilset),animationTilset))
            qDebug() << "Unable the load the datapack animation tileset: " << animationTilset;
}

void MapVisualiserPlayer::resetAll()
{
    stopGrassAnimation();
    unloadPlayerFromCurrentMap();
    timer.stop();
    moveTimer.stop();
    lookToMove.stop();
    keyPressed.clear();
    inMove=false;
    MapVisualiser::resetAll();
}

void MapVisualiserPlayer::setSpeed(const SPEED_TYPE &speed)
{
    moveTimer.setInterval(speed/5);
}

bool MapVisualiserPlayer::canGoTo(const CatchChallenger::Direction &direction, CatchChallenger::Map map, quint8 x, quint8 y, const bool &checkCollision)
{
    CatchChallenger::Map *mapPointer=&map;
    CatchChallenger::ParsedLayerLedges ledge;
    do
    {
        if(!CatchChallenger::MoveOnTheMap::canGoTo(direction,*mapPointer,x,y,checkCollision))
            return false;
        if(!CatchChallenger::MoveOnTheMap::move(direction,&mapPointer,&x,&y,checkCollision))
            return false;
        ledge=CatchChallenger::MoveOnTheMap::getLedge(map,x,y);
        if(ledge==CatchChallenger::ParsedLayerLedges_NoLedges)
            return true;
        switch(direction)
        {
            case CatchChallenger::Direction_move_at_bottom:
            if(ledge!=CatchChallenger::ParsedLayerLedges_LedgesBottom)
                return false;
            break;
            case CatchChallenger::Direction_move_at_top:
            if(ledge!=CatchChallenger::ParsedLayerLedges_LedgesTop)
                return false;
            break;
            case CatchChallenger::Direction_move_at_left:
            if(ledge!=CatchChallenger::ParsedLayerLedges_LedgesLeft)
                return false;
            break;
            case CatchChallenger::Direction_move_at_right:
            if(ledge!=CatchChallenger::ParsedLayerLedges_LedgesRight)
                return false;
            break;
            default:
            break;
        }
    } while(ledge!=CatchChallenger::ParsedLayerLedges_NoLedges);
    return true;
}

//call after enter on new map
void MapVisualiserPlayer::loadPlayerFromCurrentMap()
{
    Tiled::ObjectGroup *currentGroup=playerMapObject->objectGroup();
    if(currentGroup!=NULL)
    {
        if(ObjectGroupItem::objectGroupLink.contains(currentGroup))
            ObjectGroupItem::objectGroupLink[currentGroup]->removeObject(playerMapObject);
        //currentGroup->removeObject(playerMapObject);
        if(currentGroup!=current_map->objectGroup)
            qDebug() << QString("loadPlayerFromCurrentMap(), the playerMapObject group is wrong: %1").arg(currentGroup->name());
    }
    if(ObjectGroupItem::objectGroupLink.contains(current_map->objectGroup))
        ObjectGroupItem::objectGroupLink[current_map->objectGroup]->addObject(playerMapObject);
    else
        qDebug() << QString("loadPlayerFromCurrentMap(), ObjectGroupItem::objectGroupLink not contains current_map->objectGroup");
    mLastLocation=current_map->logicalMap.map_file;

    //move to the final position (integer), y+1 because the tile lib start y to 1, not 0
    playerMapObject->setPosition(QPoint(x,y+1));
    MapObjectItem::objectLink[playerMapObject]->setZValue(y);
    if(centerOnPlayer)
        centerOn(MapObjectItem::objectLink[playerMapObject]);
}

//call before leave the old map (and before loadPlayerFromCurrentMap())
void MapVisualiserPlayer::unloadPlayerFromCurrentMap()
{
    Tiled::ObjectGroup *currentGroup=playerMapObject->objectGroup();
    if(currentGroup==NULL)
        return;
    //unload the player sprite
    if(ObjectGroupItem::objectGroupLink.contains(playerMapObject->objectGroup()))
        ObjectGroupItem::objectGroupLink[playerMapObject->objectGroup()]->removeObject(playerMapObject);
    else
        qDebug() << QString("unloadPlayerFromCurrentMap(), ObjectGroupItem::objectGroupLink not contains playerMapObject->objectGroup()");
}

void MapVisualiserPlayer::startGrassAnimation(const CatchChallenger::Direction &direction)
{
    switch(direction)
    {
        case CatchChallenger::Direction_move_at_left:
        case CatchChallenger::Direction_move_at_right:
        case CatchChallenger::Direction_move_at_top:
        case CatchChallenger::Direction_move_at_bottom:
        break;
        default:
        return;
    }

    if(!haveGrassCurrentObject)
    {
        haveGrassCurrentObject=CatchChallenger::MoveOnTheMap::haveGrass(current_map->logicalMap,x,y);
        if(haveGrassCurrentObject)
        {
            ObjectGroupItem::objectGroupLink[current_map->objectGroup]->addObject(grassCurrentObject);
            grassCurrentObject->setPosition(QPoint(x,y+1));
            MapObjectItem::objectLink[playerMapObject]->setZValue(y);
            grassCurrentObject->setTile(animationTileset->tileAt(2));
        }
    }
    else
        qDebug() << "haveGrassCurrentObject true here, it's wrong!";

    if(!haveNextCurrentObject)
    {
        haveNextCurrentObject=false;
        CatchChallenger::Map * map_destination=&current_map->logicalMap;
        COORD_TYPE x_destination=x;
        COORD_TYPE y_destination=y;
        if(CatchChallenger::MoveOnTheMap::move(direction,&map_destination,&x_destination,&y_destination))
            if(all_map.contains(map_destination->map_file))
                haveNextCurrentObject=CatchChallenger::MoveOnTheMap::haveGrass(*map_destination,x_destination,y_destination);
        if(haveNextCurrentObject)
        {
            ObjectGroupItem::objectGroupLink[all_map[map_destination->map_file]->objectGroup]->addObject(nextCurrentObject);
            nextCurrentObject->setPosition(QPoint(x_destination,y_destination+1));
            MapObjectItem::objectLink[playerMapObject]->setZValue(y_destination);
            nextCurrentObject->setTile(animationTileset->tileAt(1));
        }
    }
    else
        qDebug() << "haveNextCurrentObject true here, it's wrong!";
}

void MapVisualiserPlayer::stopGrassAnimation()
{
    if(haveGrassCurrentObject)
    {
        ObjectGroupItem::objectGroupLink[grassCurrentObject->objectGroup()]->removeObject(grassCurrentObject);
        haveGrassCurrentObject=false;
    }
    if(haveNextCurrentObject)
    {
        ObjectGroupItem::objectGroupLink[nextCurrentObject->objectGroup()]->removeObject(nextCurrentObject);
        haveNextCurrentObject=false;
    }
}

void MapVisualiserPlayer::loadGrassTile()
{
    if(haveGrassCurrentObject)
    {
        switch(moveStep)
        {
            case 0:
            case 1:
            break;
            case 2:
                grassCurrentObject->setTile(animationTileset->tileAt(0));
            break;
        }
    }
    if(haveNextCurrentObject)
    {
        switch(moveStep)
        {
            case 0:
            case 1:
            break;
            case 3:
                nextCurrentObject->setTile(animationTileset->tileAt(2));
            break;
        }
    }
}

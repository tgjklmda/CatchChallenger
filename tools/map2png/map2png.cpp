/*
 * tmxviewer.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of the TMX Viewer example.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "map2png.h"

#include <QCoreApplication>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <QTime>
#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

#include "../../general/base/MoveOnTheMap.h"

using namespace Tiled;

/**
 * Item that represents a map object.
 */
class MapObjectItem : public QGraphicsItem
{
public:
    MapObjectItem(MapObject *mapObject, MapRenderer *renderer,
                  QGraphicsItem *parent = 0)
        : QGraphicsItem(parent)
        , mMapObject(mapObject)
        , mRenderer(renderer)
    {}

    QRectF boundingRect() const
    {
        return mRenderer->boundingRect(mMapObject);
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
    {
        const QColor &color = mMapObject->objectGroup()->color();
        mRenderer->drawMapObject(p, mMapObject,
                                 color.isValid() ? color : Qt::darkGray);
    }

private:
    MapObject *mMapObject;
    MapRenderer *mRenderer;
};

/**
 * Item that represents a tile layer.
 */
class TileLayerItem : public QGraphicsItem
{
public:
    TileLayerItem(TileLayer *tileLayer, MapRenderer *renderer,
                  QGraphicsItem *parent = 0)
        : QGraphicsItem(parent)
        , mTileLayer(tileLayer)
        , mRenderer(renderer)
    {
        setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
    }

    QRectF boundingRect() const
    {
        return mRenderer->boundingRect(mTileLayer->bounds());
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *)
    {
        mRenderer->drawTileLayer(p, mTileLayer, option->rect);
    }

private:
    TileLayer *mTileLayer;
    MapRenderer *mRenderer;
};

/**
 * Item that represents an object group.
 */
class ObjectGroupItem : public QGraphicsItem
{
public:
    ObjectGroupItem(ObjectGroup *objectGroup, MapRenderer *renderer,
                    QGraphicsItem *parent = 0)
        : QGraphicsItem(parent)
    {
        setFlag(QGraphicsItem::ItemHasNoContents);

        // Create a child item for each object
        foreach (MapObject *object, objectGroup->objects())
            new MapObjectItem(object, renderer, this);
    }

    QRectF boundingRect() const { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}
};

/**
 * Item that represents a map.
 */
MapItem::MapItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setFlag(QGraphicsItem::ItemHasNoContents);
}

void MapItem::addMap(Map *map, MapRenderer *renderer)
{
    //align zIndex to Collision Layer
    int index=0;
    while(index<map->layers().size())
    {
        if(map->layers().at(index)->name()=="Collisions")
            break;
        index++;
    }
    if(index==map->layers().size())
        index=-map->layers().size()-1;
    else
        index=-index;

    // Create a child item for each layer
    foreach (Layer *layer, map->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            TileLayerItem *item=new TileLayerItem(tileLayer, renderer, this);
            item->setZValue(index++);
            displayed_layer.insert(map,item);
        } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
            ObjectGroupItem *item=new ObjectGroupItem(objectGroup, renderer, this);
            item->setZValue(index++);
            displayed_layer.insert(map,item);
        }
    }
}

void MapItem::setMapPosition(Tiled::Map *map,qint16 x,qint16 y)
{
    QList<QGraphicsItem *> values = displayed_layer.values(map);
    int index=0;
    while(index<values.size())
    {
        values.at(index)->setPos(x*16,y*16);
        index++;
    }
}

QRectF MapItem::boundingRect() const
{
    return QRectF();
}

void MapItem::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

Map2Png::Map2Png(QWidget *parent) :
    QGraphicsView(parent),
    mScene(new QGraphicsScene(this))
{
    setWindowTitle(tr("Map2Png"));
    //scale(2,2);

    setScene(mScene);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing
                         | QGraphicsView::DontSavePainterState);
    setBackgroundBrush(Qt::black);
    setFrameStyle(QFrame::NoFrame);

    //viewport()->setAttribute(Qt::WA_StaticContents);
    //setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
}



Map2Png::~Map2Png()
{
/*    qDeleteAll(tiledMap->tilesets());
    delete tiledMap;
    delete tiledRender;*/
}

QString Map2Png::loadOtherMap(const QString &fileName)
{
    Map_full *tempMapObject=new Map_full();
    QFileInfo fileInformations(fileName);
    QString resolvedFileName=fileInformations.absoluteFilePath();
    if(other_map.contains(resolvedFileName))
        return resolvedFileName;

    //load the map
    tempMapObject->tiledMap = reader.readMap(resolvedFileName);
    if (!tempMapObject->tiledMap)
    {
        mLastError=reader.errorString();
        qDebug() << QString("Unable to load the map: %1, error: %2").arg(resolvedFileName).arg(reader.errorString());
        return QString();
    }
    int index=0;
    while(index<tempMapObject->tiledMap->layerCount())
    {
        if(tempMapObject->tiledMap->layerAt(index)->name()=="Moving" && tempMapObject->tiledMap->layerAt(index)->isObjectGroup())
        {
            QList<MapObject *> objects=tempMapObject->tiledMap->layerAt(index)->asObjectGroup()->objects();
            int index2=0;
            while(index2<objects.size())
            {
                if(objects.at(index2)->type()!="door")
                    tempMapObject->tiledMap->layerAt(index)->asObjectGroup()->removeObject(objects.at(index2));
                index2++;
            }
            break;
        }
        index++;
    }
    CatchChallenger::Map_loader map_loader;
    if(!map_loader.tryLoadMap(resolvedFileName))
    {
        mLastError=map_loader.errorString();
        qDebug() << QString("Unable to load the map: %1, error: %2").arg(resolvedFileName).arg(map_loader.errorString());
        delete tempMapObject->tiledMap;
        return QString();
    }

    //copy the variables
    tempMapObject->logicalMap.width                                 = map_loader.map_to_send.width;
    tempMapObject->logicalMap.height                                = map_loader.map_to_send.height;
    tempMapObject->logicalMap.parsed_layer.walkable                 = map_loader.map_to_send.parsed_layer.walkable;
    tempMapObject->logicalMap.parsed_layer.water                    = map_loader.map_to_send.parsed_layer.water;
    tempMapObject->logicalMap.map_file                              = resolvedFileName;
    tempMapObject->logicalMap.border.bottom.map                     = NULL;
    tempMapObject->logicalMap.border.top.map                        = NULL;
    tempMapObject->logicalMap.border.right.map                      = NULL;
    tempMapObject->logicalMap.border.left.map                       = NULL;

    //load the string
    tempMapObject->logicalMap.border_semi                = map_loader.map_to_send.border;
    if(!map_loader.map_to_send.border.bottom.fileName.isEmpty())
        tempMapObject->logicalMap.border_semi.bottom.fileName=QFileInfo(QFileInfo(resolvedFileName).absolutePath()+"/"+tempMapObject->logicalMap.border_semi.bottom.fileName).absoluteFilePath();
    if(!map_loader.map_to_send.border.top.fileName.isEmpty())
        tempMapObject->logicalMap.border_semi.top.fileName=QFileInfo(QFileInfo(resolvedFileName).absolutePath()+"/"+tempMapObject->logicalMap.border_semi.top.fileName).absoluteFilePath();
    if(!map_loader.map_to_send.border.right.fileName.isEmpty())
        tempMapObject->logicalMap.border_semi.right.fileName=QFileInfo(QFileInfo(resolvedFileName).absolutePath()+"/"+tempMapObject->logicalMap.border_semi.right.fileName).absoluteFilePath();
    if(!map_loader.map_to_send.border.left.fileName.isEmpty())
        tempMapObject->logicalMap.border_semi.left.fileName=QFileInfo(QFileInfo(resolvedFileName).absolutePath()+"/"+tempMapObject->logicalMap.border_semi.left.fileName).absoluteFilePath();

    //load the render
    switch (tempMapObject->tiledMap->orientation()) {
    case Map::Isometric:
        tempMapObject->tiledRender = new IsometricRenderer(tempMapObject->tiledMap);
        break;
    case Map::Orthogonal:
    default:
        tempMapObject->tiledRender = new OrthogonalRenderer(tempMapObject->tiledMap);
        break;
    }
    tempMapObject->tiledRender->setObjectBorder(false);

    //do the object group to move the player on it
    tempMapObject->objectGroup = new ObjectGroup();
    index=0;
    while(index<tempMapObject->tiledMap->layerCount())
    {
        if(tempMapObject->tiledMap->layerAt(index)->name()=="Collisions")
        {
            tempMapObject->tiledMap->insertLayer(index+1,tempMapObject->objectGroup);
            break;
        }
        index++;
    }

    other_map[resolvedFileName]=tempMapObject;

    return resolvedFileName;
}

void Map2Png::loadCurrentMap(const QString &fileName, qint32 x, qint32 y)
{
    qDebug() << QString("loadCurrentMap(%1)").arg(fileName);
    Map_full *tempMapObject;
    if(!other_map.contains(fileName))
    {
        qDebug() << QString("loadCurrentMap(): the current map is unable to load: %1").arg(fileName);
        return;
    }
    else
        tempMapObject=other_map[fileName];

    tempMapObject->x=x;
    tempMapObject->y=y;

    QString mapIndex;

    //if have border
    if(!tempMapObject->logicalMap.border_semi.bottom.fileName.isEmpty())
    {
        if(!other_map.contains(tempMapObject->logicalMap.border_semi.bottom.fileName))
        {
            mapIndex=loadOtherMap(tempMapObject->logicalMap.border_semi.bottom.fileName);
            //if is correctly loaded
            if(!mapIndex.isEmpty())
            {
                //if both border match
                if(fileName==other_map[mapIndex]->logicalMap.border_semi.top.fileName && tempMapObject->logicalMap.border_semi.bottom.fileName==mapIndex)
                {
                    tempMapObject->logicalMap.border.bottom.map=&other_map[mapIndex]->logicalMap;
                    int offset=tempMapObject->logicalMap.border_semi.bottom.x_offset-other_map[mapIndex]->logicalMap.border_semi.top.x_offset;
                    tempMapObject->logicalMap.border.bottom.x_offset=offset;
                    other_map[mapIndex]->logicalMap.border.top.x_offset=-offset;
                    loadCurrentMap(mapIndex,tempMapObject->x+offset,tempMapObject->y+tempMapObject->logicalMap.height);
                }
            }
        }
    }

    //if have border
    if(!tempMapObject->logicalMap.border_semi.top.fileName.isEmpty())
    {
        if(!other_map.contains(tempMapObject->logicalMap.border_semi.top.fileName))
        {
            mapIndex=loadOtherMap(tempMapObject->logicalMap.border_semi.top.fileName);
            //if is correctly loaded
            if(!mapIndex.isEmpty())
            {
                //if both border match
                if(fileName==other_map[mapIndex]->logicalMap.border_semi.bottom.fileName && tempMapObject->logicalMap.border_semi.top.fileName==mapIndex)
                {
                    tempMapObject->logicalMap.border.top.map=&other_map[mapIndex]->logicalMap;
                    int offset=tempMapObject->logicalMap.border_semi.top.x_offset-other_map[mapIndex]->logicalMap.border_semi.bottom.x_offset;
                    tempMapObject->logicalMap.border.top.x_offset=offset;
                    other_map[mapIndex]->logicalMap.border.bottom.x_offset=-offset;
                    loadCurrentMap(mapIndex,tempMapObject->x+offset,tempMapObject->y-other_map[mapIndex]->logicalMap.height);
                }
            }
        }
    }

    //if have border
    if(!tempMapObject->logicalMap.border_semi.left.fileName.isEmpty())
    {
        if(!other_map.contains(tempMapObject->logicalMap.border_semi.left.fileName))
        {
            mapIndex=loadOtherMap(tempMapObject->logicalMap.border_semi.left.fileName);
            //if is correctly loaded
            if(!mapIndex.isEmpty())
            {
                //if both border match
                if(fileName==other_map[mapIndex]->logicalMap.border_semi.right.fileName && tempMapObject->logicalMap.border_semi.left.fileName==mapIndex)
                {
                    tempMapObject->logicalMap.border.left.map=&other_map[mapIndex]->logicalMap;
                    int offset=tempMapObject->logicalMap.border_semi.left.y_offset-other_map[mapIndex]->logicalMap.border_semi.right.y_offset;
                    tempMapObject->logicalMap.border.left.y_offset=offset;
                    other_map[mapIndex]->logicalMap.border.right.y_offset=-offset;
                    loadCurrentMap(mapIndex,tempMapObject->x-other_map[mapIndex]->logicalMap.width,tempMapObject->y+offset);
                }
            }
        }
    }

    //if have border
    if(!tempMapObject->logicalMap.border_semi.right.fileName.isEmpty())
    {
        if(!other_map.contains(tempMapObject->logicalMap.border_semi.right.fileName))
        {
            mapIndex=loadOtherMap(tempMapObject->logicalMap.border_semi.right.fileName);
            //if is correctly loaded
            if(!mapIndex.isEmpty())
            {
                //if both border match
                if(fileName==other_map[mapIndex]->logicalMap.border_semi.left.fileName && tempMapObject->logicalMap.border_semi.right.fileName==mapIndex)
                {
                    tempMapObject->logicalMap.border.right.map=&other_map[mapIndex]->logicalMap;
                    int offset=tempMapObject->logicalMap.border_semi.right.y_offset-other_map[mapIndex]->logicalMap.border_semi.left.y_offset;
                    tempMapObject->logicalMap.border.right.y_offset=offset;
                    other_map[mapIndex]->logicalMap.border.left.y_offset=-offset;
                    loadCurrentMap(mapIndex,tempMapObject->x+tempMapObject->logicalMap.width,tempMapObject->y+offset);
                }
            }
        }
    }
}

void Map2Png::viewMap(const QString &fileName)
{
    QTime startTime;
    startTime.restart();

    mapItem=new MapItem();
    mScene->clear();
    centerOn(0, 0);

    QString current_map_fileName=loadOtherMap(fileName);
    if(current_map_fileName.isEmpty())
    {
        QMessageBox::critical(this,"Error",mLastError);
        return;
    }
    loadCurrentMap(current_map_fileName,0,0);

    displayMap();
    qDebug() << startTime.elapsed();
    mScene->addItem(mapItem);

    QSizeF size=mScene->sceneRect().size();
    QImage newImage(QSize(size.width(),size.height()),QImage::Format_ARGB32);
    newImage.fill(Qt::transparent);
    QPainter painter(&newImage);
    mScene->render(&painter);//,mScene->sceneRect()
    qDebug() << QString("mScene size: %1,%2").arg(mScene->sceneRect().size().width()).arg(mScene->sceneRect().size().height());

    QString destination = QFileDialog::getSaveFileName(NULL,"Save the render",QString(),"Png Images (*.png)");
    if(destination.isEmpty() || destination.isNull() || destination=="")
        return;
    qDebug() << QString("save as: %1").arg(destination);
    if(!destination.endsWith(".png"))
        destination+=".png";
    if(!newImage.save(destination))
    {
        QMessageBox::critical(NULL,"Error","Unable to save the image");
        qDebug() << QString("Unable to save the image");
    }
}

void Map2Png::displayMap()
{
    qDebug() << QString("displayMap()");

    QHash<QString,Map_full *>::const_iterator i = other_map.constBegin();
     while (i != other_map.constEnd()) {
         qDebug() << QString("displayMap(): %1 at %2,%3").arg(i.key()).arg(i.value()->x).arg(i.value()->y);
         mapItem->addMap(i.value()->tiledMap,i.value()->tiledRender);
         mapItem->setMapPosition(i.value()->tiledMap,i.value()->x,i.value()->y);
         ++i;
     }
}

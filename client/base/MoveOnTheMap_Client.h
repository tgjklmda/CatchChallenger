#ifndef MOVEONTHEMAP_CLIENT_H
#define MOVEONTHEMAP_CLIENT_H

#include <QObject>

#include "GeneralStructures.h"

class MoveOnTheMap_Client
{
public:
	MoveOnTheMap_Client();
protected:
	MoveOnTheMap_Client *map;
	quint16 x,y;
protected:
	bool canGoTo(Direction direction);
};

#endif // MOVEONTHEMAP_CLIENT_H
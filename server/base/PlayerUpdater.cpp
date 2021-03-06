#include "PlayerUpdater.h"
#include "../../general/base/DebugClass.h"
#include "GlobalServerData.h"

using namespace CatchChallenger;
PlayerUpdater::PlayerUpdater()
{
    connected_players=0;
    sended_connected_players=0;
    next_send_timer.setSingleShot(true);

    //Max bandwith: (number max of player in this mode)*(packet size)*(tick by second)=9*16*4=576B/s
    next_send_timer.setInterval(250);

    connect(this,SIGNAL(send_addConnectedPlayer()),this,SLOT(internal_addConnectedPlayer()),Qt::QueuedConnection);
    connect(this,SIGNAL(send_removeConnectedPlayer()),this,SLOT(internal_removeConnectedPlayer()),Qt::QueuedConnection);
    connect(&next_send_timer,SIGNAL(timeout()),this,SLOT(send_timer()),Qt::QueuedConnection);
}

void PlayerUpdater::addConnectedPlayer()
{
    emit send_addConnectedPlayer();
}

void PlayerUpdater::removeConnectedPlayer()
{
    emit send_removeConnectedPlayer();
}

void PlayerUpdater::internal_addConnectedPlayer()
{
    connected_players++;
    switch(connected_players)
    {
        //Max bandwith: (number max of player in this mode)*(packet size)*(tick by second)=49*16*1=735B/s
        case 10:
            next_send_timer.setInterval(1000);
        break;
        //Max bandwith: (number max of player in this mode)*(packet size)*(tick by second)=99*16*0.5=792B/s
        case 50:
            next_send_timer.setInterval(2000);
        break;
        //Max bandwith: (number max of player in this mode)*(packet size)*(tick by second)=999*16*0.1=1.6KB/s
        case 100:
            next_send_timer.setInterval(10000);
        break;
        //Max bandwith: (number max of player in this mode)*(packet size)*(tick by second)=65535*16*0.016=16.7KB/s
        case 1000:
            next_send_timer.setInterval(60000);
        break;
        default:
        break;
    }

    if(!next_send_timer.isActive())
        next_send_timer.start();
}

void PlayerUpdater::internal_removeConnectedPlayer()
{
    connected_players--;
    switch(connected_players)
    {
        case (10-1):
            next_send_timer.setInterval(250);
        break;
        case (50-1):
            next_send_timer.setInterval(1000);
        break;
        case (100-1):
            next_send_timer.setInterval(2000);
        break;
        case (1000-1):
            next_send_timer.setInterval(10000);
        break;
        default:
        break;
    }
    if(!next_send_timer.isActive())
        next_send_timer.start();
}

void PlayerUpdater::send_timer()
{
    if(GlobalServerData::serverSettings.commmonServerSettings.sendPlayerNumber && sended_connected_players!=connected_players)
    {
        sended_connected_players=connected_players;
        emit newConnectedPlayer(connected_players);
    }
}

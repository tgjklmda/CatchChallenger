#ifndef CATCHCHALLENGER_VARIABLESERVER_H
#define CATCHCHALLENGER_VARIABLESERVER_H

#define DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
#define DEBUG_MESSAGE_CLIENT_FIGHT
#define DEBUG_MESSAGE_CLIENT_SQL
/*
#define DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
#define DEBUG_MESSAGE_CLIENT_MOVE
//*/

/*
#define DEBUG_MESSAGE_CLIENT_RAW_NETWORK
#define DEBUG_MESSAGE_MAP_LOAD
#define DEBUG_MESSAGE_MAP_PLANTS
#define DEBUG_MESSAGE_BUFF_LOAD
#define DEBUG_MESSAGE_SKILL_LOAD
#define DEBUG_MESSAGE_MONSTER_XP_LOAD
//*/
#define DEBUG_MESSAGE_MONSTER_LOAD

//in ms
#define CATCHCHALLENGER_SERVER_MAP_TIME_TO_SEND_MOVEMENT 150
#define CATCHCHALLENGER_SERVER_NORMAL_SPEED 250 //then 250ms, see the protocol

#define CATCHCHALLENGER_SERVER_OWNER_TIMEOUT 60*60*24
#define RANDOM_FLOAT_PART_DIVIDER 10000

/** map visibility bandwith optimisation
 Do in define to not drop cpu performance, due to heavy call **/

/** convert the overflow of move into insert, use more cpu than CATCHCHALLENGER_SERVER_VISIBILITY_CLEAR
 * Disable on very slow configuration, add <5% of cpu with simple algorithm with 400 benchmark's bot on same map */
#define CATCHCHALLENGER_SERVER_MAP_DROP_OVER_MOVE
/// not send all packet, drop move/insert if remove, ... use very few more cpu, < 1% more cpu
#define CATCHCHALLENGER_SERVER_VISIBILITY_CLEAR
/// \brief drop the previous if stopped step, need CATCHCHALLENGER_SERVER_MAP_DROP_OVER_MOVE
#define CATCHCHALLENGER_SERVER_MAP_DROP_STOP_MOVE

//put for map, add for local thread
//put this size into the options
#define CATCHCHALLENGER_SERVER_RANDOM_LIST_SIZE 512
#define CATCHCHALLENGER_SERVER_MIN_RANDOM_LIST_SIZE 128

/// check more, then use more cpu, it's to develop and detect the internal bug
#define CATCHCHALLENGER_SERVER_EXTRA_CHECK

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#endif // VARIABLESERVER_H

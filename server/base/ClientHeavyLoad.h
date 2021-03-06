#ifndef CATCHCHALLENGER_CLIENTHEAVYLOAD_H
#define CATCHCHALLENGER_CLIENTHEAVYLOAD_H

#include <QObject>
#include <QSqlDatabase>
#include <QStringList>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QCoreApplication>
#include <QFile>
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>

#include "ServerStructures.h"
#include "../VariableServer.h"
#include "../../general/base/DebugClass.h"

namespace CatchChallenger {
class ClientHeavyLoad : public QObject
{
    Q_OBJECT
public:
    explicit ClientHeavyLoad();
    ~ClientHeavyLoad();
    void setVariable(Player_internal_informations *player_informations);
    static QList<quint16> simplifiedIdList;
public slots:
    virtual void askLogin(const quint8 &query_id,const QString &login,const QByteArray &hash);
    virtual void askLoginBot(const quint8 &query_id);
    //check each element of the datapack, determine if need be removed, updated, add as new file all the missing file
    void datapackList(const quint8 &query_id, const QStringList &files, const QList<quint64> &timestamps);
    void dbQuery(const QString &queryText);
    void askedRandomNumber();
    //normal slots
    void askIfIsReadyToStop();
private:
    // ------------------------------
    bool sendFile(const QString &fileName, const quint64 &mtime);
    QString SQL_text_quote(QString text);
    // ------------------------------
    Player_internal_informations *player_informations;
    bool loadTheRawUTF8String();
    void loginIsRight(const quint8 &query_id,quint32 id,Map* map,const /*COORD_TYPE*/ quint8 &x,const /*COORD_TYPE*/ quint8 &y,const Orientation &orientation);
    void loginIsRightWithParsedRescue(const quint8 &query_id,quint32 id,Map* map,const /*COORD_TYPE*/ quint8 &x,const /*COORD_TYPE*/ quint8 &y,const Orientation &orientation,
                      Map* rescue_map,const /*COORD_TYPE*/ quint8 &rescue_x,const /*COORD_TYPE*/ quint8 &rescue_y,const Orientation &rescue_orientation);
    void loginIsRightWithRescue(const quint8 &query_id,quint32 id,Map* map,const /*COORD_TYPE*/ quint8 &x,const /*COORD_TYPE*/ quint8 &y,const Orientation &orientation,
                      const QVariant &rescue_map,const QVariant &rescue_x,const QVariant &rescue_y,const QVariant &rescue_orientation);
    void loginIsWrong(const quint8 &query_id,const QString &messageToSend,const QString &debugMessage);
    //load linked data (like item, quests, ...)
    void loadLinkedData();
    void loadItems();
    void loadRecipes();
    void loadMonsters();
    void sendInventory();
    QList<PlayerMonster::Buff> loadMonsterBuffs(const quint32 &monsterId);
    QList<PlayerMonster::Skill> loadMonsterSkills(const quint32 &monsterId);
signals:
    //normal signals
    void error(const QString &error);
    void message(const QString &message);
    void isReadyToStop();
    //send packet on network
    void sendPacket(const quint8 &mainIdent,const quint16 &subIdent,const QByteArray &data=QByteArray());
    void sendPacket(const quint8 &mainIdent,const QByteArray &data=QByteArray());
    //send reply
    void postReply(const quint8 &queryNumber,const QByteArray &data);
    //login linked signals
    void send_player_informations();
    void isLogged();
    void put_on_the_map(Map* map,const /*COORD_TYPE*/ quint8 &x,const /*COORD_TYPE*/ quint8 &y,const Orientation &orientation);
    //random linked signals
    void newRandomNumber(const QByteArray &randomData);
};
}

#endif // CLIENTHEAVYLOAD_H

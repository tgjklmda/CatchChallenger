#ifndef CHAT_H
#define CHAT_H

#include "../../general/base/ChatParsing.h"
#include "../../general/base/GeneralStructures.h"
#include "../Api_protocol.h"

#include <QWidget>
#include <QTimer>
#include <QString>

namespace Ui {
class Chat;
}

namespace Pokecraft {
class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(QWidget *parent,Pokecraft::Api_protocol *client);
    ~Chat();
    Pokecraft::Api_protocol *client;
    void resetAll();
public slots:
    void new_system_text(Pokecraft::Chat_type chat_type,QString text);
    void new_chat_text(Pokecraft::Chat_type chat_type,QString text,QString pseudo,Pokecraft::Player_type type);
private slots:
    void on_pushButtonChat_toggled(bool checked);
    void lineEdit_chat_text_returnPressed();
    void removeNumberForFlood();
    void update_chat();
    QString toHtmlEntities(QString text);
    QString toSmilies(QString text);
    void comboBox_chat_type_currentIndexChanged(int index);
private:
    Ui::Chat *ui;
    QString lastMessageSend;
    QTimer stopFlood;
    int numberForFlood;

    QStringList chat_list_player_pseudo;
    QList<Pokecraft::Player_type> chat_list_player_type;
    QList<Pokecraft::Chat_type> chat_list_type;
    QList<QString> chat_list_text;
};
}

#endif // CHAT_H
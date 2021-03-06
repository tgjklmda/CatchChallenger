#include "Chat.h"
#include "ui_Chat.h"
#include "../Api_client_real.h"

using namespace CatchChallenger;

Chat* Chat::chat=NULL;

Chat::Chat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chat)
{
    ui->setupUi(this);
    connect(&stopFlood,SIGNAL(timeout()),this,SLOT(removeNumberForFlood()),Qt::QueuedConnection);
    connect(ui->comboBox_chat_type,SIGNAL(currentIndexChanged(int)),this,SLOT(comboBox_chat_type_currentIndexChanged(int)));
    connect(ui->lineEdit_chat_text,SIGNAL(returnPressed()),this,SLOT(lineEdit_chat_text_returnPressed()));

    stopFlood.setSingleShot(false);
    stopFlood.start(1500);
    numberForFlood=0;
}

Chat::~Chat()
{
    delete ui;
}

void Chat::resetAll()
{
    chat_list_player_pseudo.clear();
    chat_list_player_type.clear();
    chat_list_type.clear();
    chat_list_text.clear();
    ui->textBrowser_chat->clear();
    ui->comboBox_chat_type->setCurrentIndex(1);
    ui->lineEdit_chat_text->setText("");
    update_chat();
}

void Chat::comboBox_chat_type_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    update_chat();
}

void Chat::on_pushButtonChat_toggled(bool checked)
{
    ui->textBrowser_chat->setVisible(checked);
    ui->lineEdit_chat_text->setVisible(checked);
    ui->comboBox_chat_type->setVisible(checked);
}

void Chat::lineEdit_chat_text_returnPressed()
{
    QString text=ui->lineEdit_chat_text->text();
    if(text.isEmpty())
        return;
    if(text.contains(QRegExp("^ +$")))
    {
        ui->lineEdit_chat_text->clear();
        new_system_text(Chat_type_system,"Space text not allowed");
        return;
    }
    if(text.size()>256)
    {
        ui->lineEdit_chat_text->clear();
        new_system_text(Chat_type_system,"Message too long");
        return;
    }
    if(!text.startsWith('/'))
    {
        if(text==lastMessageSend)
        {
            ui->lineEdit_chat_text->clear();
            new_system_text(Chat_type_system,"Send message like as previous");
            return;
        }
        if(numberForFlood>2)
        {
            ui->lineEdit_chat_text->clear();
            new_system_text(Chat_type_system,"Stop flood");
            return;
        }
    }
    numberForFlood++;
    lastMessageSend=text;
    ui->lineEdit_chat_text->setText("");
    if(!text.startsWith("/pm "))
    {
        Chat_type chat_type;
        switch(ui->comboBox_chat_type->currentIndex())
        {
        default:
        case 0:
            chat_type=Chat_type_all;
        break;
        case 1:
            chat_type=Chat_type_local;
        break;
        case 2:
            chat_type=Chat_type_clan;
        break;
        }
        CatchChallenger::Api_client_real::client->sendChatText(chat_type,text);
    }
    else if(text.contains(QRegExp("^/pm [^ ]+ .+$")))
    {
        QString pseudo=text;
        pseudo.replace(QRegExp("^/pm ([^ ]+) .+$"), "\\1");
        text.replace(QRegExp("^/pm [^ ]+ (.+)$"), "\\1");
        CatchChallenger::Api_client_real::client->sendPM(text,pseudo);
    }
}

void Chat::removeNumberForFlood()
{
    if(numberForFlood<=0)
        return;
    numberForFlood--;
}

void Chat::new_system_text(CatchChallenger::Chat_type chat_type,QString text)
{
    #ifdef DEBUG_BASEWINDOWS
    qDebug() << QString("new_system_text: %1").arg(text);
    #endif
    chat_list_player_type << Player_type_normal;
    chat_list_player_pseudo << "";
    chat_list_type << chat_type;
    chat_list_text << text;
    while(chat_list_player_type.size()>64)
    {
        chat_list_player_type.removeFirst();
        chat_list_player_pseudo.removeFirst();
        chat_list_type.removeFirst();
        chat_list_text.removeFirst();
    }
    update_chat();
}

void Chat::new_chat_text(CatchChallenger::Chat_type chat_type,QString text,QString pseudo,CatchChallenger::Player_type type)
{
    #ifdef DEBUG_BASEWINDOWS
    qDebug() << QString("new_chat_text: %1 by %2").arg(text).arg(pseudo);
    #endif
    chat_list_player_type << type;
    chat_list_player_pseudo << pseudo;
    chat_list_type << chat_type;
    chat_list_text << text;
    while(chat_list_player_type.size()>64)
    {
        chat_list_player_type.removeFirst();
        chat_list_player_pseudo.removeFirst();
        chat_list_type.removeFirst();
        chat_list_text.removeFirst();
    }
    update_chat();
}

void Chat::update_chat()
{
    QString nameHtml;
    int index=0;
    while(index<chat_list_player_pseudo.size())
    {
        bool addPlayerInfo=true;
        if(chat_list_type.at(index)==Chat_type_system || chat_list_type.at(index)==Chat_type_system_important)
            addPlayerInfo=false;
        if(!addPlayerInfo)
            nameHtml+=ChatParsing::new_chat_message("",Player_type_normal,chat_list_type.at(index),chat_list_text.at(index));
        else
            nameHtml+=ChatParsing::new_chat_message(chat_list_player_pseudo.at(index),chat_list_player_type.at(index),chat_list_type.at(index),chat_list_text.at(index));
        index++;
    }
    ui->textBrowser_chat->setHtml(nameHtml);
    //textBrowser_chat->scrollToAnchor(QString::number(index-1));
}

QString Chat::toHtmlEntities(QString text)
{
    text.replace("&","&amp;");
    text.replace("\"","&quot;");
    text.replace("'","&#039;");
    text.replace("<","&lt;");
    text.replace(">","&gt;");
    return text;
}

QString Chat::toSmilies(QString text)
{
    text.replace(":)","<img src=\":/images/smiles/face-smile.png\" alt=\"\" style=\"vertical-align:middle;\" />");
    text.replace(":|","<img src=\":/images/smiles/face-plain.png\" alt=\"\" style=\"vertical-align:middle;\" />");
    text.replace(":(","<img src=\":/images/smiles/face-sad.png\" alt=\"\" style=\"vertical-align:middle;\" />");
    text.replace(":P","<img src=\":/images/smiles/face-raspberry.png\" alt=\"\" style=\"vertical-align:middle;\" />");
    text.replace(":p","<img src=\":/images/smiles/face-raspberry.png\" alt=\"\" style=\"vertical-align:middle;\" />");
    text.replace(":D","<img src=\":/images/smiles/face-laugh.png\" alt=\"\" style=\"vertical-align:middle;\" />");
    text.replace(":o","<img src=\":/images/smiles/face-surprise.png\" alt=\"\" style=\"vertical-align:middle;\" />");
    text.replace(";)","<img src=\":/images/smiles/face-wink.png\" alt=\"\" style=\"vertical-align:middle;\" />");
    return text;
}

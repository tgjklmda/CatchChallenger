#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	settings=new QSettings(QCoreApplication::applicationDirPath()+"/server.properties",QSettings::IniFormat);
	ui->setupUi(this);
	on_MapVisibilityAlgorithm_currentIndexChanged(0);
	updateActionButton();
	qRegisterMetaType<Chat_type>("Chat_type");
	connect(&eventDispatcher,SIGNAL(is_started(bool)),this,SLOT(server_is_started(bool)));
	connect(&eventDispatcher,SIGNAL(need_be_stopped()),this,SLOT(server_need_be_stopped()));
	connect(&eventDispatcher,SIGNAL(need_be_restarted()),this,SLOT(server_need_be_restarted()));
	connect(&eventDispatcher,SIGNAL(new_player_is_connected(Player_private_and_public_informations)),this,SLOT(new_player_is_connected(Player_private_and_public_informations)));
	connect(&eventDispatcher,SIGNAL(player_is_disconnected(QString)),this,SLOT(player_is_disconnected(QString)));
	connect(&eventDispatcher,SIGNAL(new_chat_message(QString,Chat_type,QString)),this,SLOT(new_chat_message(QString,Chat_type,QString)));
	connect(&eventDispatcher,SIGNAL(error(QString)),this,SLOT(server_error(QString)));
	connect(&eventDispatcher,SIGNAL(benchmark_result(int,double,double,double,double,double)),this,SLOT(benchmark_result(int,double,double,double,double,double)));
	connect(&timer_update_the_info,SIGNAL(timeout()),this,SLOT(update_the_info()));
	connect(&check_latency,SIGNAL(timeout()),this,SLOT(start_calculate_latency()));
	connect(this,SIGNAL(record_latency()),this,SLOT(stop_calculate_latency()),Qt::QueuedConnection);
	timer_update_the_info.start(200);
	check_latency.setSingleShot(true);
	check_latency.start(1000);
	need_be_restarted=false;
	need_be_closed=false;
	ui->tabWidget->setCurrentIndex(0);
	internal_currentLatency=0;
	load_settings();
}

MainWindow::~MainWindow()
{
	delete settings;
	delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	event->ignore();
	this->hide();
	need_be_closed=true;
	if(!eventDispatcher.isStopped())
		server_need_be_stopped();
	else
		QCoreApplication::exit();
}

void MainWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
	break;
	default:
	break;
	}
}

void MainWindow::on_lineEdit_returnPressed()
{
	ui->lineEdit->setText("");
}

void MainWindow::updateActionButton()
{
	ui->pushButton_server_start->setEnabled(eventDispatcher.isStopped());
	ui->pushButton_server_restart->setEnabled(eventDispatcher.isListen());
	ui->pushButton_server_stop->setEnabled(eventDispatcher.isListen());
	ui->pushButton_server_benchmark->setEnabled(eventDispatcher.isStopped());
}

void MainWindow::on_pushButton_server_start_clicked()
{
	eventDispatcher.start_server();
}

void MainWindow::on_pushButton_server_stop_clicked()
{
	eventDispatcher.stop_server();
}

void MainWindow::on_pushButton_server_restart_clicked()
{
	need_be_restarted=true;
	eventDispatcher.stop_server();
}

void MainWindow::server_is_started(bool is_started)
{
	updateActionButton();
	if(need_be_closed)
	{
		QCoreApplication::exit();
		return;
	}
	if(!is_started)
	{
		clean_updated_info();
		if(need_be_restarted)
		{
			need_be_restarted=false;
			eventDispatcher.start_server();
		}
	}
}

void MainWindow::server_need_be_stopped()
{
	eventDispatcher.stop_server();
}

void MainWindow::server_need_be_restarted()
{
	need_be_restarted=true;
	eventDispatcher.stop_server();
}

void MainWindow::new_player_is_connected(Player_private_and_public_informations player)
{
	ui->listPlayer->addItem(player.public_informations.pseudo);
	players << player;
}

void MainWindow::player_is_disconnected(QString pseudo)
{
	QList<QListWidgetItem *> tempList=ui->listPlayer->findItems(pseudo,Qt::MatchExactly);
	int index=0;
	while(index<tempList.size())
	{
		delete tempList.at(index);
		index++;
	}
	index=0;
	while(index<players.size())
	{
		if(players.at(index).public_informations.pseudo==pseudo)
		{
			players.removeAt(index);
			break;
		}
		index++;
	}
}

void MainWindow::new_chat_message(QString pseudo,Chat_type type,QString text)
{
	int index=0;
	while(index<players.size())
	{
		if(players.at(index).public_informations.pseudo==pseudo)
		{
			QString html=ui->textBrowserChat->toHtml();
			html+=ChatParsing::new_chat_message(players.at(index).public_informations.pseudo,players.at(index).public_informations.type,type,text);
			if(html.size()>1024*1024)
				html=html.mid(html.size()-1024*1024,1024*1024);
			ui->textBrowserChat->setHtml(html);
			return;
		}
		index++;
	}
	QMessageBox::information(this,"warning","unable to locate the player");
}

void MainWindow::server_error(QString error)
{
	QMessageBox::information(this,"warning",error);
}

void MainWindow::update_the_info()
{
	if(ui->listLatency->count()<=0)
		ui->listLatency->addItem(tr("%1ms").arg(internal_currentLatency));
	else
		ui->listLatency->item(0)->setText(tr("%1ms").arg(internal_currentLatency));
	quint16 total_latency=internal_currentLatency;
	if(eventDispatcher.isListen() || eventDispatcher.isInBenchmark())
	{
		quint16 player_current,player_max;
		player_current=eventDispatcher.player_current();
		player_max=eventDispatcher.player_max();
		ui->label_player->setText(QString("%1/%2").arg(player_current).arg(player_max));
		ui->progressBar_player->setMaximum(player_max);
		ui->progressBar_player->setValue(player_current);
	}
	else
	{
		ui->progressBar_player->setValue(0);
	}
	QStringList latencyList=eventDispatcher.getLatency();
	int index=0;
	while(index<latencyList.size())
	{
		if(ui->listLatency->count()<=(index+1))
			ui->listLatency->addItem(latencyList.at(index));
		else
			ui->listLatency->item(index+1)->setText(latencyList.at(index));
		index++;
	}
	total_latency+=eventDispatcher.getTotalLatency();
	int last_item=index+1;
	while(last_item<ui->listLatency->count())
	{
		delete ui->listLatency->item(last_item+1);
		last_item++;
	}

	ui->label_latency->setText(tr("%1ms").arg(total_latency));
}

QString MainWindow::sizeToString(double size)
{
	if(size<1024)
		return QString::number(size)+tr("B");
	if((size=size/1024)<1024)
		return adaptString(size)+tr("KB");
	if((size=size/1024)<1024)
		return adaptString(size)+tr("MB");
	if((size=size/1024)<1024)
		return adaptString(size)+tr("GB");
	if((size=size/1024)<1024)
		return adaptString(size)+tr("TB");
	if((size=size/1024)<1024)
		return adaptString(size)+tr("PB");
	if((size=size/1024)<1024)
		return adaptString(size)+tr("EB");
	if((size=size/1024)<1024)
		return adaptString(size)+tr("ZB");
	if((size=size/1024)<1024)
		return adaptString(size)+tr("YB");
	return tr("Too big");
}

QString MainWindow::adaptString(float size)
{
	if(size>=100)
		return QString::number(size,'f',0);
	else
		return QString::number(size,'g',3);
}


void MainWindow::start_calculate_latency()
{
	time_latency.restart();
	emit record_latency();
}

void MainWindow::stop_calculate_latency()
{
	internal_currentLatency=time_latency.elapsed();
	check_latency.start();
}

void MainWindow::benchmark_result(int latency,double TX_speed,double RX_speed,double TX_size,double RX_size,double second)
{
	updateActionButton();
	clean_updated_info();
	QMessageBox::information(this,"Benchmark",tr("The latency of the benchmark: %1\nTX_speed: %2/s, RX_speed %3/s\nTX_size: %4, RX_size: %5, in %6s\nThis latency is cumulated latency of different point. That's not show the database performance.")
				 .arg(latency)
				 .arg(sizeToString(TX_speed))
				 .arg(sizeToString(RX_speed))
				 .arg(sizeToString(TX_size))
				 .arg(sizeToString(RX_size))
				 .arg(second)
				 );
}

void MainWindow::clean_updated_info()
{
	ui->label_player->setText("?/?");
	ui->listLatency->clear();
}

void MainWindow::load_settings()
{
	if(!settings->contains("max-players"))
		settings->setValue("max-players",200);
	if(!settings->contains("server-ip"))
		settings->setValue("server-ip","");
	if(!settings->contains("pvp"))
		settings->setValue("pvp",true);
	if(!settings->contains("server-port"))
		settings->setValue("server-port",42489);

	settings->beginGroup("MapVisibilityAlgorithm");
	if(!settings->contains("MapVisibilityAlgorithm"))
		settings->setValue("MapVisibilityAlgorithm",0);
	settings->endGroup();

	settings->beginGroup("MapVisibilityAlgorithm-Simple");
	if(!settings->contains("Max"))
		settings->setValue("Max",50);
	settings->endGroup();

	settings->beginGroup("rates");
	if(!settings->contains("xp_normal"))
		settings->setValue("xp_normal",1.0);
	if(!settings->contains("xp_premium"))
		settings->setValue("xp_premium",1.0);
	if(!settings->contains("gold_normal"))
		settings->setValue("gold_normal",1.0);
	if(!settings->contains("gold_premium"))
		settings->setValue("gold_premium",1.0);
	if(!settings->contains("shiny_normal"))
		settings->setValue("shiny_normal",0.0001);
	if(!settings->contains("shiny_premium"))
		settings->setValue("shiny_premium",0.0002);
	settings->endGroup();

	settings->beginGroup("chat");
	if(!settings->contains("allow-all"))
		settings->setValue("allow-all",true);
	if(!settings->contains("allow-local"))
		settings->setValue("allow-local",true);
	if(!settings->contains("allow-private"))
		settings->setValue("allow-private",true);
	if(!settings->contains("allow-aliance"))
		settings->setValue("allow-aliance",true);
	if(!settings->contains("allow-clan"))
		settings->setValue("allow-clan",true);
	settings->endGroup();

	settings->beginGroup("db");
	if(!settings->contains("host"))
		settings->setValue("host","localhost");
	if(!settings->contains("login"))
		settings->setValue("login","pokecraft-login");
	if(!settings->contains("pass"))
		settings->setValue("pass","pokecraft-pass");
	if(!settings->contains("db"))
		settings->setValue("db","pokecraft");
	settings->endGroup();
	// --------------------------------------------------
	ui->max_player->setValue(settings->value("max-players").toUInt());
	ui->server_ip->setText(settings->value("server-ip").toString());
	ui->pvp->setChecked(settings->value("pvp").toBool());
	ui->server_port->setValue(settings->value("server-port").toUInt());

	quint32 tempValue=0;
	settings->beginGroup("MapVisibilityAlgorithm");
	tempValue=settings->value("MapVisibilityAlgorithm").toUInt();
	settings->endGroup();
	if(tempValue<(quint32)ui->MapVisibilityAlgorithm->count())
		ui->MapVisibilityAlgorithm->setCurrentIndex(tempValue);

	settings->beginGroup("MapVisibilityAlgorithm-Simple");
	tempValue=settings->value("Max").toUInt();
	settings->endGroup();
	ui->MapVisibilityAlgorithmSimpleMax->setValue(tempValue);

	settings->beginGroup("rates");
	double rates_xp_normal=settings->value("xp_normal").toReal();
	double rates_xp_premium=settings->value("xp_premium").toReal();
	double rates_gold_normal=settings->value("gold_normal").toReal();
	double rates_gold_premium=settings->value("gold_premium").toReal();
	double rates_shiny_normal=settings->value("shiny_normal").toReal();
	double rates_shiny_premium=settings->value("shiny_premium").toReal();
	settings->endGroup();

	ui->rates_xp_normal->setValue(rates_xp_normal);
	ui->rates_xp_premium->setValue(rates_xp_premium);
	ui->rates_gold_normal->setValue(rates_gold_normal);
	ui->rates_gold_premium->setValue(rates_gold_premium);
	ui->rates_shiny_normal->setValue(rates_shiny_normal);
	ui->rates_shiny_premium->setValue(rates_shiny_premium);

	settings->beginGroup("chat");
	bool chat_allow_all=settings->value("allow-all").toBool();
	bool chat_allow_local=settings->value("allow-local").toBool();
	bool chat_allow_private=settings->value("allow-private").toBool();
	bool chat_allow_aliance=settings->value("allow-aliance").toBool();
	bool chat_allow_clan=settings->value("allow-clan").toBool();
	settings->endGroup();

	ui->chat_allow_all->setChecked(chat_allow_all);
	ui->chat_allow_local->setChecked(chat_allow_local);
	ui->chat_allow_private->setChecked(chat_allow_private);
	ui->chat_allow_aliance->setChecked(chat_allow_aliance);
	ui->chat_allow_clan->setChecked(chat_allow_clan);

	settings->beginGroup("db");
	QString db_host=settings->value("host").toString();
	QString bd_login=settings->value("login").toString();
	QString bd_pass=settings->value("pass").toString();
	QString bd_base=settings->value("db").toString();
	settings->endGroup();

	ui->db_host->setText(db_host);
	ui->db_login->setText(bd_login);
	ui->db_pass->setText(bd_pass);
	ui->db_base->setText(bd_base);
}

void MainWindow::on_max_player_valueChanged(int arg1)
{
	settings->setValue("max-players",arg1);
}

void MainWindow::on_server_ip_editingFinished()
{
	settings->setValue("server-ip",ui->server_ip->text());
}

void MainWindow::on_pvp_stateChanged(int arg1)
{
	Q_UNUSED(arg1)
	settings->setValue("pvp",ui->pvp->isChecked());
}

void MainWindow::on_server_port_valueChanged(int arg1)
{
	settings->setValue("server-port",arg1);
}

void MainWindow::on_rates_xp_normal_valueChanged(double arg1)
{
	settings->beginGroup("rates");
	settings->setValue("xp_normal",arg1);
	settings->endGroup();
}

void MainWindow::on_rates_xp_premium_valueChanged(double arg1)
{
	settings->beginGroup("rates");
	settings->setValue("xp_premium",arg1);
	settings->endGroup();
}

void MainWindow::on_rates_gold_normal_valueChanged(double arg1)
{
	settings->beginGroup("rates");
	settings->setValue("gold_normal",arg1);
	settings->endGroup();
}

void MainWindow::on_rates_gold_premium_valueChanged(double arg1)
{
	settings->beginGroup("rates");
	settings->setValue("gold_premium",arg1);
	settings->endGroup();
}

void MainWindow::on_rates_shiny_normal_valueChanged(double arg1)
{
	settings->beginGroup("rates");
	settings->setValue("shiny_normal",arg1);
	settings->endGroup();
}

void MainWindow::on_rates_shiny_premium_valueChanged(double arg1)
{
	settings->beginGroup("rates");
	settings->setValue("shiny_premium",arg1);
	settings->endGroup();
}

void MainWindow::on_chat_allow_all_toggled(bool checked)
{
	settings->beginGroup("chat");
	settings->setValue("allow-all",checked);
	settings->endGroup();
}

void MainWindow::on_chat_allow_local_toggled(bool checked)
{
	settings->beginGroup("chat");
	settings->setValue("allow-local",checked);
	settings->endGroup();
}

void MainWindow::on_chat_allow_private_toggled(bool checked)
{
	settings->beginGroup("chat");
	settings->setValue("allow-private",checked);
	settings->endGroup();
}

void MainWindow::on_chat_allow_aliance_toggled(bool checked)
{
	settings->beginGroup("chat");
	settings->setValue("allow-aliance",checked);
	settings->endGroup();
}

void MainWindow::on_chat_allow_clan_toggled(bool checked)
{
	settings->beginGroup("chat");
	settings->setValue("allow-clan",checked);
	settings->endGroup();
}

void MainWindow::on_db_host_editingFinished()
{
	settings->beginGroup("db");
	settings->setValue("host",ui->db_host->text());
	settings->endGroup();
}

void MainWindow::on_db_login_editingFinished()
{
	settings->beginGroup("db");
	settings->setValue("login",ui->db_login->text());
	settings->endGroup();
}

void MainWindow::on_db_pass_editingFinished()
{
	settings->beginGroup("db");
	settings->setValue("pass",ui->db_pass->text());
	settings->endGroup();
}

void MainWindow::on_db_base_editingFinished()
{
	settings->beginGroup("db");
	settings->setValue("db",ui->db_base->text());
	settings->endGroup();
}

void MainWindow::on_pushButton_server_benchmark_clicked()
{
	eventDispatcher.start_benchmark(ui->benchmark_seconds->value(),ui->benchmark_clients->value());
	updateActionButton();
}

void MainWindow::on_MapVisibilityAlgorithm_currentIndexChanged(int index)
{
	if(index==0)
		ui->groupBoxMapVisibilityAlgorithmSimple->setEnabled(true);
	settings->beginGroup("MapVisibilityAlgorithm");
	settings->setValue("MapVisibilityAlgorithm",index);
	settings->endGroup();
}

void MainWindow::on_MapVisibilityAlgorithmSimpleMax_valueChanged(int arg1)
{
	settings->beginGroup("MapVisibilityAlgorithm-Simple");
	settings->setValue("Max",arg1);
	settings->endGroup();
}
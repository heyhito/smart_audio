#include "player.h"
#include "ui_player.h"

// Player类的构造函数
Player::Player(QTcpSocket *s, QString a, QString d, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Player)
{
    ui->setupUi(this); // 设置UI

    // 初始化成员变量
    this->socket = s;
    this->appid = a;
    this->deviceid = d;

    flag = 1;
    start_flag = 0;
    suspend_flag = 0;

    // 设置按钮字体和文本
    QFont f("黑体", 28);
    ui->playButton->setFont(f);
    ui->playButton->setText("▷");

    // 连接成功信号
    connect(socket, &QTcpSocket::connected, [this]()
    {
        QMessageBox::information(this, "连接提示", "连接服务器成功");
    });

    // 断开连接信号
    connect(socket, &QTcpSocket::disconnected, [this]()
    {
        QMessageBox::information(this, "连接提示", "网络异常断开");
    });

    // 读取数据准备好的信号
    connect(socket, &QTcpSocket::readyRead, this, &Player::server_reply_slot);

    // 初始化并启动定时器
    timer = new QTimer();
    timer->start(2000);

    // 定时器超时信号
    connect(timer, &QTimer::timeout, this, &Player::timeout_slot);
}

// Player类的析构函数
Player::~Player()
{
    delete ui;
}

// 服务器回复槽函数
void Player::server_reply_slot()
{
    QByteArray recvData;
    player_recv_data(recvData); // 接收数据

    QJsonObject obj = QJsonDocument::fromJson(recvData).object();

    QString cmd = obj["cmd"].toString();
    if (cmd == "info") // 信息更新
    {
        player_update_info(obj);

        if (flag)
        {
            player_get_music();
            flag = 0;
        }
    }
    else if (cmd == "upload_music") // 更新音乐
    {
        player_update_music(obj);
    }
    else if (cmd == "app_start_reply") // 开始播放回复
    {
        player_start_reply(obj);
    }
    else if (cmd == "app_suspend_reply") // 暂停播放回复
    {
        player_suspend_reply(obj);
    }
    else if (cmd == "app_continue_reply") // 继续播放回复
    {
        player_continue_reply(obj);
    }
    else if (cmd == "app_next_reply") // 下一曲回复
    {
        player_next_reply(obj);
    }
    else if (cmd == "app_prior_reply") // 上一曲回复
    {
        player_prior_reply(obj);
    }
    else if (cmd == "app_voice_up_reply" || cmd == "app_voice_down_reply") // 音量调整回复
    {
        player_voice_reply(obj);
    }
    else if (cmd == "app_circle_reply" || cmd == "app_sequence_reply") // 播放模式回复
    {
        player_mode_reply(obj);
    }
}

// 播放模式回复处理函数
void Player::player_mode_reply(QJsonObject &obj)
{
    QString result = obj["result"].toString();
    if (result == "offline")
    {
        ui->circleButton->setChecked(false);
        ui->sequenceButton->setChecked(true);
        QMessageBox::warning(this, "播放提示", "音箱下线");
    }
}

// 音量调整回复处理函数
void Player::player_voice_reply(QJsonObject &obj)
{
    QString result = obj["result"].toString();
    if (result == "offline")
    {
        QMessageBox::warning(this, "播放提示", "音箱离线");
    }
    else if (result == "success")
    {
        int volume = obj["voice"].toInt();

        QFont f("黑体", 26);
        ui->volumeLabel->setFont(f);
        ui->volumeLabel->setText(QString::number(volume));
    }
}

// 下一曲回复处理函数
void Player::player_next_reply(QJsonObject &obj)
{
    QString result = obj["result"].toString();
    if (result == "offline")
    {
        QMessageBox::warning(this, "播放提示", "音箱离线");
    }
    else if (result == "success")
    {
        QString music = obj["music"].toString();
        QFont f("黑体", 26);
        ui->musicLabel->setFont(f);
        ui->musicLabel->setText(music);
    }
}

// 上一曲回复处理函数
void Player::player_prior_reply(QJsonObject &obj)
{
    QString result = obj["result"].toString();
    if (result == "offline")
    {
        QMessageBox::warning(this, "播放提示", "音箱离线");
    }
    else if (result == "success")
    {
        QString music = obj["music"].toString();
        QFont f("黑体", 26);
        ui->musicLabel->setFont(f);
        ui->musicLabel->setText(music);
    }
}

// 暂停播放回复处理函数
void Player::player_suspend_reply(QJsonObject &obj)
{
    QString result = obj["result"].toString();
    if (result == "offline")
    {
        QMessageBox::warning(this, "播放提示", "音箱离线");
    }
    else if (result == "success")
    {
        suspend_flag = 1;

        QFont f("黑体", 26);
        ui->playButton->setFont(f);
        ui->playButton->setText("▷");
    }
}

// 继续播放回复处理函数
void Player::player_continue_reply(QJsonObject &obj)
{
    QString result = obj["result"].toString();
    if (result == "offline")
    {
        QMessageBox::warning(this, "播放提示", "音箱离线");
    }
    else if (result == "success")
    {
        suspend_flag = 0;

        QFont f("黑体", 26);
        ui->playButton->setFont(f);
        ui->playButton->setText("||");
    }
}

// 开始播放回复处理函数
void Player::player_start_reply(QJsonObject &obj)
{
    QString result = obj["result"].toString();
    if (result == "offline")
    {
        QMessageBox::warning(this, "播放提示", "音箱离线");
    }
    else if (result == "failure")
    {
        QMessageBox::warning(this, "播放提示", "音箱启动失败");
    }
    else if (result == "success")
    {
        start_flag = 1;

        QFont f("黑体", 26);
        ui->playButton->setFont(f);
        ui->playButton->setText("||");
    }
}

// 获取音乐信息
void Player::player_get_music()
{
    QJsonObject obj;
    obj.insert("cmd", "app_get_music");
    player_send_data(obj);
}

// 更新音乐信息
void Player::player_update_music(QJsonObject &obj)
{
    QJsonArray arr = obj["music"].toArray();

    QFont f("黑体", 18);
    ui->textEdit->setFont(f);
    QString result;
    for (int i = 0; i < arr.size(); i++)
    {
        result.append(arr.at(i).toString());
        result.append('\n');
    }

    // qDebug() << "服务器：" << result;
    ui->textEdit->setText(result);
}

// 更新设备信息
void Player::player_update_info(QJsonObject &obj)
{
    QString cur_music = obj["cur_music"].toString();
    QString dev_id = obj["deviceid"].toString();
    QString status = obj["status"].toString();
    int volume = obj["volume"].toInt();
    int mode = obj["mode"].toInt();

    QFont f("黑体", 26);

    ui->devidLabel->setFont(f);
    ui->devidLabel->setText(dev_id);

    ui->musicLabel->setFont(f);
    ui->musicLabel->setText(cur_music);

    ui->volumeLabel->setFont(f);
    ui->volumeLabel->setText(QString::number(volume));

    if (mode == SEQUENCE)
    {
        ui->sequenceButton->setChecked(true);
    }
    else if (mode == CIRCLE)
    {
        ui->circleButton->setChecked(true);
    }

    ui->playButton->setFont(f);
    if (status == "start")
    {
        ui->playButton->setText("||");
        start_flag = 1;
        suspend_flag = 0;
    }
    else if (status == "stop")
    {
        ui->playButton->setText("▷");
        start_flag = 0;
        suspend_flag = 0;
    }
    else if (status == "suspend")
    {
        ui->playButton->setText("▷");
        start_flag = 1;
        suspend_flag = 1;
    }
}

// 接收数据函数
void Player::player_recv_data(QByteArray &ba)
{
    char buf[1024] = {0};
    qint64 size = 0;

    // 读取数据长度
    while (true)
    {
        size += socket->read(buf + size, sizeof(int) - size);
        if (size >= 4

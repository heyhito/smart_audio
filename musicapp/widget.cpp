#include "widget.h"
#include "ui_widget.h"

// Widget类的构造函数
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this); // 设置UI

    // 创建一个新的QTcpSocket
    socket = new QTcpSocket;

    // 连接到主机
    socket->connectToHost(QHostAddress("8.134.62.10"), 8000);

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
    connect(socket, &QTcpSocket::readyRead, this, &Widget::server_reply_slot);
}

// Widget类的析构函数
Widget::~Widget()
{
    delete ui;
}

// 服务器回复槽函数
void Widget::server_reply_slot()
{
    QByteArray recvData;
    widget_recv_data(recvData); // 接收数据

    QJsonObject obj = QJsonDocument::fromJson(recvData).object();

    QString cmd = obj.value("cmd").toString();
    if (cmd == "app_register_reply") // 注册回复
    {
        app_register_handle(obj);
    }
    else if (cmd == "app_login_reply") // 登录回复
    {
        app_login_handle(obj);
    }
}

// 处理登录回复
void Widget::app_login_handle(QJsonObject &obj)
{
    QString result = obj.value("result").toString();

    if (result == "user_not_exist") // 用户不存在
    {
        QMessageBox::warning(this, "登录提示", "用户不存在，请先注册");
    }
    else if (result == "password_error") // 密码错误
    {
        QMessageBox::warning(this, "登录提示", "密码错误，请检查密码");
    }
    else if (result == "not_bind") // 未绑定设备
    {
        socket->disconnect(SIGNAL(connected()));
        socket->disconnect(SIGNAL(disconnected()));
        socket->disconnect(SIGNAL(readyRead()));

        //QMessageBox::information(this,"登录提示","登录成功，未绑定");
        Bind *m_bind = new Bind(socket, m_appid);

        this->hide();
        m_bind->show();
    }
    else if (result == "bind") // 已绑定设备
    {
        QString deviceid = obj.value("deviceid").toString();

        socket->disconnect(SIGNAL(connected()));
        socket->disconnect(SIGNAL(disconnected()));
        socket->disconnect(SIGNAL(readyRead()));

        //QMessageBox::information(this,"登录提示","登录成功，已经绑定");
        Player *m_player = new Player(socket, m_appid, deviceid);

        this->hide();
        m_player->show();
    }
}

// 处理注册回复
void Widget::app_register_handle(QJsonObject &obj)
{
    QString result = obj.value("result").toString();

    if (result == "success") // 注册成功
    {
        QMessageBox::information(this, "注册提示", "注册成功");
    }
    else if (result == "failure") // 注册失败
    {
        QMessageBox::warning(this, "注册提示", "注册失败");
    }
}

// 接收数据函数
void Widget::widget_recv_data(QByteArray &ba)
{
    char buf[1024] = {0};
    qint64 size = 0;

    // 读取数据长度
    while (true)
    {
        size += socket->read(buf + size, sizeof(int) - size);
        if (size >= 4)
            break;
    }

    int len = *(int *)buf;
    size = 0;
    memset(buf, 0, sizeof(buf));

    // 读取实际数据
    while (true)
    {
        size += socket->read(buf + size, len - size);
        if (size >= len)
        {
            break;
        }
    }
    ba.append(buf);
    qDebug() << "收到服务器发送的 " << len << "字节 " << ba;
}

// 注册按钮点击槽函数
void Widget::on_registerButton_clicked()
{
    QString appid = ui->appEdit->text();
    QString password = ui->passEdit->text();

    QJsonObject obj;
    obj.insert("cmd", "app_register");
    obj.insert("appid", appid);
    obj.insert("password", password);

    widget_send_data(obj);
}

// 发送数据函数
void Widget::widget_send_data(QJsonObject &obj)
{
    QByteArray sendData;
    QByteArray ba = QJsonDocument(obj).toJson();

    int size = ba.size();
    sendData.insert(0, (const char *)&size, 4);
    sendData.append(ba);

    socket->write(sendData);
}

// 登录按钮点击槽函数
void Widget::on_loginButton_clicked()
{
    QString appid = ui->appEdit->text();
    QString password = ui->passEdit->text();

    this->m_appid = appid;

    QJsonObject obj;
    obj.insert("cmd", "app_login");
    obj.insert("appid", appid);
    obj.insert("password", password);

    widget_send_data(obj);
}

#include "bind.h"
#include "ui_bind.h"

// Bind 类的构造函数
Bind::Bind(QTcpSocket *s, QString id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Bind)
{
    ui->setupUi(this);

    this->appid = id; // 设置应用ID
    this->socket = s; // 设置QTcpSocket对象

    // 连接成功信号
    connect(socket, &QTcpSocket::connected, [this]()
    {
        QMessageBox::information(this, "连接提示", "连接服务器成功");
    });

    // 断开连接信号
    connect(socket, &QTcpSocket::disconnected, [this]()
    {
        QMessageBox::warning(this, "连接提示", "网络异常断开");
    });

    // 读取数据准备好的信号
    connect(socket, &QTcpSocket::readyRead, this, &Bind::server_reply_slot);
}

// Bind 类的析构函数
Bind::~Bind()
{
    delete ui;
}

// 接收数据函数
void Bind::bind_recv_data(QByteArray &ba)
{
    char buf[1024] = {0};
    qint64 size = 0;

    // 读取数据长度
    while (true)
    {
        size += socket->read(buf + size, sizeof(int) - size);
        if (size >= sizeof(int))
            break;
    }

    int len = *(int *)buf;
    memset(buf, 0, sizeof(buf));
    size = 0;

    // 读取实际数据
    while (true)
    {
        size += socket->read(buf + size, len - size);
        if (size >= len)
            break;
    }

    ba.append(buf);

    qDebug() << "收到服务器数据 " << len << "字节" << ba;
}

// 服务器回复槽函数
void Bind::server_reply_slot()
{
    QByteArray recvData;
    bind_recv_data(recvData);

    QJsonObject obj = QJsonDocument::fromJson(recvData).object();

    QString cmd = obj["cmd"].toString();

    if (cmd == "app_bind_reply")
    {
        QString result = obj["result"].toString();
        if (result == "success")
        {
            // 绑定成功后跳转到控制界面

            socket->disconnect(SIGNAL(connected()));
            socket->disconnect(SIGNAL(disconnected()));
            socket->disconnect(SIGNAL(readyRead()));

            Player *m_player = new Player(socket, appid, m_deviceid);

            // 显示新的界面
            m_player->show();
            // 隐藏当前绑定界面
            this->hide();
        }
    }
}

// 发送数据函数
void Bind::bind_send_data(QJsonObject &obj)
{
    QByteArray sendData;
    QByteArray ba = QJsonDocument(obj).toJson();

    int size = ba.size();

    sendData.insert(0, (const char *)&size, 4);
    sendData.append(ba);

    socket->write(sendData);
}

// 绑定按钮点击槽函数
void Bind::on_bindButton_clicked()
{
    this->m_deviceid = ui->deviceEdit->text(); // 获取设备ID
    QJsonObject obj;
    obj.insert("cmd", "app_bind");
    obj.insert("appid", appid);
    obj.insert("deviceid", m_deviceid);

    bind_send_data(obj); // 发送绑定请求
}

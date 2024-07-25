#ifndef BIND_H
#define BIND_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <player.h>

namespace Ui {
class Bind;
}

class Bind : public QWidget
{
    Q_OBJECT

public:
    explicit Bind(QTcpSocket *,QString,QWidget *parent = 0);
    ~Bind();
private slots:

    void server_reply_slot();
    void on_bindButton_clicked();

private:

    void bind_send_data(QJsonObject &);
    void bind_recv_data(QByteArray &);

private:
    Ui::Bind *ui;
    QString appid;
    QTcpSocket *socket;
    QString m_deviceid;
};

#endif // BIND_H

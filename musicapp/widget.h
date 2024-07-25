#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <bind.h>
#include <player.h>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_registerButton_clicked();
    void server_reply_slot();

    void on_loginButton_clicked();

private:
    void widget_send_data(QJsonObject &);
    void widget_recv_data(QByteArray &);
    void app_register_handle(QJsonObject &);
    void app_login_handle(QJsonObject &);

private:
    Ui::Widget *ui;
    QTcpSocket *socket;
    QString m_appid;
};

#endif // WIDGET_H

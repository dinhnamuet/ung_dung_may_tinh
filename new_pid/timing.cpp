#include "timing.h"

Timing::Timing(QObject *parent)
    : QObject{parent}, m_timer("0"), m_speed("0"), socket(new QTcpSocket(this)), connect_status(false), m_direction("Wait ...!")
{
    connect(socket, &QTcpSocket::readyRead, this, &Timing::read_from_Server);
}

QString Timing::readTime()
{
    return m_timer;
}
void Timing::setTime(QString time)
{
    if(time == m_timer)
    {
        return;
    }
    m_timer = time;
    emit timeChanged(m_timer);
}

QString Timing::readSpeed()
{
    return m_speed;
}
void Timing::setSpeed(QString speed)
{
    if(m_speed == speed)
    {
        return;
    }
    m_speed = speed;
    emit speedChanged(m_speed);
}

QString Timing::readDirection()
{
    return m_direction;
}
void Timing::setDirection(QString direction)
{
    if(m_direction == direction)
    {
        return;
    }
    m_direction = direction;
    emit directionChanged(m_direction);
}

/* socket programming */
void Timing::connectToServer(const QString &serverAddress, QString port)
{
    int port_no = port.toInt();
    QObject::connect(socket, &QAbstractSocket::connected, [=]() {
        connect_status = true;
        emit connectionChanged(connect_status);
        //QString message = "connect success!";
        //socket->write(message.toUtf8());
        //socket->flush();
    });
    // Kết nối tới sự kiện error của socket để xử lý khi có lỗi xảy ra
    QObject::connect(socket, &QAbstractSocket::errorOccurred, [=]() {
        connect_status = false;
        emit connectionChanged(connect_status);
    });
    socket->connectToHost(serverAddress, port_no);
}
void Timing::read_from_Server()
{
    m_speed.clear();
    m_direction.clear();
    m_timer.clear();
    QString input;
    input = QString(socket->readAll());
    QStringList stringlist = input.split(" ");
    m_direction = stringlist[0];
    m_speed = stringlist[1];
    m_timer = stringlist[2];
    emit directionChanged(m_direction);
    emit speedChanged(m_speed);
    emit timeChanged(m_timer);
}
void Timing::send_to_Server(QString data)
{
    socket->write(data.toUtf8());
    socket->flush();
}
void Timing::exit_socket()
{
    QString message = "exit";
    socket->write(message.toUtf8());
    socket->flush();
}

bool Timing::read_connection()
{
    return connect_status;
}
void Timing::setConnection(bool connection)
{
    if(connect_status == connection)
    {
        return;
    }
    connect_status = connection;
    emit connectionChanged(connect_status);
}

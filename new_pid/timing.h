#ifndef TIMING_H
#define TIMING_H

#include <QObject>
#include <QTimer>
#include <QTcpSocket>
class Timing : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int time READ readTime WRITE setTime NOTIFY timeChanged FINAL)
    Q_PROPERTY(QString speed READ readSpeed WRITE setSpeed NOTIFY speedChanged FINAL)
    Q_PROPERTY(QString direction READ readDirection WRITE setDirection NOTIFY directionChanged FINAL)
    Q_PROPERTY(bool connection READ read_connection WRITE setConnection NOTIFY connectionChanged FINAL)
public:
    explicit Timing(QObject *parent = nullptr);
    Q_INVOKABLE QString readTime(void);
    Q_INVOKABLE void setTime(QString time);

    Q_INVOKABLE QString readSpeed(void);
    Q_INVOKABLE void setSpeed(QString speed);

    Q_INVOKABLE QString readDirection(void);
    Q_INVOKABLE void setDirection(QString direction);

    Q_INVOKABLE void connectToServer(const QString &serverAddress, QString port);
    Q_INVOKABLE void read_from_Server();
    Q_INVOKABLE void send_to_Server(QString data);
    Q_INVOKABLE void exit_socket();

    Q_INVOKABLE bool read_connection(void);
    Q_INVOKABLE void setConnection(bool connection);

signals:
    void timeChanged(QString time);
    void speedChanged(QString speed);
    void connectionChanged(bool connection);
    void directionChanged(QString direction);
private:
    QString m_timer;
    QString m_speed;
    QTcpSocket *socket;
    bool connect_status;
    QString m_direction;
};

#endif // TIMING_H

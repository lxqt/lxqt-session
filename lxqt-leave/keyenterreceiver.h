#ifndef KEYENTERRECEIVER_H
#define KEYENTERRECEIVER_H

#include <QObject>
#include <QKeyEvent>
#include <QToolButton>
#include <QPushButton>
class KeyEnterReceiver : public QObject
{
    Q_OBJECT
public:
    explicit KeyEnterReceiver(QObject *parent = 0);
protected:
    bool eventFilter(QObject* obj, QEvent* event);
signals:

public slots:
};

#endif // KEYENTERRECEIVER_H

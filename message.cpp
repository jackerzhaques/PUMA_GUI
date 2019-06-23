#include "message.h"
#include <QDebug>

void dump_message(Message m)
{
    qDebug() << "ID: " << m.ID;
    qDebug() << "DLC: " << m.DLC;
    qDebug() << "Data: ";
    for(int i = 0; i < m.DLC; i++){
        qDebug() << "\t" << m.pData[i];
    }
}

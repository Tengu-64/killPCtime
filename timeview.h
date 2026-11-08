#ifndef TIMEVIEW_H
#define TIMEVIEW_H
#include <QtTypes>
#include <QString>

struct Task
{
    QString msg; // информационное сообщение для ui
    bool find; // наличие скрипта
};

Task searchProc(); // поиск процесса
Task timeViewMsg(uint8_t h, uint8_t m); // вывод сообщения времени для ui
Task killProcess(); // удаление процесса

#endif // TIMEVIEW_H

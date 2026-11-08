#include "timeview.h"
#include <QDebug>
#include <QString>
#include <QProcess>
#include <QRegularExpression>
#include <QTextCodec>
#include <QTime>

static QString decodeOem(const QByteArray &data) { // поиск скрипта, возвращает сообщение для ui и результат поиска (bool)
    static QTextCodec *codec = QTextCodec::codecForName("CP866");
    return codec ? codec->toUnicode(data) : QString::fromLocal8Bit(data);
}

Task searchProc() {
    QProcess proc;
    Task task;
    proc.start("schtasks", {
        "/query", "/tn", "killPcTime",
        "/fo", "LIST", "/v"
    });

    if (!proc.waitForFinished(7000)) {
        proc.kill();
        proc.waitForFinished(1000);
        task.msg = "не удалось получить время";
        task.find = false;
        return task;
    }

    const QByteArray rawOut = proc.readAllStandardOutput();
    const QByteArray rawErr = proc.readAllStandardError();

    // qDebug().noquote() << "[searchProc] exit:" << proc.exitCode()
    //                    << "stdout:" << rawOut.size()
    //                    << "stderr:" << rawErr.size();

    if (proc.exitCode() != 0) {
        if(proc.exitCode() == 1) {
            task.msg = "время не назначено";
            task.find = false;
            return task;
        }
        const QString err   = decodeOem(rawErr).trimmed();
        const QString lower = err.toLower();

        qDebug() << "[searchProc] ошибка:" << err;

        if (lower.contains("не удается найти") ||
            lower.contains("cannot find") ||
            lower.contains("not found") ||
            lower.contains("не найдена"))
        {
            task.msg = "процесс не найден";
            task.find = false;
            return task;
        }
        task.msg = "err: " + (err.isEmpty() ? "неизвестная ошибка" : err);
        task.find = false;
        return task;
    }

    const QString output = decodeOem(rawOut);

    const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith("Next Run Time:", Qt::CaseInsensitive) ||
            trimmed.startsWith("Следующее время запуска:", Qt::CaseInsensitive) ||
            trimmed.startsWith("Время следующего запуска:", Qt::CaseInsensitive))
        {
            const int colon = trimmed.indexOf(':');
            task.msg = "выключение в " + trimmed.mid(colon + 1).trimmed();
            task.find = true;
            return task;
        }
    }
    task.msg = "время выключения не найдено";
    task.find = false;
    return task;
}

Task timeViewMsg(uint8_t h, uint8_t m) { // выполнение скрипта, возвращает статус-сообщение выполнения для ui

    Task task;

    if (h >= 24 || m >= 60) { // допустимые значения времени
        task.find = false;
        task.msg = "недопустимое время выключения";
        return task;
    }

    QString timeStr = QTime(h,m).toString("hh:mm");
    QProcess proc;
    proc.start("schtasks", {"/create", "/tn", "killPcTime", "/tr", "shutdown /s", "/sc", "daily", "/st", timeStr, "/f"});

    if(!proc.waitForFinished(10000) || (proc.exitCode() != 0) ) {
        task.find = false;
        task.msg = "ошибка назначения времени";
        return task;
    }

    task.find = true;
    task.msg = "выключение в " + timeStr;
    return task;
}

Task killProcess() {
    QProcess proc;
    Task task;
    proc.start("schtasks", {"/delete", "/tn", "killPcTime", "/f"});

    if(!proc.waitForFinished()) {
        task.msg = "задачу не удалось удалить";
        task.find = false;
        return task;
    }

    if(proc.exitCode() == 0) {
        task.find = true;
        task.msg = "задача успешно удалена";
    } else {
        task.find = false;
        task.msg = "задачу не удалось удалить";
    }

    return task;
}

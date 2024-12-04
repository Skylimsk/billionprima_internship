#ifndef THREADLOGGER_H
#define THREADLOGGER_H

#pragma once

#include <string>
#include <mutex>
#include <QDebug>

class ThreadLogger {
public:
    static void logThreadOperation(int threadId,
                                   const std::string& phase,
                                   const std::string& message,
                                   int current,
                                   int total,
                                   double elapsedMs) {
        std::lock_guard<std::mutex> lock(logMutex);
        QString logMessage = QString("[Thread %1] [%2] %3 (%4/%5) %.2f ms")
                                 .arg(threadId)
                                 .arg(QString::fromStdString(phase))
                                 .arg(QString::fromStdString(message))
                                 .arg(current)
                                 .arg(total)
                                 .arg(elapsedMs);
        qDebug() << logMessage;
    }

private:
    static std::mutex logMutex;
};

#endif // THREADLOGGER_H

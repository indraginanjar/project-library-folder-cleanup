#pragma once

#include "ApplicationController.h"
#include <QObject>
#include <QString>
#include <filesystem>

class DeleteWorker : public QObject {
    Q_OBJECT

public:
    explicit DeleteWorker(ApplicationController* controller);

public slots:
    void process();

signals:
    void progress(const QString& path, size_t completed, size_t total);
    void finished();
    void error(const QString& message);

private:
    ApplicationController* controller_;
};

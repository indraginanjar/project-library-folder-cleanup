#pragma once

#include "ApplicationController.h"
#include <QObject>
#include <QString>
#include <filesystem>

class ScanWorker : public QObject {
    Q_OBJECT

public:
    explicit ScanWorker(ApplicationController* controller,
                       const ApplicationController::OperationConfig& config);

public slots:
    void process();

signals:
    void progress(const QString& path, size_t processed, size_t total);
    void finished();
    void error(const QString& message);

private:
    ApplicationController* controller_;
    ApplicationController::OperationConfig config_;
};

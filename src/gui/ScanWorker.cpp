#include "ScanWorker.h"

ScanWorker::ScanWorker(ApplicationController* controller,
                       const ApplicationController::OperationConfig& config)
    : controller_(controller)
    , config_(config) {
}

void ScanWorker::process() {
    try {
        bool success = controller_->execute_scan(config_, 
            [this](const std::string& path, size_t processed, size_t total) {
                emit progress(QString::fromStdString(path), processed, total);
            });
        
        if (!success) {
            emit error("Scan operation failed. Check the log file for details.");
        }
        
        emit finished();
    } catch (const std::filesystem::filesystem_error& e) {
        QString error_msg = QString("Filesystem error during scan:\n\n%1\n\nPath: %2")
            .arg(e.what())
            .arg(QString::fromStdString(e.path1().string()));
        emit error(error_msg);
        emit finished();
    } catch (const std::exception& e) {
        QString error_msg = QString("Error during scan:\n\n%1").arg(e.what());
        emit error(error_msg);
        emit finished();
    } catch (...) {
        emit error("Unknown error occurred during scan.");
        emit finished();
    }
}

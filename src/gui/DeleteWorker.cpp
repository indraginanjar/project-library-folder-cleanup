#include "DeleteWorker.h"

DeleteWorker::DeleteWorker(ApplicationController* controller)
    : controller_(controller) {
}

void DeleteWorker::process() {
    try {
        bool success = controller_->execute_deletion(
            [this](const std::filesystem::path& path, size_t completed, size_t total) {
                emit progress(QString::fromStdString(path.string()), completed, total);
            });
        
        if (!success) {
            emit error("Deletion operation failed. Check the log file for details.");
        }
        
        emit finished();
    } catch (const std::filesystem::filesystem_error& e) {
        QString error_msg = QString("Filesystem error during deletion:\n\n%1\n\nPath: %2")
            .arg(e.what())
            .arg(QString::fromStdString(e.path1().string()));
        emit error(error_msg);
        emit finished();
    } catch (const std::exception& e) {
        QString error_msg = QString("Error during deletion:\n\n%1").arg(e.what());
        emit error(error_msg);
        emit finished();
    } catch (...) {
        emit error("Unknown error occurred during deletion.");
        emit finished();
    }
}

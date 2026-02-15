#pragma once

#include "ApplicationController.h"
#include <QMainWindow>
#include <QThread>
#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_browse_button_clicked();
    void on_scan_button_clicked();
    void on_delete_button_clicked();
    void on_configure_folders_clicked();
    void on_cancel_button_clicked();
    void on_actionAbout_triggered();
    void on_scan_progress(const QString& path, size_t processed, size_t total);
    void on_scan_complete();
    void on_scan_error(const QString& message);
    void on_deletion_progress(const QString& path, size_t completed, size_t total);
    void on_deletion_complete();
    void on_deletion_error(const QString& message);

private:
    Ui::MainWindow* ui_;
    ApplicationController controller_;
    QThread* worker_thread_;
    
    void update_results_table();
    bool confirm_deletion();
    void cleanup_worker_thread();
};

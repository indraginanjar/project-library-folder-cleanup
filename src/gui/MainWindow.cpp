#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ConfigDialog.h"
#include "ConfigurationManager.h"
#include "ScanWorker.h"
#include "DeleteWorker.h"
#include "SafetyValidator.h"
#include "Version.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
    , worker_thread_(nullptr) {
    
    ui_->setupUi(this);
    
    // Set window title with version
    setWindowTitle(QString::fromStdString(Version::get_version_string()));
    
    // Connect UI signals to slots
    connect(ui_->browseButton, &QPushButton::clicked, 
            this, &MainWindow::on_browse_button_clicked);
    connect(ui_->scanButton, &QPushButton::clicked, 
            this, &MainWindow::on_scan_button_clicked);
    connect(ui_->deleteButton, &QPushButton::clicked, 
            this, &MainWindow::on_delete_button_clicked);
    connect(ui_->configureFoldersButton, &QPushButton::clicked, 
            this, &MainWindow::on_configure_folders_clicked);
    connect(ui_->cancelButton, &QPushButton::clicked,
            this, &MainWindow::on_cancel_button_clicked);
    
    // Setup results table
    ui_->resultsTable->setColumnCount(3);
    ui_->resultsTable->setHorizontalHeaderLabels({"Path", "Size (MB)", "Selected"});
    ui_->resultsTable->horizontalHeader()->setStretchLastSection(false);
    ui_->resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui_->resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui_->resultsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui_->resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Initially disable delete button
    ui_->deleteButton->setEnabled(false);
    
    // Initially disable cancel button
    ui_->cancelButton->setEnabled(false);
    
    // Reset progress bar
    ui_->progressBar->setValue(0);
}

MainWindow::~MainWindow() {
    // Cancel any ongoing operations
    controller_.cancel_operation();
    
    // Clean up worker thread
    cleanup_worker_thread();
    
    // Clean up UI
    delete ui_;
}

void MainWindow::on_browse_button_clicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Base Directory",
        ui_->baseDirectoryLineEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!dir.isEmpty()) {
        ui_->baseDirectoryLineEdit->setText(dir);
    }
}

void MainWindow::on_scan_button_clicked() {
    // Get base directory
    QString base_dir = ui_->baseDirectoryLineEdit->text();
    if (base_dir.isEmpty()) {
        QMessageBox::warning(this, "No Directory", 
                           "Please select a base directory first.");
        return;
    }
    
    // Validate base directory
    std::filesystem::path base_path(base_dir.toStdString());
    auto validation = SafetyValidator::validate_base_directory(base_path);
    
    if (validation == SafetyValidator::ValidationResult::SystemDirectory) {
        QMessageBox::critical(this, "System Directory", 
                            "Cannot scan system directories. Please select a different directory.");
        return;
    }
    
    if (validation == SafetyValidator::ValidationResult::InvalidPath) {
        QMessageBox::warning(this, "Invalid Directory", 
                           "The selected directory does not exist or is not accessible.");
        return;
    }
    
    // Disable UI during scan
    ui_->scanButton->setEnabled(false);
    ui_->deleteButton->setEnabled(false);
    ui_->browseButton->setEnabled(false);
    ui_->configureFoldersButton->setEnabled(false);
    ui_->cancelButton->setEnabled(true);
    ui_->statusLabel->setText("Scanning...");
    ui_->progressBar->setValue(0);
    
    // Clear previous results
    ui_->resultsTable->setRowCount(0);
    
    // Setup operation config
    ApplicationController::OperationConfig config;
    config.base_directory = base_path;
    config.target_folders = ConfigurationManager::instance().get_target_folders();
    config.deletion_mode = FolderDeleter::Mode::DryRun;
    config.require_confirmation = true;
    
    // Create worker thread
    cleanup_worker_thread();
    worker_thread_ = new QThread(this);
    ScanWorker* worker = new ScanWorker(&controller_, config);
    worker->moveToThread(worker_thread_);
    
    // Connect signals
    connect(worker_thread_, &QThread::started, worker, &ScanWorker::process);
    connect(worker, &ScanWorker::progress, this, &MainWindow::on_scan_progress);
    connect(worker, &ScanWorker::finished, this, &MainWindow::on_scan_complete);
    connect(worker, &ScanWorker::error, this, &MainWindow::on_scan_error);
    connect(worker, &ScanWorker::finished, worker_thread_, &QThread::quit);
    connect(worker, &ScanWorker::finished, worker, &ScanWorker::deleteLater);
    connect(worker_thread_, &QThread::finished, worker_thread_, &QThread::deleteLater);
    
    // Start worker
    worker_thread_->start();
}

void MainWindow::on_delete_button_clicked() {
    // Confirm deletion
    if (!confirm_deletion()) {
        return;
    }
    
    // Disable UI during deletion
    ui_->scanButton->setEnabled(false);
    ui_->deleteButton->setEnabled(false);
    ui_->browseButton->setEnabled(false);
    ui_->configureFoldersButton->setEnabled(false);
    ui_->cancelButton->setEnabled(true);
    ui_->statusLabel->setText("Deleting...");
    ui_->progressBar->setValue(0);
    
    // Create worker thread
    cleanup_worker_thread();
    worker_thread_ = new QThread(this);
    DeleteWorker* worker = new DeleteWorker(&controller_);
    worker->moveToThread(worker_thread_);
    
    // Connect signals
    connect(worker_thread_, &QThread::started, worker, &DeleteWorker::process);
    connect(worker, &DeleteWorker::progress, this, &MainWindow::on_deletion_progress);
    connect(worker, &DeleteWorker::finished, this, &MainWindow::on_deletion_complete);
    connect(worker, &DeleteWorker::error, this, &MainWindow::on_deletion_error);
    connect(worker, &DeleteWorker::finished, worker_thread_, &QThread::quit);
    connect(worker, &DeleteWorker::finished, worker, &DeleteWorker::deleteLater);
    connect(worker_thread_, &QThread::finished, worker_thread_, &QThread::deleteLater);
    
    // Start worker
    worker_thread_->start();
}

void MainWindow::on_configure_folders_clicked() {
    ConfigDialog dialog(this);
    dialog.set_folder_names(ConfigurationManager::instance().get_target_folders());
    
    if (dialog.exec() == QDialog::Accepted) {
        auto folders = dialog.get_folder_names();
        ConfigurationManager::instance().set_target_folders(folders);
        ui_->statusLabel->setText("Configuration updated");
    }
}

void MainWindow::on_cancel_button_clicked() {
    // Cancel the current operation
    controller_.cancel_operation();
    
    // Disable cancel button
    ui_->cancelButton->setEnabled(false);
    ui_->statusLabel->setText("Cancelling...");
}

void MainWindow::on_actionAbout_triggered() {
    QString about_text = QString::fromStdString(Version::get_version_string()) + 
                        "\n\nA utility to scan and clean up library folders "
                        "(node_modules, venv, etc.) to reclaim disk space.\n\n"
                        "Features:\n"
                        "• Recursive directory scanning\n"
                        "• Configurable target folders\n"
                        "• Safety validation\n"
                        "• Dry-run mode\n"
                        "• Progress tracking";
    
    QMessageBox::about(this, "About", about_text);
}

void MainWindow::on_scan_progress(const QString& path, size_t processed, size_t total) {
    if (total > 0) {
        int percentage = static_cast<int>((processed * 100) / total);
        ui_->progressBar->setValue(percentage);
    }
    ui_->statusLabel->setText(QString("Scanning: %1").arg(path));
}

void MainWindow::on_scan_complete() {
    // Re-enable UI
    ui_->scanButton->setEnabled(true);
    ui_->browseButton->setEnabled(true);
    ui_->configureFoldersButton->setEnabled(true);
    ui_->cancelButton->setEnabled(false);
    ui_->progressBar->setValue(100);
    
    // Update results table
    update_results_table();
    
    // Get scan results
    const auto& result = controller_.get_scan_result();
    
    // Display summary
    double total_mb = result.total_size / (1024.0 * 1024.0);
    QString summary = QString("Scan complete: Found %1 folder(s), Total size: %2 MB")
                        .arg(result.found_folders.size())
                        .arg(total_mb, 0, 'f', 2);
    
    if (!result.errors.empty()) {
        summary += QString(" (%1 error(s))").arg(result.errors.size());
    }
    
    ui_->statusLabel->setText(summary);
    
    // Enable delete button if folders found
    ui_->deleteButton->setEnabled(!result.found_folders.empty());
    
    worker_thread_ = nullptr;
}

void MainWindow::on_scan_error(const QString& message) {
    QMessageBox::critical(this, "Scan Error", message);
    
    // Re-enable UI
    ui_->scanButton->setEnabled(true);
    ui_->browseButton->setEnabled(true);
    ui_->configureFoldersButton->setEnabled(true);
    ui_->cancelButton->setEnabled(false);
    ui_->statusLabel->setText("Scan failed");
    ui_->progressBar->setValue(0);
    
    worker_thread_ = nullptr;
}

void MainWindow::on_deletion_progress(const QString& path, size_t completed, size_t total) {
    if (total > 0) {
        int percentage = static_cast<int>((completed * 100) / total);
        ui_->progressBar->setValue(percentage);
    }
    ui_->statusLabel->setText(QString("Deleting: %1").arg(path));
}

void MainWindow::on_deletion_complete() {
    // Re-enable UI
    ui_->scanButton->setEnabled(true);
    ui_->browseButton->setEnabled(true);
    ui_->configureFoldersButton->setEnabled(true);
    ui_->deleteButton->setEnabled(false);
    ui_->cancelButton->setEnabled(false);
    ui_->progressBar->setValue(100);
    
    // Get deletion results
    const auto& result = controller_.get_deletion_result();
    
    // Display summary
    double space_mb = result.space_reclaimed / (1024.0 * 1024.0);
    QString summary = QString("Deletion complete: Removed %1 folder(s), Reclaimed %2 MB")
                        .arg(result.folders_deleted)
                        .arg(space_mb, 0, 'f', 2);
    
    if (!result.errors.empty()) {
        summary += QString(" (%1 error(s))").arg(result.errors.size());
    }
    
    ui_->statusLabel->setText(summary);
    
    // Show completion dialog
    QMessageBox::information(this, "Deletion Complete", summary);
    
    // Clear results table
    ui_->resultsTable->setRowCount(0);
    
    worker_thread_ = nullptr;
}

void MainWindow::on_deletion_error(const QString& message) {
    QMessageBox::critical(this, "Deletion Error", message);
    
    // Re-enable UI
    ui_->scanButton->setEnabled(true);
    ui_->browseButton->setEnabled(true);
    ui_->configureFoldersButton->setEnabled(true);
    ui_->cancelButton->setEnabled(false);
    ui_->statusLabel->setText("Deletion failed");
    ui_->progressBar->setValue(0);
    
    worker_thread_ = nullptr;
}

void MainWindow::update_results_table() {
    const auto& result = controller_.get_scan_result();
    
    ui_->resultsTable->setRowCount(static_cast<int>(result.found_folders.size()));
    
    for (size_t i = 0; i < result.found_folders.size(); ++i) {
        // Path column
        QTableWidgetItem* path_item = new QTableWidgetItem(
            QString::fromStdString(result.found_folders[i].string())
        );
        path_item->setFlags(path_item->flags() & ~Qt::ItemIsEditable);
        ui_->resultsTable->setItem(static_cast<int>(i), 0, path_item);
        
        // Size column (calculate individual folder size)
        // Note: For now, we'll show "N/A" as individual sizes aren't stored
        QTableWidgetItem* size_item = new QTableWidgetItem("N/A");
        size_item->setFlags(size_item->flags() & ~Qt::ItemIsEditable);
        ui_->resultsTable->setItem(static_cast<int>(i), 1, size_item);
        
        // Selected column (checkbox)
        QTableWidgetItem* selected_item = new QTableWidgetItem();
        selected_item->setCheckState(Qt::Checked);
        selected_item->setFlags(selected_item->flags() & ~Qt::ItemIsEditable);
        ui_->resultsTable->setItem(static_cast<int>(i), 2, selected_item);
    }
}

bool MainWindow::confirm_deletion() {
    const auto& result = controller_.get_scan_result();
    
    if (result.found_folders.empty()) {
        QMessageBox::information(this, "No Folders", 
                               "No folders to delete.");
        return false;
    }
    
    double total_mb = result.total_size / (1024.0 * 1024.0);
    QString message = QString("Are you sure you want to delete %1 folder(s)?\n\n"
                            "Total size: %2 MB\n\n"
                            "This action cannot be undone!")
                        .arg(result.found_folders.size())
                        .arg(total_mb, 0, 'f', 2);
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Deletion",
        message,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    return reply == QMessageBox::Yes;
}

void MainWindow::cleanup_worker_thread() {
    if (worker_thread_ && worker_thread_->isRunning()) {
        worker_thread_->quit();
        worker_thread_->wait();
    }
}

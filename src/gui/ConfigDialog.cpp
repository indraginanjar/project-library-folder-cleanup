#include "ConfigDialog.h"
#include "ui_ConfigDialog.h"
#include "ConfigurationManager.h"
#include <QInputDialog>
#include <QMessageBox>

ConfigDialog::ConfigDialog(QWidget* parent)
    : QDialog(parent)
    , ui_(new Ui::ConfigDialog) {
    
    ui_->setupUi(this);
    
    // Connect button signals to slots
    connect(ui_->addButton, &QPushButton::clicked, this, &ConfigDialog::on_add_button_clicked);
    connect(ui_->removeButton, &QPushButton::clicked, this, &ConfigDialog::on_remove_button_clicked);
    connect(ui_->resetButton, &QPushButton::clicked, this, &ConfigDialog::on_reset_button_clicked);
    connect(ui_->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui_->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ConfigDialog::~ConfigDialog() {
    delete ui_;
}

std::vector<std::string> ConfigDialog::get_folder_names() const {
    std::vector<std::string> folders;
    
    for (int i = 0; i < ui_->folderListWidget->count(); ++i) {
        QListWidgetItem* item = ui_->folderListWidget->item(i);
        folders.push_back(item->text().toStdString());
    }
    
    return folders;
}

void ConfigDialog::set_folder_names(const std::vector<std::string>& folders) {
    ui_->folderListWidget->clear();
    
    for (const auto& folder : folders) {
        ui_->folderListWidget->addItem(QString::fromStdString(folder));
    }
}

void ConfigDialog::on_add_button_clicked() {
    bool ok;
    QString text = QInputDialog::getText(this, 
                                         tr("Add Folder Name"),
                                         tr("Enter folder name:"),
                                         QLineEdit::Normal,
                                         "",
                                         &ok);
    
    if (ok && !text.isEmpty()) {
        std::string folder_name = text.toStdString();
        
        if (validate_folder_name(folder_name)) {
            // Check for duplicates
            for (int i = 0; i < ui_->folderListWidget->count(); ++i) {
                if (ui_->folderListWidget->item(i)->text() == text) {
                    QMessageBox::warning(this, 
                                       tr("Duplicate Entry"),
                                       tr("This folder name already exists in the list."));
                    return;
                }
            }
            
            ui_->folderListWidget->addItem(text);
        } else {
            QMessageBox::warning(this, 
                               tr("Invalid Folder Name"),
                               tr("Folder name cannot contain path separators (/ or \\) or invalid characters (< > : \" | ? *)."));
        }
    }
}

void ConfigDialog::on_remove_button_clicked() {
    QListWidgetItem* current_item = ui_->folderListWidget->currentItem();
    
    if (current_item) {
        delete ui_->folderListWidget->takeItem(ui_->folderListWidget->row(current_item));
    } else {
        QMessageBox::information(this, 
                               tr("No Selection"),
                               tr("Please select a folder name to remove."));
    }
}

void ConfigDialog::on_reset_button_clicked() {
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                             tr("Reset to Defaults"),
                                                             tr("Are you sure you want to reset to default folder names?"),
                                                             QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        auto defaults = ConfigurationManager::get_default_folders();
        set_folder_names(defaults);
    }
}

void ConfigDialog::update_list() {
    // This method can be used to refresh the list if needed
    // Currently not needed as we update directly in other methods
}

bool ConfigDialog::validate_folder_name(const std::string& name) {
    if (name.empty()) {
        return false;
    }
    
    // Check for path separators
    if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos) {
        return false;
    }
    
    // Check for invalid characters
    const std::string invalid_chars = "<>:\"|?*";
    if (name.find_first_of(invalid_chars) != std::string::npos) {
        return false;
    }
    
    return true;
}

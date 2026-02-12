#pragma once

#include <QDialog>
#include <vector>
#include <string>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget* parent = nullptr);
    ~ConfigDialog();

    std::vector<std::string> get_folder_names() const;
    void set_folder_names(const std::vector<std::string>& folders);

private slots:
    void on_add_button_clicked();
    void on_remove_button_clicked();
    void on_reset_button_clicked();

private:
    Ui::ConfigDialog* ui_;
    
    void update_list();
    bool validate_folder_name(const std::string& name);
};

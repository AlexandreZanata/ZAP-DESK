#pragma once

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QSpinBox;

namespace components {

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    void loadFromConfig();
    void saveToConfig();

private:
    QLineEdit* m_zapHome{};
    QSpinBox* m_zapPort{};
    QLineEdit* m_resultsDir{};
    QLineEdit* m_reconnerDir{};
    QLineEdit* m_zapApiKey{};
    QCheckBox* m_zapDevMode{};
    QSpinBox* m_reconRateLimit{};
    QCheckBox* m_crtOverlay{};
};

}  // namespace components

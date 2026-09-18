#pragma once

#include "isb/hub/hub.hpp"

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTimer>
#include <memory>

namespace isb::hub { class Provider; }

namespace isb::control_center {

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(std::unique_ptr<hub::Provider> provider, QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onRefresh();
    void onNavigate(int index);
    void onProfileChanged(const QString& profile);
    void onReviewPlan();
    void onApplyProfile();
    void onOptimize();
    void onAppearancePresetChanged(int index);

private:
    void buildUI();
    void buildSidebar();
    void buildHeader();
    void buildPages();
    QWidget* buildGPUPage();
    QWidget* buildTuningPage();
    QWidget* buildOptimizationPage();
    QWidget* buildGraphicsPage();
    QWidget* buildAppearancePage();
    
    void updateHeader();
    void updateGPUPage();
    void updateTuningPage();
    void refreshTelemetry();
    
    QString formatCapabilityState(hub::ControlState state) const;
    QString formatCapabilityState(cal::CapabilityState state) const;
    QString formatValueOrUnknown(const std::string& value) const;
    QString getProviderBadgeText() const;

    std::unique_ptr<hub::Provider> provider_;
    std::unique_ptr<hub::Hub> hub_;
    
    // UI Components
    QListWidget* sidebar_ = nullptr;
    QStackedWidget* pages_ = nullptr;
    QLabel* headerLabel_ = nullptr;
    QLabel* providerBadge_ = nullptr;
    QLabel* gpuStatusLabel_ = nullptr;
    
    // GPU Page widgets
    QLabel* gpuModelLabel_ = nullptr;
    QLabel* gpuArchitectureLabel_ = nullptr;
    QLabel* computeCapabilityLabel_ = nullptr;
    QLabel* tensorCoresLabel_ = nullptr;
    QLabel* rtCoresLabel_ = nullptr;
    QLabel* hbm2Label_ = nullptr;
    QLabel* eccLabel_ = nullptr;
    QLabel* nvlinkLabel_ = nullptr;
    QLabel* migLabel_ = nullptr;
    QLabel* displayOutputsLabel_ = nullptr;
    QLabel* opticalFlowLabel_ = nullptr;
    
    // Telemetry widgets
    QLabel* temperatureLabel_ = nullptr;
    QLabel* gpuUtilLabel_ = nullptr;
    QLabel* memoryUtilLabel_ = nullptr;
    QLabel* powerLabel_ = nullptr;
    QLabel* powerLimitLabel_ = nullptr;
    QLabel* gpuClockLabel_ = nullptr;
    QLabel* memoryClockLabel_ = nullptr;
    QLabel* vramUsedLabel_ = nullptr;
    QLabel* vramTotalLabel_ = nullptr;
    QLabel* perfStateLabel_ = nullptr;
    QLabel* pcieLabel_ = nullptr;
    QLabel* processCountLabel_ = nullptr;
    
    // Tuning widgets
    QComboBox* profileSelector_ = nullptr;
    QLabel* profileDescriptionLabel_ = nullptr;
    QPushButton* reviewPlanButton_ = nullptr;
    QPushButton* applyProfileButton_ = nullptr;
    
    // Appearance
    QComboBox* appearancePreset_ = nullptr;
    
    QTimer* refreshTimer_ = nullptr;
    
    std::string currentProfile_ = "Balanced";
};

} // namespace isb::control_center

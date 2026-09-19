#include "mainwindow.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>
#include <QSpacerItem>

#include <optional>
#include "isb/common/capability_state.hpp"

namespace isb::control_center {

namespace {

QWidget* createPanel(const QString& title) {
    auto* box = new QWidget;
    auto* layout = new QVBoxLayout(box);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(8);
    auto* heading = new QLabel(title);
    heading->setObjectName("panelHeading");
    layout->addWidget(heading);
    return box;
}

QWidget* createSection(const QString& title) {
    auto* section = new QFrame;
    section->setObjectName("section");
    auto* layout = new QVBoxLayout(section);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(6);
    auto* heading = new QLabel(title);
    heading->setObjectName("sectionHeading");
    layout->addWidget(heading);
    return section;
}

QVBoxLayout* getSectionLayout(QWidget* section) {
    return qobject_cast<QVBoxLayout*>(section->layout());
}

QString formatOptionalInt(const std::optional<int>& value, const QString& suffix = {}) {
    if (!value.has_value()) {
        return "Unknown";
    }
    return QString::number(*value) + suffix;
}

void addFormRow(QLayout* layout, const QString& label, const QString& value) {
    auto* row = new QHBoxLayout;
    row->setSpacing(8);
    auto* nameLabel = new QLabel(label);
    nameLabel->setFixedWidth(140);
    nameLabel->setObjectName("fieldName");
    auto* valueLabel = new QLabel(value);
    valueLabel->setObjectName("fieldValue");
    valueLabel->setWordWrap(true);
    row->addWidget(nameLabel);
    row->addWidget(valueLabel, 1);
    qobject_cast<QVBoxLayout*>(layout)->addLayout(row);
}

} // namespace

MainWindow::MainWindow(std::unique_ptr<hub::Provider> provider, QWidget* parent)
    : QMainWindow(parent)
    , provider_(std::move(provider))
    , hub_(std::make_unique<hub::Hub>(*provider_)) {
    
    setWindowTitle("ISB V100 Control Center");
    resize(1200, 750);
    setMinimumSize(900, 600);
    
    buildUI();
    
    refreshTimer_ = new QTimer(this);
    connect(refreshTimer_, &QTimer::timeout, this, &MainWindow::onRefresh);
    refreshTimer_->start(2000);
    
    onRefresh();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUI() {
    auto* central = new QWidget;
    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    buildSidebar();
    buildHeader();
    buildPages();
    
    auto* rightPanel = new QWidget;
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addWidget(headerLabel_->parentWidget());
    rightLayout->addWidget(pages_);
    
    mainLayout->addWidget(sidebar_);
    mainLayout->addWidget(rightPanel);
    
    setCentralWidget(central);
}

void MainWindow::buildSidebar() {
    sidebar_ = new QListWidget;
    sidebar_->setFixedWidth(160);
    sidebar_->setObjectName("sidebar");
    sidebar_->addItems({"GPU", "TUNING", "OPTIMIZATION", "GRAPHICS", "APPEARANCE"});
    sidebar_->setCurrentRow(0);
    sidebar_->setSpacing(2);
    connect(sidebar_, &QListWidget::currentRowChanged, this, &MainWindow::onNavigate);
}

void MainWindow::buildHeader() {
    auto* headerWidget = new QWidget;
    headerWidget->setObjectName("header");
    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 10, 16, 10);
    headerLayout->setSpacing(16);
    
    auto* titleLabel = new QLabel("ISB V100 Control Center");
    titleLabel->setObjectName("headerTitle");
    
    headerLabel_ = new QLabel("");
    headerLabel_->setObjectName("headerInfo");
    headerLabel_->setWordWrap(false);
    
    providerBadge_ = new QLabel("");
    providerBadge_->setObjectName("providerBadge");
    providerBadge_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    gpuStatusLabel_ = new QLabel("");
    gpuStatusLabel_->setObjectName("gpuStatus");
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(headerLabel_, 1);
    headerLayout->addWidget(gpuStatusLabel_);
    headerLayout->addWidget(providerBadge_);
}

void MainWindow::buildPages() {
    pages_ = new QStackedWidget;
    pages_->setObjectName("pages");
    pages_->addWidget(buildGPUPage());
    pages_->addWidget(buildTuningPage());
    pages_->addWidget(buildOptimizationPage());
    pages_->addWidget(buildGraphicsPage());
    pages_->addWidget(buildAppearancePage());
}

QWidget* MainWindow::buildGPUPage() {
    auto* page = new QWidget;
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(page);
    
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(16);
    
    auto* hardwareSection = createSection("Hardware");
    auto* hwLayout = hardwareSection->layout();
    
    gpuModelLabel_ = new QLabel("Unknown");
    gpuArchitectureLabel_ = new QLabel("Unknown");
    computeCapabilityLabel_ = new QLabel("Unknown");
    tensorCoresLabel_ = new QLabel("Unknown");
    rtCoresLabel_ = new QLabel("Unknown");
    hbm2Label_ = new QLabel("Unknown");
    eccLabel_ = new QLabel("Unknown");
    nvlinkLabel_ = new QLabel("Unknown");
    migLabel_ = new QLabel("Unknown");
    displayOutputsLabel_ = new QLabel("Unknown");
    opticalFlowLabel_ = new QLabel("Unknown");
    
    addFormRow(hwLayout, "Model", "");
    addFormRow(hwLayout, "Architecture", "");
    addFormRow(hwLayout, "Compute Capability", "");
    addFormRow(hwLayout, "Tensor Cores", "");
    addFormRow(hwLayout, "RT Cores", "");
    addFormRow(hwLayout, "HBM2", "");
    addFormRow(hwLayout, "ECC", "");
    addFormRow(hwLayout, "NVLink", "");
    addFormRow(hwLayout, "MIG", "");
    addFormRow(hwLayout, "Display Outputs", "");
    addFormRow(hwLayout, "Optical Flow", "");
    
    layout->addWidget(hardwareSection);
    
    auto* driverSection = createSection("Driver / Runtime");
    auto* drvLayout = driverSection->layout();
    addFormRow(drvLayout, "NVIDIA Driver", "Unknown");
    addFormRow(drvLayout, "CUDA", "Unknown");
    addFormRow(drvLayout, "NVML", "Unknown");
    addFormRow(drvLayout, "Vulkan", "Unknown");
    addFormRow(drvLayout, "OpenGL", "Unknown");
    layout->addWidget(driverSection);
    
    auto* telemetrySection = createSection("Live State");
    auto* telLayout = telemetrySection->layout();
    
    temperatureLabel_ = new QLabel("Unknown");
    gpuUtilLabel_ = new QLabel("Unknown");
    memoryUtilLabel_ = new QLabel("Unknown");
    powerLabel_ = new QLabel("Unknown");
    powerLimitLabel_ = new QLabel("Unknown");
    gpuClockLabel_ = new QLabel("Unknown");
    memoryClockLabel_ = new QLabel("Unknown");
    vramUsedLabel_ = new QLabel("Unknown");
    vramTotalLabel_ = new QLabel("Unknown");
    perfStateLabel_ = new QLabel("Unknown");
    pcieLabel_ = new QLabel("Unknown");
    processCountLabel_ = new QLabel("Unknown");
    
    addFormRow(telLayout, "Temperature", "");
    addFormRow(telLayout, "GPU Utilization", "");
    addFormRow(telLayout, "Memory Utilization", "");
    addFormRow(telLayout, "Power", "");
    addFormRow(telLayout, "Power Limit", "");
    addFormRow(telLayout, "GPU Clock", "");
    addFormRow(telLayout, "Memory Clock", "");
    addFormRow(telLayout, "VRAM Used", "");
    addFormRow(telLayout, "VRAM Total", "");
    addFormRow(telLayout, "Performance State", "");
    addFormRow(telLayout, "PCIe", "");
    addFormRow(telLayout, "Process Count", "");
    
    layout->addWidget(telemetrySection);
    layout->addStretch();
    
    return scroll;
}

QWidget* MainWindow::buildTuningPage() {
    auto* page = new QWidget;
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(page);
    
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(16);
    
    auto* profileSection = createSection("Performance Profile");
    
    auto* profileRow = new QHBoxLayout;
    profileRow->setSpacing(12);
    
    profileSelector_ = new QComboBox;
    profileSelector_->setFixedWidth(200);
    profileSelector_->addItems({
        "Balanced", "Power Saving", "Gaming", 
        "Maximum Performance", "Compute", "AI / Tensor", "Custom"
    });
    connect(profileSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { onProfileChanged(profileSelector_->currentText()); });
    
    profileDescriptionLabel_ = new QLabel("Normal performance / power balance.");
    profileDescriptionLabel_->setWordWrap(true);
    profileDescriptionLabel_->setObjectName("profileDescription");
    
    reviewPlanButton_ = new QPushButton("Review Plan");
    connect(reviewPlanButton_, &QPushButton::clicked, this, &MainWindow::onReviewPlan);
    
    applyProfileButton_ = new QPushButton("Apply Profile");
    connect(applyProfileButton_, &QPushButton::clicked, this, &MainWindow::onApplyProfile);
    
    profileRow->addWidget(new QLabel("Profile:"));
    profileRow->addWidget(profileSelector_);
    profileRow->addWidget(reviewPlanButton_);
    profileRow->addWidget(applyProfileButton_);
    profileRow->addStretch();
    
    auto* profLayout = getSectionLayout(profileSection);
    profLayout->addLayout(profileRow);
    profLayout->addWidget(profileDescriptionLabel_);
    
    layout->addWidget(profileSection);
    
    auto* controlsSection = createSection("Available Controls");
    auto* ctrlLayout = controlsSection->layout();
    
    auto* table = new QTableWidget;
    table->setObjectName("controlsTable");
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"Control", "State", "Details"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    
    ctrlLayout->addWidget(table);
    layout->addWidget(controlsSection);
    
    auto* infoLabel = new QLabel(
        "Profiles are policy templates. Apply requires backend support. "
        "Read-only backends will show 'Unsupported' for mutation operations.");
    infoLabel->setWordWrap(true);
    infoLabel->setObjectName("infoText");
    layout->addWidget(infoLabel);
    
    layout->addStretch();
    
    return scroll;
}

QWidget* MainWindow::buildOptimizationPage() {
    auto* page = new QWidget;
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(page);
    
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(16);
    
    auto* title = new QLabel("Optimize V100");
    title->setObjectName("pageTitle");
    layout->addWidget(title);
    
    auto* flowSection = createSection("Optimization Flow");
    auto* flowLayout = flowSection->layout();
    
    auto* flowLabel = new QLabel(
        "SCAN → DETECT → ANALYZE → PLAN → USER REVIEW → APPLY → VERIFY → RESULT\n\n"
        "Application discovery and optimization plans require backend provider support.\n"
        "Current provider capabilities determine what can be detected and optimized.");
    flowLabel->setWordWrap(true);
    flowLabel->setObjectName("flowDescription");
    flowLayout->addWidget(flowLabel);
    
    layout->addWidget(flowSection);
    
    auto* planSection = createSection("Optimization Plan");
    auto* planLayout = planSection->layout();
    
    auto* planTable = new QTableWidget(0, 5);
    planTable->setHorizontalHeaderLabels({"Application", "Executable", "API", "Profile", "Compatibility"});
    planTable->horizontalHeader()->setStretchLastSection(true);
    planTable->verticalHeader()->setVisible(false);
    planLayout->addWidget(planTable);
    
    layout->addWidget(planSection);
    
    auto* statusLabel = new QLabel(
        "Status: Waiting for scan\n\n"
        "Detected: —\n"
        "Recommended: —\n"
        "Skipped: —\n"
        "Unsupported: —\n"
        "Risks: None identified");
    statusLabel->setObjectName("optimizationStatus");
    statusLabel->setWordWrap(true);
    layout->addWidget(statusLabel);
    
    auto* optimizeButton = new QPushButton("Scan for Applications");
    optimizeButton->setEnabled(false);
    optimizeButton->setToolTip("Backend provider does not support application discovery");
    layout->addWidget(optimizeButton);
    
    layout->addStretch();
    
    return scroll;
}

QWidget* MainWindow::buildGraphicsPage() {
    auto* page = new QWidget;
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(page);
    
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(16);
    
    auto* title = new QLabel("Graphics Compatibility");
    title->setObjectName("pageTitle");
    layout->addWidget(title);
    
    auto* upscalingSection = createSection("Upscaling");
    auto* upscLayout = upscalingSection->layout();
    addFormRow(upscLayout, "Native Scaling", "Unknown — requires verified application/backend support");
    addFormRow(upscLayout, "FSR", "Unknown");
    addFormRow(upscLayout, "XeSS", "Unknown");
    addFormRow(upscLayout, "OptiScaler", "Not installed — external component not detected");
    addFormRow(upscLayout, "DLSS Compatibility", "Unavailable — DLSS is not native to V100");
    layout->addWidget(upscalingSection);
    
    auto* frameGenSection = createSection("Frame Generation");
    auto* fgLayout = frameGenSection->layout();
    addFormRow(fgLayout, "Frame Generation", "Unknown — native and compatibility-layer features are distinct");
    layout->addWidget(frameGenSection);
    
    auto* otherSection = createSection("Other Graphics Features");
    auto* otherLayout = otherSection->layout();
    addFormRow(otherLayout, "VSync", "Unknown");
    addFormRow(otherLayout, "FPS Limit", "Unknown");
    addFormRow(otherLayout, "Frame Pacing", "Unknown");
    addFormRow(otherLayout, "Sharpening", "Unknown");
    addFormRow(otherLayout, "DRS", "Unknown");
    addFormRow(otherLayout, "HDR", "Unknown");
    addFormRow(otherLayout, "Shader Cache", "Unknown");
    layout->addWidget(otherSection);
    
    auto* infoLabel = new QLabel(
        "Graphics features are exposed only when a provider can establish a safe, verified configuration path.\n"
        "DLSS is never shown as native V100 capability.");
    infoLabel->setWordWrap(true);
    infoLabel->setObjectName("infoText");
    layout->addWidget(infoLabel);
    
    layout->addStretch();
    
    return scroll;
}

QWidget* MainWindow::buildAppearancePage() {
    auto* page = new QWidget;
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(page);
    
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(16);
    
    auto* title = new QLabel("Appearance");
    title->setObjectName("pageTitle");
    layout->addWidget(title);
    
    auto* presetSection = createSection("Appearance Preset");
    
    auto* presetRow = new QHBoxLayout;
    presetRow->setSpacing(12);
    
    appearancePreset_ = new QComboBox;
    appearancePreset_->setFixedWidth(200);
    appearancePreset_->addItems({
        "ISB Default", "Industrial", "Minimal", 
        "Terminal", "Laboratory", "Linux Rice", "High Contrast"
    });
    connect(appearancePreset_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAppearancePresetChanged);
    
    presetRow->addWidget(new QLabel("Preset:"));
    presetRow->addWidget(appearancePreset_);
    presetRow->addStretch();
    
    auto* presetLayout = getSectionLayout(presetSection);
    presetLayout->addLayout(presetRow);
    
    auto* presetDesc = new QLabel(
        "Presets change only presentation/appearance. They do not affect GPU settings or hardware configuration.");
    presetDesc->setWordWrap(true);
    presetDesc->setObjectName("presetDescription");
    presetLayout->addWidget(presetDesc);
    
    layout->addWidget(presetSection);
    
    auto* wallpaperSection = createSection("Wallpaper");
    auto* wpLayout = wallpaperSection->layout();
    
    auto* wpDesc = new QLabel(
        "Wallpaper modes:\n"
        "- Solid background\n"
        "- Static image\n"
        "- User-selected image\n"
        "- Fit/Fill/Center/Stretch modes\n"
        "\n"
        "Default wallpaper is dark for readability. Overlays/dimming preserve text legibility.");
    wpDesc->setWordWrap(true);
    wpLayout->addWidget(wpDesc);
    
    auto* selectWpButton = new QPushButton("Select Wallpaper");
    selectWpButton->setEnabled(false);
    selectWpButton->setToolTip("Wallpaper selection not yet implemented");
    wpLayout->addWidget(selectWpButton);
    
    layout->addWidget(wallpaperSection);
    
    auto* customizationSection = createSection("Customization Options");
    auto* custLayout = customizationSection->layout();
    
    addFormRow(custLayout, "UI Opacity", "100% (adjustable if platform-safe)");
    addFormRow(custLayout, "Accent Color", "Configurable per theme");
    addFormRow(custLayout, "Density", "Compact / Normal");
    addFormRow(custLayout, "Font Size", "Configurable");
    addFormRow(custLayout, "Sidebar Mode", "Compact / Expanded");
    addFormRow(custLayout, "Animation Level", "None / Minimal / Full");
    addFormRow(custLayout, "Transparency", "Platform-dependent");
    addFormRow(custLayout, "Theme", "Dark (default) / Light");
    
    layout->addWidget(customizationSection);
    
    auto* infoLabel = new QLabel(
        "Appearance settings are stored separately from hardware configuration.\n"
        "Changing wallpaper never affects GPU settings.\n"
        "Configuration loads safely — unknown fields are ignored.");
    infoLabel->setWordWrap(true);
    infoLabel->setObjectName("infoText");
    layout->addWidget(infoLabel);
    
    layout->addStretch();
    
    return scroll;
}

void MainWindow::onRefresh() {
    updateHeader();
    updateGPUPage();
    updateTuningPage();
}

void MainWindow::onNavigate(int index) {
    pages_->setCurrentIndex(index);
}

void MainWindow::onProfileChanged(const QString& profile) {
    currentProfile_ = profile.toStdString();
    
    QString description;
    if (profile == "Balanced") {
        description = "Normal performance / power balance.";
    } else if (profile == "Power Saving") {
        description = "Reduce power usage and thermal load where supported.";
    } else if (profile == "Gaming") {
        description = "Prioritize graphics performance where supported.";
    } else if (profile == "Maximum Performance") {
        description = "Prioritize sustained GPU performance where supported.";
    } else if (profile == "Compute") {
        description = "Prioritize compute workloads.";
    } else if (profile == "AI / Tensor") {
        description = "Prioritize Tensor-oriented workloads where supported.";
    } else if (profile == "Custom") {
        description = "User-defined configuration.";
    }
    
    profileDescriptionLabel_->setText(description);
}

void MainWindow::onReviewPlan() {
    const auto plan = hub_->profile_plan(currentProfile_);
    
    QString message = QString("Operations: %1\nUnknown: %2\nUnsupported: %3")
        .arg(static_cast<int>(plan.operations.size()))
        .arg(static_cast<int>(plan.unknown.size()))
        .arg(static_cast<int>(plan.unsupported.size()));
    
    QString informativeText;
    if (plan.risks.empty()) {
        informativeText = "No additional risk information.";
    } else {
        informativeText = QString::fromStdString(plan.risks.front());
    }
    
    QMessageBox dialog(this);
    dialog.setWindowTitle(QString("Profile Plan: %1").arg(QString::fromStdString(currentProfile_)));
    dialog.setText(message);
    dialog.setInformativeText(informativeText);
    dialog.setStandardButtons(QMessageBox::Cancel | QMessageBox::Apply);
    
    if (dialog.exec() == QMessageBox::Apply) {
        onApplyProfile();
    }
}

void MainWindow::onApplyProfile() {
    const auto plan = hub_->profile_plan(currentProfile_);
    const auto result = hub_->apply(plan, true);
    
    QString stateText;
    switch (result.state) {
        case hub::OperationState::Planned: stateText = "Planned"; break;
        case hub::OperationState::Applied: stateText = "Applied"; break;
        case hub::OperationState::Verified: stateText = "Verified"; break;
        case hub::OperationState::Failed: stateText = "Failed"; break;
        case hub::OperationState::RolledBack: stateText = "Rolled Back"; break;
    }
    
    QString message = QString("State: %1\nMutated: %2\nVerification: %3")
        .arg(stateText)
        .arg(result.mutated ? "Yes" : "No")
        .arg(QString::fromStdString(result.verification.code));
    
    QMessageBox::information(this, "Apply Result", message);
}

void MainWindow::onOptimize() {
    QMessageBox::information(this, "Optimize", "Optimization scan not yet implemented.");
}

void MainWindow::onAppearancePresetChanged(int index) {
    const QString preset = appearancePreset_->currentText();
    
    if (preset == "ISB Default") {
        // Default dark theme applied via stylesheet
    } else if (preset == "High Contrast") {
        // High contrast would be applied here
    }
    
    appearancePreset_->setToolTip(QString("Selected: %1").arg(preset));
}

void MainWindow::updateHeader() {
    const auto env = hub_->environment();
    const auto caps = hub_->capabilities();
    const auto telemetry = hub_->telemetry();
    
    QString headerText = QString("MODE: %1  |  GPU: %2  |  TEMP: %3°C  |  UTIL: %4%  |  POWER: %5W")
        .arg(QString::fromStdString(hub::to_string(env.mode)))
        .arg(QString::fromStdString(caps.identity.exact_hardware_variant))
        .arg(formatOptionalInt(telemetry.temperature_c, "°C"))
        .arg(formatOptionalInt(telemetry.gpu_utilization_percent, "%"))
        .arg(formatOptionalInt(telemetry.power_w, "W"));
    
    headerLabel_->setText(headerText);
    
    QString badgeText = getProviderBadgeText();
    providerBadge_->setText(badgeText);
    
    QString statusText = env.mode == hub::ProviderMode::Unavailable 
        ? "GPU Unavailable" 
        : (env.provenance.synthetic ? "Demo / Mock Data" : "Available");
    gpuStatusLabel_->setText(statusText);
}

void MainWindow::updateGPUPage() {
    const auto caps = hub_->capabilities();
    const auto telemetry = hub_->telemetry();
    const auto env = hub_->environment();
    
    gpuModelLabel_->setText(QString::fromStdString(caps.identity.exact_hardware_variant));
    gpuArchitectureLabel_->setText(QString::fromStdString(caps.identity.architecture));
    
    if (caps.identity.compute_capability.has_value()) {
        const auto& cc = caps.identity.compute_capability.value();
        computeCapabilityLabel_->setText(QString("%1.%2").arg(cc.major).arg(cc.minor));
    } else {
        computeCapabilityLabel_->setText("Unknown");
    }
    
    tensorCoresLabel_->setText(formatCapabilityState(caps.hardware.tensor_cores.state));
    rtCoresLabel_->setText(formatCapabilityState(caps.hardware.rt_cores.state));
    hbm2Label_->setText(formatCapabilityState(caps.hardware.hbm2.state));
    eccLabel_->setText(formatCapabilityState(caps.hardware.ecc.state));
    nvlinkLabel_->setText(formatCapabilityState(caps.hardware.nvlink.state));
    migLabel_->setText(formatCapabilityState(caps.hardware.mig.state));
    displayOutputsLabel_->setText(formatCapabilityState(caps.hardware.display_outputs.state));
    opticalFlowLabel_->setText(formatCapabilityState(caps.hardware.optical_flow_accelerator.state));
    
    temperatureLabel_->setText(formatOptionalInt(telemetry.temperature_c, "°C"));
    gpuUtilLabel_->setText(formatOptionalInt(telemetry.gpu_utilization_percent, "%"));
    memoryUtilLabel_->setText(formatOptionalInt(telemetry.memory_utilization_percent, "%"));
    
    if (telemetry.power_limit_w.has_value()) {
        powerLabel_->setText(formatOptionalInt(telemetry.power_w, " W"));
        powerLimitLabel_->setText(formatOptionalInt(telemetry.power_limit_w, " W"));
    } else {
        powerLabel_->setText("Unknown");
        powerLimitLabel_->setText("Unknown");
    }
    
    gpuClockLabel_->setText(formatOptionalInt(telemetry.gpu_clock_mhz, " MHz"));
    memoryClockLabel_->setText(formatOptionalInt(telemetry.memory_clock_mhz, " MHz"));
    
    vramUsedLabel_->setText(formatOptionalInt(telemetry.vram_used_mib, " MiB"));
    vramTotalLabel_->setText(formatOptionalInt(telemetry.vram_total_mib, " MiB"));
    
    perfStateLabel_->setText(formatValueOrUnknown(telemetry.performance_state));
    pcieLabel_->setText(formatValueOrUnknown(telemetry.pcie));
    processCountLabel_->setText(formatOptionalInt(telemetry.process_count));
}

void MainWindow::updateTuningPage() {
    const auto controls = hub_->controls();
    
    auto* tuningPage = pages_->widget(1);
    auto* table = tuningPage->findChild<QTableWidget*>("controlsTable");
    if (table) {
        table->setRowCount(static_cast<int>(controls.size()));
        for (int i = 0; i < static_cast<int>(controls.size()); ++i) {
            const auto& control = controls[static_cast<std::size_t>(i)];
            table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(control.name)));
            table->setItem(i, 1, new QTableWidgetItem(formatCapabilityState(control.state)));
            table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(control.reason)));
        }
    }
}

QString MainWindow::formatCapabilityState(hub::ControlState state) const {
    return QString::fromStdString(hub::to_string(state));
}

QString MainWindow::formatCapabilityState(cal::CapabilityState state) const {
    return QString::fromStdString(common::to_string(state));
}

QString MainWindow::formatValueOrUnknown(const std::string& value) const {
    if (value.empty() || value == "unknown") {
        return "Unknown";
    }
    return QString::fromStdString(value);
}

QString MainWindow::getProviderBadgeText() const {
    const auto env = hub_->environment();
    if (env.mode == hub::ProviderMode::Mock) {
        return "Provider: Mock | Mode: Demo";
    } else if (env.mode == hub::ProviderMode::Real) {
        return "Provider: NVML | Mode: Real";
    } else {
        return "Provider: Unavailable";
    }
}

} // namespace isb::control_center

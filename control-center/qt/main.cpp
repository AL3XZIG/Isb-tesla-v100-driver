#include "isb/hub/hub.hpp"

#include <QApplication>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <memory>
#include <string>

using namespace isb::hub;

namespace {

QWidget* panel(const QString& title) {
    auto* box = new QWidget;
    auto* layout = new QVBoxLayout(box);
    layout->setContentsMargins(12, 10, 12, 10);
    auto* heading = new QLabel(title);
    heading->setObjectName("heading");
    layout->addWidget(heading);
    return box;
}

QString capability(const cal::CapabilityState state) {
    return QString::fromStdString(cal::to_string(state));
}

class Window final : public QMainWindow {
public:
    Window()
        : provider_(std::make_unique<MockProvider>()),
          hub_(*provider_),
          capabilities_(hub_.capabilities()),
          telemetry_(hub_.telemetry()) {
        setWindowTitle("ISB V100 Control Center");
        resize(1080, 680);
        build();
    }

private:
    std::unique_ptr<Provider> provider_;
    Hub hub_;
    cal::GpuCapabilities capabilities_;
    TelemetrySnapshot telemetry_;
    QStackedWidget* pages_ = nullptr;

    void build() {
        auto* root = new QWidget;
        auto* body = new QHBoxLayout(root);
        body->setContentsMargins(0, 0, 0, 0);

        auto* nav = new QListWidget;
        nav->setFixedWidth(152);
        nav->addItems({"GPU", "TUNING", "OPTIMIZATION", "GRAPHICS"});
        nav->setCurrentRow(0);

        pages_ = new QStackedWidget;
        pages_->addWidget(gpu());
        pages_->addWidget(tuning());
        pages_->addWidget(optimization());
        pages_->addWidget(graphics());

        auto* right = new QVBoxLayout;
        const auto environment = hub_.environment();
        auto* header = new QLabel(
            QString("MODE: %1  |  GPU: %2  |  TEMP: %3 C  |  UTIL: %4%  |  POWER: %5 W  |  CLOCK: %6 MHz  |  DRIVER: %7")
                .arg(QString::fromStdString(to_string(environment.mode)))
                .arg(QString::fromStdString(capabilities_.identity.exact_hardware_variant))
                .arg(telemetry_.temperature_c)
                .arg(telemetry_.gpu_utilization_percent)
                .arg(telemetry_.power_w)
                .arg(telemetry_.gpu_clock_mhz)
                .arg(QString::fromStdString(environment.driver_version)));
        header->setObjectName("header");
        right->addWidget(header);
        right->addWidget(pages_);

        body->addWidget(nav);
        body->addLayout(right);
        connect(nav, &QListWidget::currentRowChanged, pages_, &QStackedWidget::setCurrentIndex);
        setCentralWidget(root);
    }

    QWidget* gpu() {
        auto* page = new QWidget;
        auto* layout = new QHBoxLayout(page);

        auto* hardware = panel("GPU");
        auto* hardware_layout = qobject_cast<QVBoxLayout*>(hardware->layout());
        auto* form = new QFormLayout;
        const auto& c = capabilities_;
        form->addRow("Model", new QLabel(QString::fromStdString(c.identity.exact_hardware_variant)));
        form->addRow("Architecture", new QLabel(QString::fromStdString(c.identity.architecture)));
        form->addRow("Compute capability", new QLabel(QString("%1.%2").arg(c.identity.compute_capability.major).arg(c.identity.compute_capability.minor)));
        form->addRow("Tensor Cores", new QLabel(capability(c.hardware.tensor_cores.state)));
        form->addRow("RT Cores", new QLabel(capability(c.hardware.rt_cores.state)));
        form->addRow("Optical Flow", new QLabel(capability(c.hardware.optical_flow_accelerator.state)));
        form->addRow("Display outputs", new QLabel(capability(c.hardware.display_outputs.state)));
        hardware_layout->addLayout(form);
        hardware_layout->addStretch();

        auto* current = panel("CURRENT STATE");
        auto* current_layout = qobject_cast<QVBoxLayout*>(current->layout());
        auto* state = new QFormLayout;
        state->addRow("GPU utilization", new QLabel(QString("%1%").arg(telemetry_.gpu_utilization_percent)));
        state->addRow("Memory utilization", new QLabel(QString("%1%").arg(telemetry_.memory_utilization_percent)));
        state->addRow("Temperature", new QLabel(QString("%1 C").arg(telemetry_.temperature_c)));
        state->addRow("Power", new QLabel(QString("%1 / %2 W").arg(telemetry_.power_w).arg(telemetry_.power_limit_w)));
        state->addRow("GPU clock", new QLabel(QString("%1 MHz").arg(telemetry_.gpu_clock_mhz)));
        state->addRow("Memory clock", new QLabel(QString("%1 MHz").arg(telemetry_.memory_clock_mhz)));
        state->addRow("VRAM", new QLabel(QString("%1 / %2 MiB").arg(telemetry_.vram_used_mib).arg(telemetry_.vram_total_mib)));
        state->addRow("ECC", new QLabel(QString::fromStdString(telemetry_.ecc)));
        current_layout->addLayout(state);

        layout->addWidget(hardware);
        layout->addWidget(current);
        return page;
    }

    QWidget* tuning() {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);

        auto* profile = new QHBoxLayout;
        auto* selector = new QComboBox;
        selector->addItems({"Balanced", "Gaming", "Compute", "AI / Tensor", "Maximum Performance", "Low Power", "Custom"});
        auto* review = new QPushButton("Review plan");
        profile->addWidget(new QLabel("Profile"));
        profile->addWidget(selector);
        profile->addWidget(review);
        profile->addStretch();
        layout->addLayout(profile);

        const auto controls = hub_.controls();
        auto* table = new QTableWidget(static_cast<int>(controls.size()), 3);
        table->setHorizontalHeaderLabels({"Control", "State", "Details"});
        for (int i = 0; i < static_cast<int>(controls.size()); ++i) {
            const auto& control = controls[static_cast<std::size_t>(i)];
            table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(control.name)));
            table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(to_string(control.state))));
            table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(control.reason)));
        }
        layout->addWidget(table);

        connect(review, &QPushButton::clicked, this, [this, selector] {
            const auto plan = hub_.profile_plan(selector->currentText().toStdString());
            QMessageBox dialog(this);
            dialog.setWindowTitle("Requested → Planned → Apply → Verify");
            dialog.setText(QString("Operations: %1\nUnknown: %2\nUnsupported: %3")
                .arg(static_cast<int>(plan.operations.size()))
                .arg(static_cast<int>(plan.unknown.size()))
                .arg(static_cast<int>(plan.unsupported.size())));
            dialog.setInformativeText(QString::fromStdString(plan.risks.empty() ? "No additional risk information." : plan.risks.front()));
            dialog.setStandardButtons(QMessageBox::Cancel | QMessageBox::Apply);
            if (dialog.exec() == QMessageBox::Apply) {
                const auto result = hub_.apply(plan, true);
                QMessageBox::information(
                    this,
                    "Apply result",
                    QString("%1; verification: %2")
                        .arg(result.state == OperationState::Failed ? "Failed" : "Completed")
                        .arg(QString::fromStdString(result.verification.code)));
            }
        });

        return page;
    }

    QWidget* optimization() {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto* title = new QLabel("GAMES & APPLICATIONS");
        title->setObjectName("heading");
        layout->addWidget(title);
        auto* table = new QTableWidget(0, 5);
        table->setHorizontalHeaderLabels({"Application", "Executable", "API", "Profile", "Compatibility"});
        layout->addWidget(table);
        layout->addWidget(new QLabel(
            "Optimize follows SCAN → DETECT → ANALYZE → PLAN → USER REVIEW → APPLY → VERIFY. "
            "Application discovery is not implemented by the current provider."));
        return page;
    }

    QWidget* graphics() {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto* form = new QFormLayout;
        form->addRow("Upscaling", new QLabel("Unknown — requires verified application/backend support"));
        form->addRow("Frame generation", new QLabel("Unknown — native and compatibility-layer features are distinct"));
        form->addRow("OptiScaler", new QLabel("Unknown — external component not detected by the current provider"));
        layout->addLayout(form);
        layout->addWidget(new QLabel(
            "Graphics features are exposed only when a provider can establish a safe, verified configuration path."));
        layout->addStretch();
        return page;
    }
};

} // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setStyleSheet(
        "QWidget{background:#202124;color:#e8eaed;font-size:13px}"
        "QListWidget{background:#171819;border:0;padding:8px}"
        "QListWidget::item{padding:10px 8px}"
        "QListWidget::item:selected{background:#34422d;color:#fff}"
        "#header{background:#292b2e;padding:9px;color:#c9e7b9}"
        "#heading{font-weight:700;color:#b8df9b}"
        "QTableWidget{gridline-color:#3a3c3f;border:1px solid #3a3c3f}"
        "QPushButton{background:#4d772f;border:0;padding:7px 12px;color:white}"
        "QComboBox{padding:5px;background:#303236}");
    Window window;
    window.show();
    return app.exec();
}

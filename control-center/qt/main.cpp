#include "mainwindow.hpp"

#include "isb/hub/runtime_provider.hpp"

#include <QApplication>
#include <memory>

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QWidget{background:#1a1b1e;color:#e8eaed;font-size:13px}"
        "QMainWindow{background:#1a1b1e}"
        "QListWidget#sidebar{background:#0f1012;border:0;padding:8px;font-weight:500}"
        "QListWidget#sidebar::item{padding:10px 12px;border-radius:4px;margin:2px 4px}"
        "QListWidget#sidebar::item:selected{background:#3d7e36;color:#ffffff;font-weight:600}"
        "QListWidget#sidebar::item:hover{background:#2a2c30}"
        "QListWidget#sidebar::item:pressed{background:#3d7e36}"
        "#header{background:#25272b;border-bottom:1px solid #3a3c42;padding:10px 16px}"
        "#headerTitle{font-weight:700;font-size:14px;color:#8bc34a}"
        "#headerInfo{color:#aab0b9;font-size:12px}"
        "#providerBadge{background:#2d3136;color:#8bc34a;padding:4px 10px;border-radius:3px;font-size:11px;font-weight:600}"
        "#gpuStatus{color:#ffb74d;font-size:12px;font-weight:600}"
        "#panelHeading{font-weight:700;color:#8bc34a;font-size:13px}"
        "#section{background:#25272b;border:1px solid #3a3c42;border-radius:6px}"
        "#sectionHeading{color:#aab0b9;font-size:12px;margin-bottom:8px}"
        "#fieldName{color:#8bc34a;font-weight:600;font-size:12px}"
        "#fieldValue{color:#e8eaed;font-size:12px}"
        "#pageTitle{font-size:18px;color:#8bc34a;font-weight:600}"
        "#profileDescription{color:#aab0b9;font-size:12px;margin-top:6px}"
        "#infoText{color:#6a6f77;font-size:11px;font-style:italic;margin-top:6px}"
        "#presetDescription{color:#aab0b9;font-size:11px;font-style:italic;margin-top:6px}"
        "#flowDescription{color:#aab0b9;font-size:12px;line-height:1.5}"
        "#optimizationStatus{color:#aab0b9;font-size:12px;line-height:1.6}"
        "QTableWidget{background:#1f2126;gridline-color:#3a3c42;border:1px solid #3a3c42;border-radius:4px}"
        "QTableWidget::item{padding:6px}"
        "QHeaderView::section{background:#2d3136;color:#aab0b9;padding:6px;border:1px solid #3a3c42}"
        "QPushButton{background:#3d7e36;color:white;padding:8px 14px;border:0;border-radius:4px}"
        "QPushButton:hover{background:#4a9441}"
        "QPushButton:disabled{background:#3a3c42;color:#6a6f77}"
        "QComboBox{background:#2d3136;color:#e8eaed;border:1px solid #3a3c42;padding:6px;border-radius:4px}"
        "QScrollArea{border:0}"
        "QMessageBox QLabel{color:#e8eaed}"
        "QGroupBox{border:1px solid #3a3c42;border-radius:4px;margin-top:12px;padding-top:8px}"
        "QGroupBox::title{subcontrol-origin:margin;left:10px;padding:0 5px;color:#8bc34a}");

    auto provider = isb::hub::make_runtime_provider();
    isb::control_center::MainWindow window(std::move(provider));
    window.show();

    return app.exec();
}

#include "mainwindow.hpp"

#include <QApplication>
#include <QMessageBox>
#include <memory>

using namespace isb::hub;

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
        "#sectionHeading{font-weight:700;color:#aab0b9;font-size:12px;margin-bottom:8px}"
        "#fieldName{color:#8bc34a;font-weight:600;font-size:12px}"
        "#fieldValue{color:#e8eaed;font-size:12px}"
        "#pageTitle{font-weight:700;font-size:18px;color:#8bc34a;margin-bottom:8px}"
        "#profileDescription{color:#aab0b9;font-size:12px;margin-top:8px}"
        "#infoText{color:#6a6f77;font-size:11px;font-style:italic;margin-top:8px}"
        "#presetDescription{color:#aab0b9;font-size:12px;margin-top:8px}"
        "#flowDescription{color:#aab0b9;font-size:12px;line-height:1.5}"
        "#optimizationStatus{color:#aab0b9;font-size:12px;line-height:1.6}"
        "QTableWidget{background:#1f2126;gridline-color:#3a3c42;border:1px solid #3a3c42;font-size:12px}"
        "QTableWidget::item{padding:6px 8px}"
        "QHeaderView::section{background:#2d3136;color:#aab0b9;padding:8px;border:1px solid #3a3c42;font-weight:600}"
        "QPushButton{background:#3d7e36;border:0;padding:8px 16px;color:white;font-weight:600;border-radius:4px;min-width:80px}"
        "QPushButton:hover{background:#4a9441}"
        "QPushButton:pressed{background:#2f662a}"
        "QPushButton:disabled{background:#3a3c42;color:#6a6f77}"
        "QComboBox{padding:6px 10px;background:#2d3136;border:1px solid #3a3c42;border-radius:4px;color:#e8eaed;font-size:12px}"
        "QComboBox::drop-down{border:0;width:20px}"
        "QComboBox QAbstractItemView{background:#2d3136;border:1px solid #3a3c42;selection-background-color:#3d7e36}"
        "QScrollArea{border:0;background:transparent}"
        "QFrame{background:transparent}"
        "QLabel{color:#e8eaed}"
        "QMessageBox{background:#1f2126}"
        "QMessageBox QLabel{color:#e8eaed}"
        "QGroupBox{border:1px solid #3a3c42;border-radius:6px;margin-top:12px;font-weight:600;color:#8bc34a}"
        "QGroupBox::title{subcontrol-origin:margin;subcontrol-position:top left;left:12px;padding:0 6px;color:#8bc34a}");
    
    auto provider = std::make_unique<MockProvider>();
    isb::control_center::MainWindow window(std::move(provider));
    window.show();
    
    return app.exec();
}

/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.hpp'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../control-center/qt/mainwindow.hpp"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_isb__control_center__MainWindow_t {
    uint offsetsAndSizes[22];
    char stringdata0[32];
    char stringdata1[10];
    char stringdata2[1];
    char stringdata3[11];
    char stringdata4[6];
    char stringdata5[17];
    char stringdata6[8];
    char stringdata7[13];
    char stringdata8[15];
    char stringdata9[11];
    char stringdata10[26];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_isb__control_center__MainWindow_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_isb__control_center__MainWindow_t qt_meta_stringdata_isb__control_center__MainWindow = {
    {
        QT_MOC_LITERAL(0, 31),  // "isb::control_center::MainWindow"
        QT_MOC_LITERAL(32, 9),  // "onRefresh"
        QT_MOC_LITERAL(42, 0),  // ""
        QT_MOC_LITERAL(43, 10),  // "onNavigate"
        QT_MOC_LITERAL(54, 5),  // "index"
        QT_MOC_LITERAL(60, 16),  // "onProfileChanged"
        QT_MOC_LITERAL(77, 7),  // "profile"
        QT_MOC_LITERAL(85, 12),  // "onReviewPlan"
        QT_MOC_LITERAL(98, 14),  // "onApplyProfile"
        QT_MOC_LITERAL(113, 10),  // "onOptimize"
        QT_MOC_LITERAL(124, 25)   // "onAppearancePresetChanged"
    },
    "isb::control_center::MainWindow",
    "onRefresh",
    "",
    "onNavigate",
    "index",
    "onProfileChanged",
    "profile",
    "onReviewPlan",
    "onApplyProfile",
    "onOptimize",
    "onAppearancePresetChanged"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_isb__control_center__MainWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x08,    1 /* Private */,
       3,    1,   57,    2, 0x08,    2 /* Private */,
       5,    1,   60,    2, 0x08,    4 /* Private */,
       7,    0,   63,    2, 0x08,    6 /* Private */,
       8,    0,   64,    2, 0x08,    7 /* Private */,
       9,    0,   65,    2, 0x08,    8 /* Private */,
      10,    1,   66,    2, 0x08,    9 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,

       0        // eod
};

Q_CONSTINIT const QMetaObject isb::control_center::MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_isb__control_center__MainWindow.offsetsAndSizes,
    qt_meta_data_isb__control_center__MainWindow,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_isb__control_center__MainWindow_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'onRefresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onNavigate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onProfileChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onReviewPlan'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onApplyProfile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onOptimize'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAppearancePresetChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void isb::control_center::MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onRefresh(); break;
        case 1: _t->onNavigate((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->onProfileChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->onReviewPlan(); break;
        case 4: _t->onApplyProfile(); break;
        case 5: _t->onOptimize(); break;
        case 6: _t->onAppearancePresetChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *isb::control_center::MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *isb::control_center::MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_isb__control_center__MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int isb::control_center::MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

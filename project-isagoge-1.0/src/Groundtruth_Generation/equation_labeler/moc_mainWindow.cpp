/****************************************************************************
** Meta object code from reading C++ file 'mainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "src/mainWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_mainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      32,   26,   11,   11, 0x0a,
      46,   11,   11,   11, 0x0a,
      57,   11,   11,   11, 0x0a,
      69,   11,   11,   11, 0x0a,
      87,   11,   11,   11, 0x0a,
     102,   11,   11,   11, 0x0a,
     114,   11,   11,   11, 0x0a,
     132,   11,   11,   11, 0x0a,
     149,   11,   11,   11, 0x0a,
     168,   11,   11,   11, 0x0a,
     185,   11,   11,   11, 0x0a,
     196,   11,   11,   11, 0x0a,
     209,   11,   11,   11, 0x0a,
     222,   11,   11,   11, 0x0a,
     231,   11,   11,   11, 0x0a,
     241,   11,   11,   11, 0x0a,
     256,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_mainWindow[] = {
    "mainWindow\0\0tab_changed()\0index\0"
    "closeTab(int)\0file_new()\0file_open()\0"
    "openGroundTruth()\0import_image()\0"
    "file_exit()\0writecoordsdisp()\0"
    "writecoordsemb()\0writecoordslabel()\0"
    "view_tool_file()\0zoomMode()\0selectMode()\0"
    "normalMode()\0zoomIn()\0zoomOut()\0"
    "doc_img_only()\0closePage()\0"
};

void mainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        mainWindow *_t = static_cast<mainWindow *>(_o);
        switch (_id) {
        case 0: _t->tab_changed(); break;
        case 1: _t->closeTab((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->file_new(); break;
        case 3: _t->file_open(); break;
        case 4: _t->openGroundTruth(); break;
        case 5: _t->import_image(); break;
        case 6: _t->file_exit(); break;
        case 7: _t->writecoordsdisp(); break;
        case 8: _t->writecoordsemb(); break;
        case 9: _t->writecoordslabel(); break;
        case 10: _t->view_tool_file(); break;
        case 11: _t->zoomMode(); break;
        case 12: _t->selectMode(); break;
        case 13: _t->normalMode(); break;
        case 14: _t->zoomIn(); break;
        case 15: _t->zoomOut(); break;
        case 16: _t->doc_img_only(); break;
        case 17: _t->closePage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData mainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject mainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_mainWindow,
      qt_meta_data_mainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &mainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *mainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *mainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_mainWindow))
        return static_cast<void*>(const_cast< mainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int mainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}
static const uint qt_meta_data_MouseEventEater[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_MouseEventEater[] = {
    "MouseEventEater\0"
};

void MouseEventEater::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MouseEventEater::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MouseEventEater::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_MouseEventEater,
      qt_meta_data_MouseEventEater, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MouseEventEater::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MouseEventEater::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MouseEventEater::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MouseEventEater))
        return static_cast<void*>(const_cast< MouseEventEater*>(this));
    return QObject::qt_metacast(_clname);
}

int MouseEventEater::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE

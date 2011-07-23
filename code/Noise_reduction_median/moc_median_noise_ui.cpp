/****************************************************************************
** Meta object code from reading C++ file 'median_noise_ui.h'
**
** Created: Sun Jul 24 03:30:47 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../median_noise_ui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'median_noise_ui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_median_noise_ui[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   17,   16,   16, 0x08,
      44,   16,   16,   16, 0x28,
      63,   17,   16,   16, 0x08,
      89,   16,   16,   16, 0x28,
     109,   17,   16,   16, 0x08,
     131,   16,   16,   16, 0x28,

       0        // eod
};

static const char qt_meta_stringdata_median_noise_ui[] = {
    "median_noise_ui\0\0t\0setCompareWindow(double)\0"
    "setCompareWindow()\0setResearchWindow(double)\0"
    "setResearchWindow()\0setSigmaValue(double)\0"
    "setSigmaValue()\0"
};

const QMetaObject median_noise_ui::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_median_noise_ui,
      qt_meta_data_median_noise_ui, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &median_noise_ui::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *median_noise_ui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *median_noise_ui::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_median_noise_ui))
        return static_cast<void*>(const_cast< median_noise_ui*>(this));
    return QDialog::qt_metacast(_clname);
}

int median_noise_ui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setCompareWindow((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: setCompareWindow(); break;
        case 2: setResearchWindow((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: setResearchWindow(); break;
        case 4: setSigmaValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: setSigmaValue(); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

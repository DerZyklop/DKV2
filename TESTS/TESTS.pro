QT += testlib
QT += sql

CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES +=  tst_db.cpp \
    ../dkv2/dbfield.cpp \
    ../dkv2/dbstructure.cpp \
    ../dkv2/dbtable.cpp \
    ../dkv2/filehelper.cpp \
    ../dkv2/helper.cpp \
    test_dkdbhelper.cpp \
    test_main.cpp

HEADERS += \
    test_dkdbhelper.h \
    tst_db.h

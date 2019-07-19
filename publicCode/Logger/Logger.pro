TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    CLogWthSqlite.cpp \
    clogger.cpp \
    SQLiteWrapper.cpp

HEADERS += \
    CLogWthSqlite.h \
    clogger.h \
    SQLiteWrapper.h

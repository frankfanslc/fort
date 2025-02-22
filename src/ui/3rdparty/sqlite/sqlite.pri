CONFIG(release, debug|release) {
    DEFINES *= NDEBUG
} else {
    DEFINES += SQLITE_DEBUG SQLITE_ENABLE_API_ARMOR
}

DEFINES += _HAVE_SQLITE_CONFIG_H

SQLITE_DIR = $$PWD/../../../3rdparty/sqlite

INCLUDEPATH += $$SQLITE_DIR

SOURCES += \
    $$SQLITE_DIR/sqlite3.c \
    $$PWD/sqlitedb.cpp \
    $$PWD/sqlitestmt.cpp

HEADERS += \
    $$SQLITE_DIR/config.h \
    $$SQLITE_DIR/sqlite.h \
    $$SQLITE_DIR/sqlite3.h \
    $$PWD/sqlitedb.h \
    $$PWD/sqlitestmt.h

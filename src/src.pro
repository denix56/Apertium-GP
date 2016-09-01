#-------------------------------------------------
#
# Project created by QtCreator 2016-03-02T18:01:36
#
#-------------------------------------------------
QT       += core gui network sql
win32:QT += winextras
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = apertium-gp
TEMPLATE = app
VERSION = 1.0.0
DESTDIR = ../bin
MOC_DIR = ../build/moc
RCC_DIR = ../build/rcc
UI_DIR = ../build/ui
unix:OBJECTS_DIR = ../build/o/unix
win32:OBJECTS_DIR = ../build/o/win32
macx:OBJECTS_DIR = ../build/o/macx

INCLUDEPATH += \
    GUI \
    Installer \
    ChooseDialog

SOURCES += \
    GUI/headbutton.cpp \
    GUI/inputtextedit.cpp \
    GUI/langdelegate.cpp \
    GUI/languagetablemodel.cpp \
    GUI/tablecombobox.cpp \
    GUI/translator.cpp \
    Installer/downloadmodel.cpp \
    Installer/downloadwindow.cpp \
    Installer/installerdelegate.cpp \
    initializer.cpp \
    main.cpp \
    Installer/managerhelper.cpp \
    GUI/dragndropwidget.cpp \
    GUI/settingsdialog.cpp \
    GUI/traywidget.cpp \
    GUI/trayinputtextedit.cpp \
    GUI/gpmainwindow.cpp \
    GUI/docshandler.cpp \
    singleapplication.cpp

HEADERS  += \
    GUI/headbutton.h \
    GUI/inputtextedit.h \
    GUI/langdelegate.h \
    GUI/languagetablemodel.h \
    GUI/tablecombobox.h \
    GUI/translator.h \
    Installer/downloadmodel.h \
    Installer/downloadwindow.h \
    Installer/installerdelegate.h \
    initializer.h \
    Installer/managerhelper.h \
    GUI/dragndropwidget.h \
    GUI/settingsdialog.h \
    GUI/traywidget.h \
    GUI/trayinputtextedit.h \
    GUI/gpmainwindow.h \
    GUI/docshandler.h \
    singleapplication.h


FORMS    += \
    Installer/downloadwindow.ui \
    GUI/settingsdialog.ui \
    GUI/traywidget.ui \
    GUI/gpmainwindow.ui \
    GUI/docshandler.ui


QMAKE_CXXFLAGS += -std=c++11
DEFINES  += QT_NO_SSL

RESOURCES += \
    GUI/application.qrc

RC_FILE = GUI/apertium.rc

unix:!macx {
req.files = \
    langNames.db \
    scripts/apertium-gp-helper.sh
req.path = /usr/share/apertium-gp
policy.files = policy/org.apertium.apertium-gp.policy
policy.path = /usr/share/polkit-1/actions

INSTALLS += req policy

QMAKE_INSTALL_FILE = install -m 744 -p -o root -g root
}

DISTFILES += \
    ../AUTHORS \
    ../COPYING \
    ../Apertium-GP.desktop \
    ../apertium.ico \
    scripts/apertium-gp-helper.pl


unix:!macx:contains(QT_VERSION, ^5\\.[0-5]\\..*) {
warning("There may be some problems with tray icon. Please, use Qt version >= 5.6.\
Your version Qt $${QT_VERSION}")
}

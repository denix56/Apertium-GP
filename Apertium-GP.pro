#-------------------------------------------------
#
# Project created by QtCreator 2016-03-02T18:01:36
#
#-------------------------------------------------
QT       += core gui network widgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Apertium-GP
TEMPLATE = app


SOURCES += main.cpp\
    Installer/downloadmodel.cpp \
    GUI/apertiumgui.cpp \
    GUI/headbutton.cpp \
    GUI/inputtextedit.cpp \
    GUI/langdelegate.cpp \
    GUI/languagetablemodel.cpp \
    GUI/tablecombobox.cpp \
    GUI/translator.cpp \
    ChooseDialog/choosedialog.cpp \
    Installer/installerdelegate.cpp \
    initializer.cpp \
    Installer/downloadwindow.cpp

HEADERS  += \
    Installer/downloadmodel.h \
    GUI/apertiumgui.h \
    GUI/headbutton.h \
    GUI/inputtextedit.h \
    GUI/langdelegate.h \
    GUI/languagetablemodel.h \
    GUI/tablecombobox.h \
    GUI/translator.h \
    ChooseDialog/choosedialog.h \
    Installer/installerdelegate.h \
    initializer.h \
    Installer/downloadwindow.h


FORMS    += \
    GUI/apertiumgui.ui \
    ChooseDialog/choosedialog.ui \
    Installer/downloadwindow.ui

INCLUDEPATH += $$PWD/GUI \
    $$PWD/Installer \
    $$PWD/ChooseDialog \

INCLUDEPATH +=

QMAKE_CXXFLAGS += -std=gnu++11
DEFINES  += QT_NO_SSL

RESOURCES += \
    GUI/application.qrc

DISTFILES += \
    langNames.db

RC_FILE = GUI/apertium.rc


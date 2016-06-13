#-------------------------------------------------
#
# Project created by QtCreator 2016-03-02T18:01:36
#
#-------------------------------------------------
QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = apertium-gp
TEMPLATE = app

DESTDIR = ../bin
MOC_DIR = ../build/moc
RCC_DIR = ../build/rcc
UI_DIR = ../build/ui
unix:OBJECTS_DIR = ../build/o/unix
win32:OBJECTS_DIR = ../build/o/win32
macx:OBJECTS_DIR = ../build/o/mac

INCLUDEPATH += \
    GUI \
    Installer \
    ChooseDialog

SOURCES += \
    ChooseDialog/choosedialog.cpp \
    GUI/apertiumgui.cpp \
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
    Installer/managerhelper.cpp

HEADERS  += \
    ChooseDialog/choosedialog.h \
    GUI/apertiumgui.h \
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
    Installer/managerhelper.h


FORMS    += \
    ChooseDialog/choosedialog.ui \
    GUI/apertiumgui.ui \
    Installer/downloadwindow.ui


QMAKE_CXXFLAGS += -std=c++11
DEFINES  += QT_NO_SSL

RESOURCES += \
    GUI/application.qrc

RC_FILE = GUI/apertium.rc

unix:!macx {
appdata.extra = \
if !( [ -e "~/.local" ] ) then \
    mkdir "~/.apertium-gp"; \
    mv langNames.db ~/.apertium-gp/langNames.db; \
    cp policy/org.apertium.apertiumgp.policy ../build/org.apertium.apertiumgp.policy; \
    cd ../build \
    perl -p -i -e \"s/local\/share\///g\" org.apertium.apertiumgp.policy; \
else \
    mv langNames.db ~/.local/share/apertium-gp/langNames.db; \
fi
appdata.files = ../build/org.apertium.apertium-gp.policy
appdata.path = /usr/share/polkit-1/actions

INSTALLS += appdata
}


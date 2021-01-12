QT       += \
    core gui \
    websockets \

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chatwindow.cpp \
    connectwindow.cpp \
    keyeventtextedit.cpp \
    main.cpp \
    packets.cpp \
    server.cpp \
    shared.cpp

HEADERS += \
    chatwindow.h \
    connectwindow.h \
    keyeventtextedit.h \
    packets.h \
    server.h \
    shared.h

FORMS += \
    chatwindow.ui \
    connectwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
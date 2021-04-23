QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        DataGenerator.cpp \
        ProNmr.cpp \
        main.cpp \
        nmrsim.cpp

INCLUDEPATH += /home/tim/usr/include
INCLUDEPATH += /home/tim/usr/include/eigen3

LIBS += -L/home/tim/usr/lib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Notes.txt

HEADERS += \
    DataGenerator.h \
    ProNmr.h \
    nmrsim.h

QT += widgets

CONFIG += c++17


SOURCES += \
    CraftDialog.cpp \
    Personazh.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CraftDialog.h \
    Personazh.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

#TRANSLATIONS += \
    #NPSCraft_ru_RU.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    mag_res/wizard.png

RESOURCES += \
    resources.qrc

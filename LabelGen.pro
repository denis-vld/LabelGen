QT       += core gui gui-private printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_DEBUG += -gdwarf-4
QMAKE_CFLAGS_DEBUG += -gdwarf-4

# Путь для Windows сборки
win32 {
    CONFIG(debug, debug|release) {
        BUILD_DIR = $$PWD/build/win32/debug
    } else {
        BUILD_DIR = $$PWD/build/win32/release
    }
}

# Путь для Linux сборки
unix:!macx {
    CONFIG(debug, debug|release) {
        BUILD_DIR = $$PWD/build/linux/debug
    } else {
        BUILD_DIR = $$PWD/build/linux/release
    }
}

# ---------------------------------------------------------
# Перенаправление выходных файлов qmake в указанные папки
# ---------------------------------------------------------

DESTDIR      = $$BUILD_DIR/bin          # Путь к готовому .exe или бинарнику
OBJECTS_DIR  = $$BUILD_DIR/obj          # Путь к объектным файлам (.obj / .o)
MOC_DIR      = $$BUILD_DIR/moc          # Путь к сгенерированным moc-файлам
RCC_DIR      = $$BUILD_DIR/rcc          # Путь к ресурсам qrc
UI_DIR       = $$BUILD_DIR/ui           # Путь к заголовочным файлам из .ui

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
unix : !macx {
    INCLUDEPATH += /usr/include/c++/11/
    INCLUDEPATH += /usr/include/x86_64-linux-gnu/c++/11/
    LIBS += -L/usr/lib/x86_64-linux-gnu
    LIBS += -L/usr/lib/gcc/x86_64-linux-gnu/11
}

INCLUDEPATH += ./xlsx-read
INCLUDEPATH += ./qrcodegen
INCLUDEPATH += ./libxls
INCLUDEPATH += ./libxls/include
INCLUDEPATH += ./libxls/include/libxls

SOURCES += \
    libxls/src/endian.c \
    libxls/src/locale.c \
    libxls/src/ole.c \
    libxls/src/xls.c \
    libxls/src/xlstool.c \
    libxls/XlsReader.cpp \
    main.cpp \
    mainwindow.cpp \
    qrcodegen/qrcodegen.cpp \
    xlsx-read/xlsxabstractooxmlfile.cpp \
    xlsx-read/xlsxabstractsheet.cpp \
    xlsx-read/xlsxcell.cpp \
    xlsx-read/xlsxcellformula.cpp \
    xlsx-read/xlsxcelllocation.cpp \
    xlsx-read/xlsxcellrange.cpp \
    xlsx-read/xlsxcellreference.cpp \
    xlsx-read/xlsxchart.cpp \
    xlsx-read/xlsxchartsheet.cpp \
    xlsx-read/xlsxcolor.cpp \
    xlsx-read/xlsxconditionalformatting.cpp \
    xlsx-read/xlsxcontenttypes.cpp \
    xlsx-read/xlsxdatavalidation.cpp \
    xlsx-read/xlsxdatetype.cpp \
    xlsx-read/xlsxdocpropsapp.cpp \
    xlsx-read/xlsxdocpropscore.cpp \
    xlsx-read/xlsxdocument.cpp \
    xlsx-read/xlsxdrawing.cpp \
    xlsx-read/xlsxdrawinganchor.cpp \
    xlsx-read/xlsxformat.cpp \
    xlsx-read/xlsxmediafile.cpp \
    xlsx-read/xlsxnumformatparser.cpp \
    xlsx-read/xlsxreadsax.cpp \
    xlsx-read/xlsxrelationships.cpp \
    xlsx-read/xlsxrichstring.cpp \
    xlsx-read/xlsxsharedstrings.cpp \
    xlsx-read/xlsxsimpleooxmlfile.cpp \
    xlsx-read/xlsxstyles.cpp \
    xlsx-read/xlsxtheme.cpp \
    xlsx-read/xlsxutility.cpp \
    xlsx-read/xlsxworkbook.cpp \
    xlsx-read/xlsxworksheet.cpp \
    xlsx-read/xlsxzipreader.cpp \
    xlsx-read/xlsxzipwriter.cpp

HEADERS += \
    libxls/include/libxls/brdb.c.h \
    libxls/include/libxls/brdb.h \
    libxls/include/libxls/config.h \
    libxls/include/libxls/endian.h \
    libxls/include/libxls/locale.h \
    libxls/include/libxls/ole.h \
    libxls/include/libxls/xlsstruct.h \
    libxls/include/libxls/xlstool.h \
    libxls/include/libxls/xlstypes.h \
    libxls/include/xls.h \
    libxls/XlsReader.h \
    mainwindow.h \
    qrcodegen/qrcodegen.hpp \
    xlsx-read/xlsxabstractooxmlfile.h \
    xlsx-read/xlsxabstractooxmlfile_p.h \
    xlsx-read/xlsxabstractsheet.h \
    xlsx-read/xlsxcell.h \
    xlsx-read/xlsxcell_p.h \
    xlsx-read/xlsxcellformula.h \
    xlsx-read/xlsxcellformula_p.h \
    xlsx-read/xlsxcelllocation.h \
    xlsx-read/xlsxcellrange.h \
    xlsx-read/xlsxcellreference.h \
    xlsx-read/xlsxchart.h \
    xlsx-read/xlsxchart_p.h \
    xlsx-read/xlsxchartsheet.h \
    xlsx-read/xlsxchartsheet_p.h \
    xlsx-read/xlsxcolor_p.h \
    xlsx-read/xlsxconditionalformatting.h \
    xlsx-read/xlsxconditionalformatting_p.h \
    xlsx-read/xlsxcontenttypes_p.h \
    xlsx-read/xlsxdatavalidation.h \
    xlsx-read/xlsxdatavalidation_p.h \
    xlsx-read/xlsxdatetype.h \
    xlsx-read/xlsxdocpropsapp_p.h \
    xlsx-read/xlsxdocpropscore_p.h \
    xlsx-read/xlsxdocument.h \
    xlsx-read/xlsxdocument_p.h \
    xlsx-read/xlsxdrawing_p.h \
    xlsx-read/xlsxdrawinganchor_p.h \
    xlsx-read/xlsxformat.h \
    xlsx-read/xlsxformat_p.h \
    xlsx-read/xlsxglobal.h \
    xlsx-read/xlsxmediafile_p.h \
    xlsx-read/xlsxnumformatparser_p.h \
    xlsx-read/xlsxreadsax.h \
    xlsx-read/xlsxrelationships_p.h \
    xlsx-read/xlsxrichstring.h \
    xlsx-read/xlsxrichstring_p.h \
    xlsx-read/xlsxsharedstrings_p.h \
    xlsx-read/xlsxsimpleooxmlfile_p.h \
    xlsx-read/xlsxstyles_p.h \
    xlsx-read/xlsxtheme_p.h \
    xlsx-read/xlsxutility_p.h \
    xlsx-read/xlsxworkbook.h \
    xlsx-read/xlsxworkbook_p.h \
    xlsx-read/xlsxworksheet.h \
    xlsx-read/xlsxzipreader_p.h \
    xlsx-read/xlsxzipwriter_p.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    LabelGenRsr.qrc

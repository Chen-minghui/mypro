TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt



#增加线程库
LIBS += -lpthread

SOURCES += main.c \
    get_video_pic.c \
    list.c \
    show.c \
    thread_pool.c

HEADERS += \
    myplayer.h \
    kernellist.h \
    jconfig.h \
    jmorecfg.h \
    jpeglib.h \
    thread_pool.h

#增加程序库文件路径
LIBS += \
        -L ~/lib_jpg/lib -ljpeg


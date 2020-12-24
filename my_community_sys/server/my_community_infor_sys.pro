TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    http.c \
    client.c


#增加线程库
LIBS += -lpthread

HEADERS += \
    my_community_sys.h \
    cJSON.h \
    cJSON_Utils.h



#库文件链接
LIBS += -L /home/gec/cJSON/lib -lcjson\

QT += widgets
qtHaveModule(printsupport): QT += printsupport

TEMPLATE        = app
TARGET          = Thunderquake_Qt

equals(QT_ARCH,"arm") {
    DEFINES += HOST_ANDROID_ARM
    CONFIG += android_arm
}
equals(QT_ARCH,"x86") {
    DEFINES += HOST_ANDROID_X86
    CONFIG += android_x86
}
equals(QT_ARCH,"x86_64") {
    DEFINES += HOST_ANDROID_X86
    CONFIG += android_x86
}

DEFINES += APPDEV_TEST_SETA=yes

# developers working on the JSON Glk formatting can comment this out to get a full idea of the available output formatting options.
DEFINES += HIDE_TEXT_EDIT_TOOLBARS=yes

HEADERS         = \
    remglkprocess.h \
    completerwindow.h \
    remglkcompletertextedit.h \
    remglkoutputlayout.h \
    remglktexteditwindow.h \
    storyrunparameters.h \
    findfileswindow.h \
    findfilessearch.h \
    findfilesthread.h \
    glkwindowdata.h \
    outputstyledchunk.h \
    findstorieswindow.h
SOURCES         = \
    remglkprocess.cpp \
    main.cpp \
    completerwindow.cpp \
    remglkcompletertextedit.cpp \
    remglktexteditwindow.cpp \
    remglkoutputlayout.cpp \
    storyrunparameters.cpp \
    findfileswindow.cpp \
    findfilessearch.cpp \
    findfilesthread.cpp \
    glkwindowdata.cpp \
    outputstyledchunk.cpp \
    findstorieswindow.cpp

RESOURCES += \
    remglktextedit.qrc

# Example customcompleter
HEADERS   +=
SOURCES   +=

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

#
# As mentioned in one of the README files, this is incomplete (non-working) logic to try and
#  bundle the correct RemGlk executable for arm or x86 CPU into the bundle.
#  Right now, it is set for the Android Studio x86 emulator
#

android-g++ {
    message("::: section 0 android-g++")
    #ANDROID_EXTRA_LIBS += /data/app/com.wakereality.thunderfall-1/lib/x86/lib_app_git.so
}

# standard Qt way to act on Android builds
android {
    message("::: section 1 android")
    message("QT_ARCH: $$QT_ARCH")
    message("QMAKE_HOST.arch: $$QMAKE_HOST.arch")
    message (HOST:$$QMAKE_HOST)
    message (HOST:$$QMAKE_HOST.arch)
    message (TARGET:$$QMAKE_TARGET)
    message (TARGET:$$QMAKE_TARGET.arch)

    #ANDROID_EXTRA_LIBS += /data/app/com.wakereality.thunderfall-1/lib/x86/lib_app_git.so
}

android_x86 {
    message("::: section 2 android_x86 CPU x86")
    # to get compiled Linux console binary into the APK generated by QT Creator
    #ANDROID_EXTRA_LIBS += /data/app/com.wakereality.thunderfall-1/lib/x86/lib_app_git.so
}

android_arm {
    message("::: section 3 android_arm CPU arm")
    ANDROID_EXTRA_LIBS += /data/app/com.wakereality.thunderfall-1/lib/armeabi-v7a/lib_app_git.so
}


#
# Try to copy interpreter binary executable to build output path for Desktop system builds
# ToDo: skip this step for Android builds
#
# using shell_path() to correct path depending on platform
# escaping quotes and backslashes for file paths
copydata.commands = $(COPY_FILE) \"$$shell_path($$PWD\\remglk_desktop_src\\glulxe\\remglk_glulxe)\" \"$$shell_path($$OUT_PWD)\"
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata


# install
target.path = /tmp/Qt_Thunderquake_WR0
INSTALLS += target

#
# NOTE on RAMdisk use
#     "the build directory must be at the same folder level as the project (i.e. it can't be above or below). The reason why is that the linker is called from the build directory. Hence, any relative paths to library files will be different than what you entered in your project file."
#     A symbolic link could perhaps work?
#

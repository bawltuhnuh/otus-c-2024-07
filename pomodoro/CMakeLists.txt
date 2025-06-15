cmake_minimum_required(VERSION 3.5)

project(pomodoro LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(NOTIFY REQUIRED libnotify)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

include_directories(${GTK3_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
)

link_directories(${GTK3_LIBRARY_DIRS}
    ${GST_LIBRARY_DIRS}
)

add_definitions(${GTK3_CFLAGS_OTHER}
    ${GST_CFLAGS_OTHER}
)

add_definitions(-DLOCALEDIR=\"${CMAKE_INSTALL_FULL_DATADIR}/locale\")

add_executable(pomodoro
    src/main.c
    src/settings.h
    src/settings.c
    src/main_window.h
    src/main_window.c
    src/notification.h
    src/notification.c
)

target_link_libraries(pomodoro
    ${GTK3_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    ${GST_LIBRARIES}
)

install(TARGETS pomodoro DESTINATION bin)

# Локализация
install(FILES po/ru/LC_MESSAGES/pomodoro.mo DESTINATION share/locale/ru/LC_MESSAGES)
install(FILES po/en/LC_MESSAGES/pomodoro.mo DESTINATION share/locale/en/LC_MESSAGES)

# .desktop файл
install(FILES debian/pomodoro.desktop DESTINATION share/applications)

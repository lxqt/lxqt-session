cmake_minimum_required(VERSION 2.8.11)
project(lxqt-leave)

set(CPP_FILES
    main.cpp
    leavedialog.cpp
)

set(H_FILES
    leavedialog.h
)

set(UI_FILES
    leavedialog.ui
)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})

# Translations **********************************
include(LXQtTranslate)
lxqt_translate_desktop(DESKTOP_FILES
    SOURCES
    resources/lxqt-leave.desktop.in
    resources/lxqt-hibernate.desktop.in
    resources/lxqt-lockscreen.desktop.in
    resources/lxqt-logout.desktop.in
    resources/lxqt-reboot.desktop.in
    resources/lxqt-shutdown.desktop.in
    resources/lxqt-suspend.desktop.in
)

lxqt_translate_ts(lxqt-leave_QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${CPP_FILES}
        ${UI_FILES}
        ${H_FILES}
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_app_translation_loader(lxqt-leave_QM_LOADER ${PROJECT_NAME})
#************************************************


add_executable(lxqt-leave
    ${CPP_FILES}
    ${UI_HEADERS}
    ${DESKTOP_FILES}
    ${lxqt-leave_QM_LOADER}
    ${lxqt-leave_QM_FILES}
)

target_link_libraries(lxqt-leave
    ${LXQT_LIBRARIES}
    ${LXQT_LIBRARY_NAME}
)

install(TARGETS lxqt-leave RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION share/applications)
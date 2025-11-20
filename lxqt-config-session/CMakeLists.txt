project("lxqt-config-session")


set(lxqt-sessioncfg_HDRS
    sessionconfigwindow.h
    basicsettings.h
    modulemodel.h
    autostartpage.h
    autostartmodel.h
    autostartitem.h
    autostartedit.h
    autostartutils.h
    environmentpage.h
    userlocationspage.h
    x11settings.h
    waylandsettings.h
)

set(lxqt-sessioncfg_SRCS
    main.cpp
    sessionconfigwindow.cpp
    basicsettings.cpp
    modulemodel.cpp
    autostartpage.cpp
    autostartmodel.cpp
    autostartitem.cpp
    autostartedit.cpp
    autostartutils.cpp
    environmentpage.cpp
    userlocationspage.cpp
    x11settings.cpp
    waylandsettings.cpp
    ../lxqt-session/src/windowmanager.cpp
)

set(lxqt-sessioncfg_UI_FILES
    basicsettings.ui
    autostartpage.ui
    autostartedit.ui
    environmentpage.ui
    x11settings.ui
    waylandsettings.ui
)

# Translations
lxqt_translate_ts(lxqt-sessioncfg_QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${lxqt-sessioncfg_HDRS}
        ${lxqt-sessioncfg_SRCS}
        ${lxqt-sessioncfg_UI_FILES}
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_app_translation_loader(lxqt-sessioncfg_QM_LOADER ${PROJECT_NAME})
lxqt_translate_desktop(lxqt-sessioncfg_DESKTOP_FILES SOURCES lxqt-config-session.desktop.in USE_YAML)

add_executable(lxqt-config-session
    ${lxqt-sessioncfg_SRCS}
    ${lxqt-sessioncfg_DESKTOP_FILES}
    ${lxqt-sessioncfg_QM_FILES}
    ${lxqt-sessioncfg_QM_LOADER})

target_link_libraries(lxqt-config-session
    KF6::WindowSystem
    Qt6::Widgets
    Qt6::DBus
    lxqt
)

INSTALL(TARGETS
    lxqt-config-session
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT Runtime
)

install(FILES
        ${lxqt-sessioncfg_DESKTOP_FILES}
    DESTINATION
         "${CMAKE_INSTALL_DATAROOTDIR}/applications"
    COMPONENT Runtime
)
install(FILES
    man/lxqt-config-session.1
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
    COMPONENT Runtime
)

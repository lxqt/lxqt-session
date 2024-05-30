# install default config files to /etc/xdg
install(FILES
    lxqt.conf
    session.conf
    windowmanagers.conf
    DESTINATION "${CMAKE_INSTALL_DATADIR}/lxqt"
    COMPONENT Runtime
)
install(FILES
    lxqt-hyprland.conf
    lxqt-wayfire.ini
    lxqt-sway.config
    DESTINATION "${CMAKE_INSTALL_DATADIR}/lxqt/wayland"
    COMPONENT Runtime
)
install(DIRECTORY
    labwc
    DESTINATION "${CMAKE_INSTALL_DATADIR}/lxqt/wayland"
    COMPONENT Runtime
)

# install default config files to /etc/xdg
install(FILES
    lxqt.conf
    session.conf
    windowmanagers.conf
    waylandwindowmanagers.conf
    DESTINATION "${CMAKE_INSTALL_DATADIR}/lxqt"
    COMPONENT Runtime
)

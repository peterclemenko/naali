macro (configure_qxmpp)
    if (NOT WIN32)
    sagase_configure_package (QXMPP
        NAMES qxmpp
        COMPONENTS qxmpp speex
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    sagase_configure_report (QXMPP)
    else ()
        # Unimplemented
    endif ()
endmacro (configure_qxmpp)

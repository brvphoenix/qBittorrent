# Executable target configuration
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
add_executable(qbt_app)

file(GLOB QBT_TS_FILES "${qBittorrent_SOURCE_DIR}/src/lang/*.ts")

if (BUILD_WITH_LANG)
    foreach (TS_FILE ${QBT_TS_FILES})
        get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
        string(REGEX REPLACE "^qbittorrent_" "" TS_LANG "${TS_NAME}")
        if(NOT TS_LANG IN_LIST BUILD_WITH_LANG)
            list(REMOVE_ITEM QBT_TS_FILES ${TS_FILE})
        endif()
    endforeach()
endif()

# Custom target to update .ts files and include new strings from source files
# Depends on Qt's LinguistTools
get_target_property(QT_LUPDATE_EXECUTABLE Qt::lupdate IMPORTED_LOCATION)
add_custom_target(qbt_update_translations
                    COMMAND ${QT_LUPDATE_EXECUTABLE} -extensions ui,c,c++,cc,cpp,cxx,ch,h,h++,hh,hpp,hxx
                                                     ${qBittorrent_SOURCE_DIR}
                                                     -ts ${QBT_TS_FILES}
                    WORKING_DIRECTORY "${qBittorrent_SOURCE_DIR}"
                    VERBATIM
                    COMMAND_EXPAND_LISTS)

# Generate and configure translation files
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# Based on https://gist.github.com/giraldeau/546ba5512a74dfe9d8ea0862d66db412
set_source_files_properties(${QBT_TS_FILES} PROPERTIES OUTPUT_LOCATION "${qBittorrent_BINARY_DIR}/src/lang")
qt_add_translation(QBT_QM_FILES ${QBT_TS_FILES} OPTIONS -silent)

qt_add_resources(qbt_app "lang"
    PREFIX "/lang"
    BASE ${qBittorrent_BINARY_DIR}/src/lang
    FILES ${QBT_QM_FILES}
)

if (WEBUI)
    file(GLOB QBT_WEBUI_TS_FILES "${qBittorrent_SOURCE_DIR}/src/webui/www/translations/*.ts")
    if (BUILD_WITH_LANG)
        foreach (TS_FILE ${QBT_WEBUI_TS_FILES})
            get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
            string(REGEX REPLACE "^webui_" "" TS_LANG "${TS_NAME}")
            if(NOT TS_LANG IN_LIST BUILD_WITH_LANG)
                list(REMOVE_ITEM QBT_WEBUI_TS_FILES ${TS_FILE})
            endif()
        endforeach()
    endif()
    set_source_files_properties(${QBT_WEBUI_TS_FILES}
        PROPERTIES OUTPUT_LOCATION "${qBittorrent_BINARY_DIR}/src/webui/www/translations")
    qt_add_translation(QBT_WEBUI_QM_FILES ${QBT_WEBUI_TS_FILES} OPTIONS -silent)
    qt_add_resources(qbt_app "webui_translation"
        PREFIX "/www/translations"
        BASE ${qBittorrent_BINARY_DIR}/src/webui/www/translations
        FILES ${QBT_WEBUI_QM_FILES}
    )
endif()

target_sources(qbt_app PRIVATE
    # headers
    application.h
    applicationinstancemanager.h
    cmdoptions.h
    filelogger.h
    qtlocalpeer/qtlocalpeer.h
    signalhandler.h
    upgrade.h

    # sources
    application.cpp
    applicationinstancemanager.cpp
    cmdoptions.cpp
    filelogger.cpp
    main.cpp
    qtlocalpeer/qtlocalpeer.cpp
    signalhandler.cpp
    upgrade.cpp

    # resources
    "${qBittorrent_SOURCE_DIR}/src/icons/icons.qrc"
    "${qBittorrent_SOURCE_DIR}/src/searchengine/searchengine.qrc"
)

target_link_libraries(qbt_app PRIVATE
    qbt_base
)

set_target_properties(qbt_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
if (GUI)
    set_target_properties(qbt_app PROPERTIES OUTPUT_NAME qbittorrent)
else()
    set_target_properties(qbt_app PROPERTIES OUTPUT_NAME qbittorrent-nox)
endif()

# Additional platform specific configuration
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    include(FindQtTranslations)
    qbt_get_qt_translations(QT_TRANSLATIONS)
    set_source_files_properties(${QT_TRANSLATIONS} PROPERTIES MACOSX_PACKAGE_LOCATION translations)
    set_source_files_properties(
        "${qBittorrent_SOURCE_DIR}/dist/mac/qt.conf"
        "${qBittorrent_SOURCE_DIR}/dist/mac/qBitTorrentDocument.icns"
        "${qBittorrent_SOURCE_DIR}/dist/mac/qbittorrent_mac.icns"
            PROPERTIES
                MACOSX_PACKAGE_LOCATION Resources
    )
    # provide variables for substitution in dist/mac/Info.plist
    get_target_property(EXECUTABLE_NAME qbt_app OUTPUT_NAME)
    # This variable name should be changed once qmake is no longer used. Refer to the discussion in PR #14813
    set(MACOSX_DEPLOYMENT_TARGET ${CMAKE_OSX_DEPLOYMENT_TARGET})
    set_target_properties(qbt_app PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_BUNDLE_NAME "qBittorrent"
        MACOSX_BUNDLE_INFO_PLIST ${qBittorrent_SOURCE_DIR}/dist/mac/Info.plist
    )
    target_sources(qbt_app PRIVATE
        ${QT_TRANSLATIONS}
        ${qBittorrent_SOURCE_DIR}/dist/mac/qt.conf
        ${qBittorrent_SOURCE_DIR}/dist/mac/qBitTorrentDocument.icns
        ${qBittorrent_SOURCE_DIR}/dist/mac/qbittorrent_mac.icns
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(qbt_app PROPERTIES WIN32_EXECUTABLE ON)
    if (MINGW)
        target_sources(qbt_app PRIVATE ${qBittorrent_SOURCE_DIR}/src/qbittorrent_mingw.rc)
    else()
        target_sources(qbt_app PRIVATE ${qBittorrent_SOURCE_DIR}/src/qbittorrent.rc)
    endif()
    target_sources(qbt_app PRIVATE ${qBittorrent_SOURCE_DIR}/src/qbittorrent.exe.manifest)
endif()

# Additional feature dependent configuration
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
if (GUI)
    target_link_libraries(qbt_app PRIVATE qbt_gui)
    if ((CMAKE_SYSTEM_NAME STREQUAL "Windows") OR (CMAKE_SYSTEM_NAME STREQUAL "Darwin"))
        qt_import_plugins(qbt_app INCLUDE Qt::QSvgIconPlugin Qt::QSvgPlugin)
    endif()
endif()

if (STACKTRACE)
    target_compile_definitions(qbt_app PRIVATE STACKTRACE)

    target_sources(qbt_app PRIVATE
        stacktrace.h
        stacktrace.cpp
    )

    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_compile_definitions(qbt_app PRIVATE BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED)
        target_link_options(qbt_app PUBLIC -rdynamic)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(qbt_app PUBLIC ${CMAKE_DL_LIBS})
        target_link_options(qbt_app PUBLIC -rdynamic)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if (MSVC)
            target_compile_options(qbt_app PRIVATE /Zi)
            target_link_options(qbt_app PUBLIC
                /DEBUG
                /PDBALTPATH:$<TARGET_PDB_FILE_NAME:qbt_app>
            )

            if (CMAKE_SIZEOF_VOID_P EQUAL 4)
                target_compile_options(qbt_app PRIVATE /Oy-)
            endif()
        else()
            target_link_options(qbt_app PUBLIC LINKER:--export-dynamic)

            if (CMAKE_SIZEOF_VOID_P EQUAL 4)
                target_compile_options(qbt_app PRIVATE -fno-omit-frame-pointer)
            endif()
        endif()
    endif()
endif()

if (WEBUI)
    target_link_libraries(qbt_app PRIVATE qbt_webui)
endif()

# Installation
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
install(TARGETS qbt_app
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE  DESTINATION .
    COMPONENT runtime
)

if (MSVC)
    install(FILES $<TARGET_PDB_FILE:qbt_app>
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        OPTIONAL
    )
endif()

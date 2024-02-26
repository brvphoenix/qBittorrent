# Executable target configuration
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
add_executable(qbt_app)

file(GLOB QBT_TS_FILES "${qBittorrent_SOURCE_DIR}/src/lang/*.ts")
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
# Based on https://doc.qt.io/qt-6/qtlinguist-cmake-qt-add-translation.html
set_source_files_properties(${QBT_TS_FILES} PROPERTIES OUTPUT_LOCATION "${qBittorrent_BINARY_DIR}/src/lang")
qt_add_translation(QBT_QM_FILES ${QBT_TS_FILES} OPTIONS -removeidentical -silent)

qt_add_resources(qbt_app "lang"
    PREFIX
        "/lang"
    BASE
        ${qBittorrent_BINARY_DIR}/src/lang
    FILES
        ${QBT_QM_FILES}
)

if (WEBUI)
    file(GLOB QBT_WEBUI_TS_FILES "${qBittorrent_SOURCE_DIR}/src/webui/www/translations/*.ts")
    set_source_files_properties(${QBT_WEBUI_TS_FILES}
        PROPERTIES OUTPUT_LOCATION "${qBittorrent_BINARY_DIR}/src/webui/www/translations")
    qt_add_translation(QBT_WEBUI_QM_FILES ${QBT_WEBUI_TS_FILES} OPTIONS -removeidentical -silent)
    qt_add_resources(qbt_app "webui_translation"
        PREFIX
            "/www/translations"
        BASE
            ${qBittorrent_BINARY_DIR}/src/webui/www/translations
        FILES
            ${QBT_WEBUI_QM_FILES}
    )
endif()

# icons
set(icons_resource_files
        "../icons/application-exit.svg"
        "../icons/application-rss.svg"
        "../icons/application-url.svg"
        "../icons/browser-cookies.svg"
        "../icons/chart-line.svg"
        "../icons/checked-completed.svg"
        "../icons/configure.svg"
        "../icons/connected.svg"
        "../icons/dialog-warning.svg"
        "../icons/directory.svg"
        "../icons/disconnected.svg"
        "../icons/download.svg"
        "../icons/downloading.svg"
        "../icons/edit-clear.svg"
        "../icons/edit-copy.svg"
        "../icons/edit-find.svg"
        "../icons/edit-rename.svg"
        "../icons/error.svg"
        "../icons/fileicon.svg"
        "../icons/filter-active.svg"
        "../icons/filter-all.svg"
        "../icons/filter-inactive.svg"
        "../icons/filter-stalled.svg"
        "../icons/firewalled.svg"
        "../icons/flags/ac.svg"
        "../icons/flags/ad.svg"
        "../icons/flags/ae.svg"
        "../icons/flags/af.svg"
        "../icons/flags/ag.svg"
        "../icons/flags/ai.svg"
        "../icons/flags/al.svg"
        "../icons/flags/am.svg"
        "../icons/flags/ao.svg"
        "../icons/flags/aq.svg"
        "../icons/flags/ar.svg"
        "../icons/flags/arab.svg"
        "../icons/flags/as.svg"
        "../icons/flags/at.svg"
        "../icons/flags/au.svg"
        "../icons/flags/aw.svg"
        "../icons/flags/ax.svg"
        "../icons/flags/az.svg"
        "../icons/flags/ba.svg"
        "../icons/flags/bb.svg"
        "../icons/flags/bd.svg"
        "../icons/flags/be.svg"
        "../icons/flags/bf.svg"
        "../icons/flags/bg.svg"
        "../icons/flags/bh.svg"
        "../icons/flags/bi.svg"
        "../icons/flags/bj.svg"
        "../icons/flags/bl.svg"
        "../icons/flags/bm.svg"
        "../icons/flags/bn.svg"
        "../icons/flags/bo.svg"
        "../icons/flags/bq.svg"
        "../icons/flags/br.svg"
        "../icons/flags/bs.svg"
        "../icons/flags/bt.svg"
        "../icons/flags/bv.svg"
        "../icons/flags/bw.svg"
        "../icons/flags/by.svg"
        "../icons/flags/bz.svg"
        "../icons/flags/ca.svg"
        "../icons/flags/cc.svg"
        "../icons/flags/cd.svg"
        "../icons/flags/cefta.svg"
        "../icons/flags/cf.svg"
        "../icons/flags/cg.svg"
        "../icons/flags/ch.svg"
        "../icons/flags/ci.svg"
        "../icons/flags/ck.svg"
        "../icons/flags/cl.svg"
        "../icons/flags/cm.svg"
        "../icons/flags/cn.svg"
        "../icons/flags/co.svg"
        "../icons/flags/cp.svg"
        "../icons/flags/cr.svg"
        "../icons/flags/cu.svg"
        "../icons/flags/cv.svg"
        "../icons/flags/cw.svg"
        "../icons/flags/cx.svg"
        "../icons/flags/cy.svg"
        "../icons/flags/cz.svg"
        "../icons/flags/de.svg"
        "../icons/flags/dg.svg"
        "../icons/flags/dj.svg"
        "../icons/flags/dk.svg"
        "../icons/flags/dm.svg"
        "../icons/flags/do.svg"
        "../icons/flags/dz.svg"
        "../icons/flags/eac.svg"
        "../icons/flags/ec.svg"
        "../icons/flags/ee.svg"
        "../icons/flags/eg.svg"
        "../icons/flags/eh.svg"
        "../icons/flags/er.svg"
        "../icons/flags/es-ct.svg"
        "../icons/flags/es-ga.svg"
        "../icons/flags/es-pv.svg"
        "../icons/flags/es.svg"
        "../icons/flags/et.svg"
        "../icons/flags/eu.svg"
        "../icons/flags/fi.svg"
        "../icons/flags/fj.svg"
        "../icons/flags/fk.svg"
        "../icons/flags/fm.svg"
        "../icons/flags/fo.svg"
        "../icons/flags/fr.svg"
        "../icons/flags/ga.svg"
        "../icons/flags/gb-eng.svg"
        "../icons/flags/gb-nir.svg"
        "../icons/flags/gb-sct.svg"
        "../icons/flags/gb-wls.svg"
        "../icons/flags/gb.svg"
        "../icons/flags/gd.svg"
        "../icons/flags/ge.svg"
        "../icons/flags/gf.svg"
        "../icons/flags/gg.svg"
        "../icons/flags/gh.svg"
        "../icons/flags/gi.svg"
        "../icons/flags/gl.svg"
        "../icons/flags/gm.svg"
        "../icons/flags/gn.svg"
        "../icons/flags/gp.svg"
        "../icons/flags/gq.svg"
        "../icons/flags/gr.svg"
        "../icons/flags/gs.svg"
        "../icons/flags/gt.svg"
        "../icons/flags/gu.svg"
        "../icons/flags/gw.svg"
        "../icons/flags/gy.svg"
        "../icons/flags/hk.svg"
        "../icons/flags/hm.svg"
        "../icons/flags/hn.svg"
        "../icons/flags/hr.svg"
        "../icons/flags/ht.svg"
        "../icons/flags/hu.svg"
        "../icons/flags/ic.svg"
        "../icons/flags/id.svg"
        "../icons/flags/ie.svg"
        "../icons/flags/il.svg"
        "../icons/flags/im.svg"
        "../icons/flags/in.svg"
        "../icons/flags/io.svg"
        "../icons/flags/iq.svg"
        "../icons/flags/ir.svg"
        "../icons/flags/is.svg"
        "../icons/flags/it.svg"
        "../icons/flags/je.svg"
        "../icons/flags/jm.svg"
        "../icons/flags/jo.svg"
        "../icons/flags/jp.svg"
        "../icons/flags/ke.svg"
        "../icons/flags/kg.svg"
        "../icons/flags/kh.svg"
        "../icons/flags/ki.svg"
        "../icons/flags/km.svg"
        "../icons/flags/kn.svg"
        "../icons/flags/kp.svg"
        "../icons/flags/kr.svg"
        "../icons/flags/kw.svg"
        "../icons/flags/ky.svg"
        "../icons/flags/kz.svg"
        "../icons/flags/la.svg"
        "../icons/flags/lb.svg"
        "../icons/flags/lc.svg"
        "../icons/flags/li.svg"
        "../icons/flags/lk.svg"
        "../icons/flags/lr.svg"
        "../icons/flags/ls.svg"
        "../icons/flags/lt.svg"
        "../icons/flags/lu.svg"
        "../icons/flags/lv.svg"
        "../icons/flags/ly.svg"
        "../icons/flags/ma.svg"
        "../icons/flags/mc.svg"
        "../icons/flags/md.svg"
        "../icons/flags/me.svg"
        "../icons/flags/mf.svg"
        "../icons/flags/mg.svg"
        "../icons/flags/mh.svg"
        "../icons/flags/mk.svg"
        "../icons/flags/ml.svg"
        "../icons/flags/mm.svg"
        "../icons/flags/mn.svg"
        "../icons/flags/mo.svg"
        "../icons/flags/mp.svg"
        "../icons/flags/mq.svg"
        "../icons/flags/mr.svg"
        "../icons/flags/ms.svg"
        "../icons/flags/mt.svg"
        "../icons/flags/mu.svg"
        "../icons/flags/mv.svg"
        "../icons/flags/mw.svg"
        "../icons/flags/mx.svg"
        "../icons/flags/my.svg"
        "../icons/flags/mz.svg"
        "../icons/flags/na.svg"
        "../icons/flags/nc.svg"
        "../icons/flags/ne.svg"
        "../icons/flags/nf.svg"
        "../icons/flags/ng.svg"
        "../icons/flags/ni.svg"
        "../icons/flags/nl.svg"
        "../icons/flags/no.svg"
        "../icons/flags/np.svg"
        "../icons/flags/nr.svg"
        "../icons/flags/nu.svg"
        "../icons/flags/nz.svg"
        "../icons/flags/om.svg"
        "../icons/flags/pa.svg"
        "../icons/flags/pe.svg"
        "../icons/flags/pf.svg"
        "../icons/flags/pg.svg"
        "../icons/flags/ph.svg"
        "../icons/flags/pk.svg"
        "../icons/flags/pl.svg"
        "../icons/flags/pm.svg"
        "../icons/flags/pn.svg"
        "../icons/flags/pr.svg"
        "../icons/flags/ps.svg"
        "../icons/flags/pt.svg"
        "../icons/flags/pw.svg"
        "../icons/flags/py.svg"
        "../icons/flags/qa.svg"
        "../icons/flags/re.svg"
        "../icons/flags/ro.svg"
        "../icons/flags/rs.svg"
        "../icons/flags/ru.svg"
        "../icons/flags/rw.svg"
        "../icons/flags/sa.svg"
        "../icons/flags/sb.svg"
        "../icons/flags/sc.svg"
        "../icons/flags/sd.svg"
        "../icons/flags/se.svg"
        "../icons/flags/sg.svg"
        "../icons/flags/sh.svg"
        "../icons/flags/si.svg"
        "../icons/flags/sj.svg"
        "../icons/flags/sk.svg"
        "../icons/flags/sl.svg"
        "../icons/flags/sm.svg"
        "../icons/flags/sn.svg"
        "../icons/flags/so.svg"
        "../icons/flags/sr.svg"
        "../icons/flags/ss.svg"
        "../icons/flags/st.svg"
        "../icons/flags/sv.svg"
        "../icons/flags/sx.svg"
        "../icons/flags/sy.svg"
        "../icons/flags/sz.svg"
        "../icons/flags/ta.svg"
        "../icons/flags/tc.svg"
        "../icons/flags/td.svg"
        "../icons/flags/tf.svg"
        "../icons/flags/tg.svg"
        "../icons/flags/th.svg"
        "../icons/flags/tj.svg"
        "../icons/flags/tk.svg"
        "../icons/flags/tl.svg"
        "../icons/flags/tm.svg"
        "../icons/flags/tn.svg"
        "../icons/flags/to.svg"
        "../icons/flags/tr.svg"
        "../icons/flags/tt.svg"
        "../icons/flags/tv.svg"
        "../icons/flags/tw.svg"
        "../icons/flags/tz.svg"
        "../icons/flags/ua.svg"
        "../icons/flags/ug.svg"
        "../icons/flags/um.svg"
        "../icons/flags/un.svg"
        "../icons/flags/us.svg"
        "../icons/flags/uy.svg"
        "../icons/flags/uz.svg"
        "../icons/flags/va.svg"
        "../icons/flags/vc.svg"
        "../icons/flags/ve.svg"
        "../icons/flags/vg.svg"
        "../icons/flags/vi.svg"
        "../icons/flags/vn.svg"
        "../icons/flags/vu.svg"
        "../icons/flags/wf.svg"
        "../icons/flags/ws.svg"
        "../icons/flags/xk.svg"
        "../icons/flags/xx.svg"
        "../icons/flags/ye.svg"
        "../icons/flags/yt.svg"
        "../icons/flags/za.svg"
        "../icons/flags/zm.svg"
        "../icons/flags/zw.svg"
        "../icons/folder-documents.svg"
        "../icons/folder-new.svg"
        "../icons/folder-remote.svg"
        "../icons/force-recheck.svg"
        "../icons/go-bottom.svg"
        "../icons/go-down.svg"
        "../icons/go-top.svg"
        "../icons/go-up.svg"
        "../icons/hash.svg"
        "../icons/help-about.svg"
        "../icons/help-contents.svg"
        "../icons/insert-link.svg"
        "../icons/ip-blocked.svg"
        "../icons/list-add.svg"
        "../icons/list-remove.svg"
        "../icons/loading.svg"
        "../icons/mail-inbox.svg"
        "../icons/mascot.png"
        "../icons/name.svg"
        "../icons/network-connect.svg"
        "../icons/network-server.svg"
        "../icons/object-locked.svg"
        "../icons/peers-add.svg"
        "../icons/peers-remove.svg"
        "../icons/peers.svg"
        "../icons/plugins.svg"
        "../icons/preferences-advanced.svg"
        "../icons/preferences-bittorrent.svg"
        "../icons/preferences-desktop.svg"
        "../icons/preferences-webui.svg"
        "../icons/qbittorrent-tray-dark.svg"
        "../icons/qbittorrent-tray-light.svg"
        "../icons/qbittorrent-tray.svg"
        "../icons/qbittorrent.ico"
        "../icons/qbittorrent_file.ico"
        "../icons/queued.svg"
        "../icons/ratio.svg"
        "../icons/reannounce.svg"
        "../icons/security-high.svg"
        "../icons/security-low.svg"
        "../icons/set-location.svg"
        "../icons/slow.svg"
        "../icons/slow_off.svg"
        "../icons/speedometer.svg"
        "../icons/splash.png"
        "../icons/stalledDL.svg"
        "../icons/stalledUP.svg"
        "../icons/stopped.svg"
        "../icons/system-log-out.svg"
        "../icons/tags.svg"
        "../icons/task-complete.svg"
        "../icons/task-reject.svg"
        "../icons/torrent-creator.svg"
        "../icons/torrent-magnet.svg"
        "../icons/torrent-start-forced.svg"
        "../icons/torrent-start.svg"
        "../icons/torrent-stop.svg"
        "../icons/tracker-error.svg"
        "../icons/tracker-warning.svg"
        "../icons/trackerless.svg"
        "../icons/trackers.svg"
        "../icons/upload.svg"
        "../icons/view-categories.svg"
        "../icons/view-preview.svg"
        "../icons/view-refresh.svg"
        "../icons/view-statistics.svg"
        "../icons/wallet-open.svg"
        "../icons/qbittorrent-tray.svg"
)

set_source_files_properties(../icons/qbittorrent-tray.svg PROPERTIES
    QT_RESOURCE_ALIAS ../icons/qbittorrent.svg
)

qt_add_resources(qbt_app "icons"
    PREFIX
        "/icons"
    FILES
        ${icons_resource_files}
)

target_sources(qbt_app PRIVATE
    # headers
    application.h
    applicationinstancemanager.h
    cmdoptions.h
    filelogger.h
    legalnotice.h
    qtlocalpeer/qtlocalpeer.h
    signalhandler.h
    upgrade.h

    # sources
    application.cpp
    applicationinstancemanager.cpp
    cmdoptions.cpp
    filelogger.cpp
    legalnotice.cpp
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

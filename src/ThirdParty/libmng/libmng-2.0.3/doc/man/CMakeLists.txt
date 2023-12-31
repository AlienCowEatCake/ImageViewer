#libmng-VERSION/doc/man
#
SET(SRC ${CMAKE_SOURCE_DIR}/doc/man)
SET(DST ${CMAKE_BINARY_DIR}/doc/man)
#
SET(MAN_3
libmng.3
)
SET(MAN_5
jng.5
mng.5
)
#
SET(MAN_3_gz "")
SET(MAN_5_gz "")
#
IF(GZIP_FOUND)
#
  FOREACH(man ${MAN_3} )
    ADD_CUSTOM_COMMAND(OUTPUT ${man}.gz
     COMMAND ${GZIP_EXE} -f -c "${SRC}/${man}" > "${DST}/${man}.gz"
     DEPENDS ${man}
     COMMENT "Building ${man}.gz")
    SET(MAN_3_gz ${DST}/${man}.gz ${MAN_3_gz})
  ENDFOREACH(man)
#
  FOREACH(man ${MAN_5} )
    ADD_CUSTOM_COMMAND(OUTPUT ${man}.gz
     COMMAND ${GZIP_EXE} -f -c "${SRC}/${man}" > "${DST}/${man}.gz"
     DEPENDS ${man}
     COMMENT "Building ${man}.gz")
    SET(MAN_5_gz ${DST}/${man}.gz ${MAN_5_gz})
  ENDFOREACH(man)
#
ELSE(GZIP_FOUND)
#
  FOREACH(man ${MAN_3} )
    SET(MAN_3_gz ${SRC}/${man} ${MAN_3_gz})
  ENDFOREACH(man)
#
  FOREACH(man ${MAN_5} )
    SET(MAN_5_gz ${SRC}/${man} ${MAN_5_gz})
  ENDFOREACH(man)
#
ENDIF(GZIP_FOUND)
#
ADD_CUSTOM_TARGET(doc ALL
 DEPENDS ${MAN_3_gz} ${MAN_5_gz} )
#
INSTALL(FILES ${MAN_3_gz}
  DESTINATION ${MNG_INSTALL_SHARE_DIR}/man/man3)
INSTALL(FILES ${MAN_5_gz}
  DESTINATION ${MNG_INSTALL_SHARE_DIR}/man/man5)
#

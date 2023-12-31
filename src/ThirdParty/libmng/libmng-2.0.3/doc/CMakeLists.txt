#libmng-VERSION/doc
IF(BUILD_MAN)
 IF(MAN_FOUND)
  ADD_SUBDIRECTORY(man)
 ELSE(MAN_FOUND)
  MESSAGE(STATUS "No MAN program found. MAN pages not built.")
 ENDIF(MAN_FOUND)
ENDIF(BUILD_MAN)

INSTALL(FILES
 Plan1.png  Plan2.png  doc.readme  libmng.txt
 DESTINATION ${MNG_INSTALL_DOC_DIR} )
#--- fin

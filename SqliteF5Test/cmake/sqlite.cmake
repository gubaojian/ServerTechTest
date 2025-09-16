
## https://github.com/gubaojian/zlibcompile/tree/main/pc/sqlite/3.50.4.1
if(EXISTS "${CDEPS_DIR}")
  message("try find sqlite in ${CDEPS_DIR}/sqlite/3.50.4/")
else ()
  message(ERROR "please include cmake/cdeps.cmake first")
endif ()

list(APPEND CDEPS_INCLUDE_DIRS
        "${CDEPS_DIR}/sqlite/3.50.4/include"
)
list(APPEND CDEPS_LIBS_DIRS
        "${CDEPS_DIR}/sqlite/3.50.4/lib/libsqlite3.a"
)


list(APPEND DEBUG_SOURCE_DIRS
        "${CDEPS_SOURCE_DIR}/sqlite/3.50.4/sqlite-src-3500400"
        "${CDEPS_SOURCE_DIR}/sqlite/3.50.4/sqlite-src-3500400/src"
        "${CDEPS_SOURCE_DIR}/sqlite/3.50.4/sqlite-src-3500400/ext/fts5"
)


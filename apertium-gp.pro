SUBDIRS += src
TEMPLATE = subdirs
CONFIG += ordered warn_on qt debug_and_release c++11 thread precompile_header

MY_QMAKE_CACHE=$${OUT_PWD}/.qmake.cache
!exists($$MY_QMAKE_CACHE) : {
system(echo QMAKEFEATURES=$$PWD/src/features> $$MY_QMAKE_CACHE)
}
unset(MY_QMAKE_CACHE)

INCLUDE_DIRECTORIES(
)

SET(TRAVERSO_SLV2_SOURCES
src/collections.c
src/plugin.c
src/pluginclass.c
src/plugininstance.c
src/plugins.c
src/pluginui.c
src/pluginuiinstance.c
src/port.c
src/query.c
src/scalepoint.c
src/util.c
src/value.c
src/world.c
)




SET(CMAKE_C_FLAGS_DEBUG "-std=c99")
SET(CMAKE_C_FLAGS_RELEASE "-std=c99")

SET(TRAVERSO_SLV2_LIBRARY "traversoslv2")


ADD_LIBRARY(${TRAVERSO_SLV2_LIBRARY} STATIC ${TRAVERSO_SLV2_SOURCES})


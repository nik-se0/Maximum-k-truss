set(target ${MP2_graphIO})

file(GLOB hdrs "*.h*")
file(GLOB srcs "*.c")

add_library(${target} STATIC ${srcs} ${hdrs})
target_link_libraries(${target} ${LIBRARY_DEPS})
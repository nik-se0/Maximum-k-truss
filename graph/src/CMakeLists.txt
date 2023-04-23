set(target ${MP2_LIBRARY})

file(GLOB hdrs "*.h*" "${MP2_INCLUDE}/*.h*")
file(GLOB srcs "*.cpp")
file(GLOB mat "*.mtx")

add_library(${target} STATIC ${srcs} ${hdrs} ${mat})
target_link_libraries(${target} ${LIBRARY_DEPS} ${MP2_graphIO})
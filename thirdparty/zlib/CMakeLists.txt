add_library(zlib STATIC IMPORTED GLOBAL)

if(WIN32)
    set_target_properties(zlib PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/win/libs/libzlib.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/win/include/zlib
    )
elseif(APPLE)
    set_target_properties(zlib PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/mac/libs/libz.a
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/mac/include/zlib
    )
endif()
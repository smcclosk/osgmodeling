SET(DATA_FILES
    ${PROJECT_SOURCE_DIR}/data/osg_banner.jpg
    ${PROJECT_SOURCE_DIR}/data/osg_banner2.jpg
    ${PROJECT_SOURCE_DIR}/data/curtain_flip.jpg
    ${PROJECT_SOURCE_DIR}/data/land_shallow_topo_2048_flip.jpg
)

INSTALL(FILES ${DATA_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

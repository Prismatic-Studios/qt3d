QT_FOR_CONFIG += 3dcore-private
qtConfig(qt3d-system-assimp):!if(cross_compile:host_build) {
    QMAKE_USE_PRIVATE += assimp
} else {
    include(assimp.pri)
}

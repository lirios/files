import qbs 1.0

LiriQmlPlugin {
    name: "archivesplugin"
    pluginPath: "Liri/Files/Archives"

    Depends { name: "lirideployment" }
    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["qml", "quick"] }

    cpp.defines: []

    files: ["*.cpp", "*.h", "qmldir", "*.qml"]
}

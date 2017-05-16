import qbs 1.0

LiriDynamicLibrary {
    name: "Liri.Files.Archives"
    targetName: "archivesplugin"

    Depends { name: "lirideployment" }
    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["qml", "quick"] }

    cpp.defines: []

    files: ["*.cpp", "*.h"]

    Group {
        name: "QML Files"
        files: [
            "*.qml",
            "qmldir"
        ]
        fileTags: ["qml"]
    }

    Group {
        qbs.install: true
        qbs.installDir: lirideployment.qmlDir + "/Liri/Files/Archives"
        fileTagsFilter: ["dynamiclibrary", "qml"]
    }
}

import qbs 1.0

LiriDynamicLibrary {
    name: "Liri.Files.PlacesModel"
    targetName: "placesmodelplugin"

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
        qbs.installDir: lirideployment.qmlDir + "/Liri/Files/PlacesModel"
        fileTagsFilter: ["dynamiclibrary", "qml"]
    }
}

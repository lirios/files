import qbs 1.0

LiriDynamicLibrary {
    name: "Liri.Files.FolderListModel"
    targetName: "folderlistmodelplugin"

    Depends { name: "qbsbuildconfig" }
    Depends { name: "lirideployment" }
    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["widgets", "qml", "quick"] }
    Depends { name: "taglib"; condition: qbsbuildconfig.enableTaglib }

    cpp.defines: {
        var defines = [];
        if (qbsbuildconfig.enableTaglib && !taglib.found)
            defines.push("DO_NOT_USE_TAG_LIB");
        return defines;
    }
    cpp.includePaths: base.concat(["disk", "trash"])

    files: ["*.cpp", "*.h", "disk/*.cpp", "disk/*.h", "trash/*.cpp", "trash/*.h"]

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
        qbs.installDir: lirideployment.qmlDir + "/Liri/Files/FolderListModel"
        fileTagsFilter: ["dynamiclibrary", "qml"]
    }
}

import qbs 1.0

LiriQmlPlugin {
    name: "folderlistmodelplugin"
    pluginPath: "Liri/Files/FolderListModel"

    Depends { name: "qbsbuildconfig" }
    Depends { name: "Qt"; submodules: ["widgets"] }
    Depends { name: "taglib"; condition: qbsbuildconfig.enableTaglib }

    cpp.defines: {
        var defines = [];
        if (qbsbuildconfig.enableTaglib && !taglib.found)
            defines.push("DO_NOT_USE_TAG_LIB");
        return defines;
    }
    cpp.includePaths: base.concat(["disk", "trash"])

    files: ["**/*.cpp", "**/*.h", "qmldir", "*.qml"]
}

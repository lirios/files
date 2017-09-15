import qbs 1.0

LiriQmlPlugin {
    name: "folderlistmodelplugin"
    pluginPath: "Liri/Files/FolderListModel"

    Depends { name: "Qt"; submodules: ["widgets"] }
    Depends { name: "taglib"; condition: project.enableTaglib }

    cpp.defines: {
        var defines = [];
        if (project.enableTaglib && !taglib.found)
            defines.push("DO_NOT_USE_TAG_LIB");
        return defines;
    }
    cpp.includePaths: base.concat(["disk", "trash"])

    files: ["**/*.cpp", "**/*.h", "qmldir", "*.qml"]
}

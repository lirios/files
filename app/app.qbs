import qbs 1.0

QtGuiApplication {
    name: "liri-files"
    targetName: "liri-files"

    Depends { name: "lirideployment" }
    Depends { name: "Qt"; submodules: ["qml", "quick", "quickcontrols2"] }

    cpp.defines: base.concat(['FILES_VERSION="' + project.version + '"'])

    Qt.core.resourcePrefix: "/"
    Qt.core.resourceSourceBase: sourceDirectory

    files: ["*.cpp", "*.h"]

    Group {
        name: "Resource Data"
        files: ["qml/**"]
        fileTags: ["qt.core.resource_data"]
    }

    Group {
        qbs.install: true
        qbs.installDir: lirideployment.binDir
        fileTagsFilter: product.type
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Desktop File"
        files: ["io.liri.Files.desktop"]
        qbs.install: true
        qbs.installDir: lirideployment.applicationsDir
    }
}

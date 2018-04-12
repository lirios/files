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
        condition: qbs.targetOS.contains("unix") &&
                   !qbs.targetOS.contains("android") &&
                   !qbs.targetOS.contains("macos")
        name: "Desktop File"
        prefix: "../data/"
        files: ["io.liri.Files.desktop"]
        qbs.install: true
        qbs.installDir: lirideployment.applicationsDir
    }

    Group {
        condition: qbs.targetOS.contains("unix") &&
                   !qbs.targetOS.contains("android") &&
                   !qbs.targetOS.contains("macos")
        name: "AppStream Metadata"
        prefix: "../data/"
        files: ["io.liri.Files.appdata.xml"]
        qbs.install: true
        qbs.installDir: lirideployment.appDataDir
    }

    Group {
        qbs.install: true
        qbs.installDir: lirideployment.binDir
        fileTagsFilter: product.type
    }
}

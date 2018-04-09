import qbs 1.0

Product {
    name: "Data"

    Depends { name: "lirideployment" }

    Group {
        condition: qbs.targetOS.contains("unix") &&
                   !qbs.targetOS.contains("android") &&
                   !qbs.targetOS.contains("macos")
        name: "Desktop File"
        files: ["io.liri.Files.desktop"]
        qbs.install: true
        qbs.installDir: lirideployment.applicationsDir
    }
}

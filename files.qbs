import qbs 1.0

Project {
    name: "Files"

    readonly property string version: "0.1.0"

    minimumQbsVersion: "1.8.0"

    qbsSearchPaths: ["qbs/local"]

    references: [
        "app/app.qbs",
        "imports/archives/archives.qbs",
        "imports/folderlistmodel/folderlistmodel.qbs",
        "imports/placesmodel/placesmodel.qbs",
    ]
}

import qbs 1.0

LiriQmlPlugin {
    name: "placesmodelplugin"
    pluginPath: "Liri/Files/PlacesModel"

    cpp.defines: []

    files: ["*.cpp", "*.h", "qmldir", "*.qml"]
}

/*
* Files app - File manager for Papyros
* Copyright (C) 2015 Michael Spencer <sonrisesoftware@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
import QtQuick 2.2
import Material 0.1
import org.nemomobile.folderlistmodel 1.0
import com.ubuntu.Archives 0.1
import com.ubuntu.PlacesModel 0.1

Object {
    id: folderModel

    property string path: places.locationHome
    property string title: pathTitle(path)
    property string folder: pathName(path)

    property bool busy: model.awaitingResults

    property bool showHiddenFiles
    property string sortingMethod: "Name" // or "Date"
    property bool sortAscending

    property alias model: __model
    property alias places: __places

    property var fileTypes: {
        "application/x-virtualbox-vdi": "disk image",
        "text/plain": "Text",
        "application/x-compressed-tar": "Archive",
        "text/x-qml": "Source code",
        "text/x-python": "Script",
        "image/png": "Image",
        "image/jpeg": "Image",
    }

    onShowHiddenFilesChanged: {
        model.showHiddenFiles = folderListPage.showHiddenFiles
    }

    onSortingMethodChanged: {
        console.log("Sorting by: " + sortingMethod)
        if (sortingMethod === "Name") {
            model.sortBy = FolderListModel.SortByName
        } else if (sortingMethod === "Date") {
            model.sortBy = FolderListModel.SortByDate
        } else {
            // Something fatal happened!
            console.log("ERROR: Invalid sort type:", sortingMethod)
        }
    }

    onSortAscendingChanged: {
        console.log("Sorting ascending: " + sortAscending)

        if (sortAscending) {
            model.sortOrder = FolderListModel.SortAscending
        } else {
            model.sortOrder = FolderListModel.SortDescending
        }
    }

    function goTo(location) {
        // This allows us to enter "~" as a shortcut to the home folder
        // when entering a location on the Go To dialog
        path = location.replace("~", places.locationHome)

        refresh()
    }

    /* Go to last folder visited */
    function goBack() {
        model.goBack()
        path = model.path
    }

    function refresh() {
        model.refresh()
    }

    function pathAccessedDate() {
        console.log("calling method model.curPathAccessedDate()")
        return model.curPathAccessedDate()
    }

    function pathModifiedDate() {
        console.log("calling method model.curPathModifiedDate()")
        return model.curPathModifiedDate()
    }

    function pathIsWritable() {
        console.log("calling method model.curPathIsWritable()")
        return model.curPathIsWritable()
    }

    function fileType(type, description) {
        if (type in fileTypes) {
            description = fileTypes[type]
        } else {
            print(type)
        }

        return description.substring(0, 1).toUpperCase() +
               description.substring(1)
    }

    function pathTitle(folder) {
        if (folder === places.locationHome) {
            return i18n.tr("Home")
        } else if (folder === "/") {
            return i18n.tr("Device")
        } else {
            return basename(folder)
        }
    }

    function pathName(folder) {
        if (folder === "/") {
            return "/"
        } else {
            return basename(folder)
        }
    }

    function basename(folder) {
        // Returns the latest component (folder) of an absolute path
        // E.g. basename('/home/phablet/Música') returns 'Música'

        // Remove the last trailing '/' if there is one

        folder.replace(/\/$/, "")
        return folder.substr(folder.lastIndexOf('/') + 1)
    }

    function pathExists(path) {
        path = path.replace("~", model.homePath())

        if (path === '/')
        return true

        if(path.charAt(0) === '/') {
            console.log("Directory: " + path.substring(0, path.lastIndexOf('/')+1))
            repeaterModel.path = path.substring(0, path.lastIndexOf('/')+1)
            console.log("Sub dir: " + path.substring(path.lastIndexOf('/')+1))
            if (path.substring(path.lastIndexOf('/')+1) !== "" && !repeaterModel.cdIntoPath(path.substring(path.lastIndexOf('/')+1))) {
                return false
            } else {
                return true
            }
        } else {
            return false
        }
    }

    function getArchiveType(fileName) {
        var splitName = fileName.split(".")
        var fileExtension = splitName[splitName.length - 1]
        if (fileExtension === "zip") {
            return "zip"
        } else if (fileExtension === "tar") {
            return "tar"
        } else {
            return ""
        }
    }

    // TODO: Set onlyAllowedPaths for restricted user accounts
    FolderListModel {
        id: __model

        path: folderModel.path
        enableExternalFSWatcher: true

        // Properties to emulate a model entry for use by FileDetailsPopover
        property bool isDir: true
        property string fileName: pathName(model.path)
        //property string fileSize: i18n.tr("%1 file", "%1 files", folderListView.count).arg(folderListView.count)
        property bool isReadable: true
        property bool isExecutable: true

        Component.onCompleted: {
            // Add default allowed paths
            addAllowedDirectory(places.locationDocuments)
            addAllowedDirectory(places.locationDownloads)
            addAllowedDirectory(places.locationMusic)
            addAllowedDirectory(places.locationPictures)
            addAllowedDirectory(places.locationVideos)
        }
    }

    FolderListModel {
        id: repeaterModel
        path: folderModel.path

        onPathChanged: {
            console.log("Path changed to: " + repeaterModel.path)
        }
    }

    PlacesModel {
        id: __places
    }
}

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
import QtQuick.Layouts 1.1
import Material 0.1
import Material.Extras 0.1
import Material.ListItems 0.1 as ListItem

ListItem.Standard {
    id: listItem

    // TODO : select the right icon for file type.
    iconName: model.isDir ? "file/folder" : "awesome/file"
    text: folderModel.pathTitle(model.filePath)
    selected: selectedFile != undefined &&
              selectedFile.filePath == model.filePath

    onClicked: {
        if (model.isDir) {
            folderModel.goTo(model.filePath)
        } else {
            snackbar.open("Opening " + model.fileName)
            Qt.openUrlExternally(model.filePath)
        }
    }

    onPressAndHold: {
        if (selected)
            selectedFile = undefined
        else
            selectedFile = model
    }

    secondaryItem: RowLayout {
        height: parent.height - Units.dp(1)
        spacing: Units.dp(16)

        Label {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: Units.dp(100)

            elide: Text.ElideRight

            text: folderModel.fileType(model.mimeType,
                model.mimeTypeDescription)
                color: Theme.light.subTextColor
        }

        Label {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: Units.dp(100)

            elide: Text.ElideRight

            text: DateUtils.friendlyTime(model.modifiedDate, true)
            color: Theme.light.subTextColor
        }
    }
}

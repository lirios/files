/*
* Files app - File manager for Papyros
* Copyright (C) 2015 Michael Spencer <sonrisesoftware@gmail.com>
*               2015 Ricardo Vieira <ricardo.vieira@tecnico.ulisboa.pt>
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
    iconName: isDir ? "file/folder" : "awesome/file"
    text: folderModel.pathTitle(filePath)
    selected: isSelected

    secondaryItem: RowLayout {
        height: parent.height - Units.dp(1)
        spacing: Units.dp(16)

        Label {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: Units.dp(100)

            elide: Text.ElideRight

            text: folderModel.fileType(mimeType, mimeTypeDescription)
                color: Theme.light.subTextColor
        }

        Label {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: Units.dp(100)

            elide: Text.ElideRight

            text: DateUtils.friendlyTime(modifiedDate, true)
            color: Theme.light.subTextColor
        }
    }

    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            if (mouse.button == Qt.RightButton){
                selectionManager.toggleIndex(index);
                return;
            }
            if (isDir) {
                folderModel.goTo(filePath)
            } else {
                snackbar.open(qsTr("Opening ") + fileName)
                Qt.openUrlExternally(filePath)
            }
        }

        onPressAndHold: {
            pageStack.push(Qt.resolvedUrl("../SelectionPage.qml"));
            selectionManager.toggleIndex(index);
        }

    }
}

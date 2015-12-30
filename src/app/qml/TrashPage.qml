/*
* Files app - File manager for Papyros
* Copyright (C) 2015 Ricardo Vieira <ricardo.vieira@tecnico.ulisboa.pt>
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
import "components"

Page {
    id: folderPage

    title: qsTr("Trash")

    backAction: Action {
        iconName: "navigation/arrow_back"
        name: qsTr("Back")

        onTriggered: {
            selectionManager.clear()
            selectionManager.setMultiSelection(false)
            folderModel.goBack()
            folderPage.pop()
        }
    }

    actions: [
        Action {
            iconName: "content/remove_circle"
            name: qsTr("Empty trash")
            onTriggered: folderModel.model.emptyTrash()
        },
        Action {
            iconName: "action/restore"
            name: qsTr("Restore trash")
            onTriggered: folderModel.model.restoreTrash()
        }
    ]

    actionBar {
        elevation: 0
        backgroundColor: Palette.colors["green"]["500"]
        decorationColor: Palette.colors["green"]["700"]
    }

    FolderListView {
        anchors.fill: parent
        model: folderModel.model
        delegate: FileListItem {}
    }

    Keys.onEscapePressed: selectionManager.clear()

    Component.onCompleted: folderModel.model.goTrash()
}

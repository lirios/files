/*
* This file is part of Liri.
 *
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
import QtQuick.Controls.Material 2.0
import Fluid.Controls 1.0
import "components"

Page {
    id: folderPage

    title: qsTr("Select files...")

    leftAction: Action {
        iconName: "navigation/arrow_back"
        text: qsTr("Back")

        onTriggered: {
            selectionManager.clear()
            selectionManager.setMultiSelection(false)
            folderPage.pop()
        }
    }

    actions: [
        Action {
            iconName: "content/content_cut"
            text: qsTr("Cut")
            shortcut: StandardKey.Cut
            onTriggered: folderModel.model.cutSelection()
        },
        Action {
            iconName: "content/content_copy"
            text: qsTr("Copy")
            shortcut: StandardKey.Copy
            onTriggered: folderModel.model.copySelection()
        },
        Action {
            iconName: "content/content_paste"
            text: qsTr("Paste")
            shortcut: StandardKey.Paste
            onTriggered: folderModel.model.paste()
            enabled: folderModel.model.clipboardUrlsCounter
        },
        Action {
            iconName: "action/delete"
            text: qsTr("Move to Trash")
            shortcut: StandardKey.Delete
            onTriggered: folderModel.model.moveSelectionToTrash()
        },
        Action {
            iconName: "content/select_all"
            text: qsTr("Select all")
            shortcut: StandardKey.SelectAll
            onTriggered: selectionManager.selectAll()
        },
        Action {
            iconName: "content/clear"
            text: qsTr("Clear selection")
            shortcut: StandardKey.Deselect
            onTriggered: selectionManager.clear()
        }
    ]

    appBar {
        elevation: 0
        //backgroundColor: Material.color(Material.Grey, Material.Shade700)
        //decorationColor: Material.color(Material.Grey, Material.Shade800)
    }

    FolderListView {
        anchors.fill: parent
        model: folderModel.model
        delegate: SelectionListItem {}
    }

    Keys.onEscapePressed: selectionManager.clear()

    Component.onCompleted: selectionManager.setMultiSelection(true);
}

/*
* This file is part of Liri.
 *
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
import QtQuick.Controls.Material 2.0
import Fluid.Controls 1.0 as FluidControls
import "components"

FluidControls.Page {
    id: folderPage

    title: qsTr("Trash")

    leftAction: FluidControls.Action {
        icon.source: FluidControls.Utils.iconUrl("navigation/arrow_back")
        text: qsTr("Back")

        onTriggered: {
            selectionManager.clear()
            selectionManager.setMultiSelection(false)
            folderModel.goBack()
            folderPage.pop()
        }
    }

    actions: [
        FluidControls.Action {
            icon.source: FluidControls.Utils.iconUrl("content/remove_circle")
            text: qsTr("Empty trash")
            onTriggered: folderModel.model.emptyTrash()
        },
        FluidControls.Action {
            icon.source: FluidControls.Utils.iconUrl("action/restore")
            text: qsTr("Restore trash")
            onTriggered: folderModel.model.restoreTrash()
        }
    ]

    appBar {
        elevation: 0
        backgroundColor: Material.color(Material.Green, Material.Shade500)
        decorationColor: Material.color(Material.Green, Material.Shade700)
    }

    FolderListView {
        anchors.fill: parent
        model: folderModel.model
        delegate: FileListItem {}
    }

    Keys.onEscapePressed: selectionManager.clear()

    Component.onCompleted: folderModel.model.goTrash()
}

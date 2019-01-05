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
import QtQuick.Controls 2.1
import Fluid.Controls 1.1 as FluidControls

Dialog {
    title: qsTr("Settings")
    modal: true
    focus: true

    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    width: 400

    Column {
        id: settingsList
        anchors.left: parent.left
        anchors.right: parent.right
        CheckBox {
            id: hiddenCheck
            text: qsTr("Show hidden files")
            checked: folderModel.model.showHiddenFiles
            onClicked: folderModel.model.toggleShowHiddenFiles();

            // If the checkbox is clicked it loses the connection with
            // folderModel.model.showHiddenFiles so we need connect to the signal
            Connections {
                target: folderModel.model
                onShowHiddenFilesChanged:
                    hiddenCheck.checked = folderModel.model.showHiddenFiles
            }
        }
        FluidControls.ListItem {
            text: qsTr("Sort by:")
            rightItem: ComboBox {
                id: sortByMenu
                anchors.centerIn: parent
                model: [qsTr("Name"), qsTr("Date")]
                currentIndex: folderModel.model.sortBy
                onCurrentIndexChanged: folderModel.model.sortBy = currentIndex
            }
        }
        FluidControls.ListItem {
            text: qsTr("Sort order:")
            rightItem: ComboBox {
                id: sortOrderMenu
                anchors.centerIn: parent
                model: [qsTr("Ascending"), qsTr("Descending")]
                currentIndex: folderModel.model.sortOrder
                onCurrentIndexChanged: folderModel.model.sortOrder = currentIndex
            }
        }
    }
}

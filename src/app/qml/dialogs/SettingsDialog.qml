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
import Material.Extras 0.1
import Material.ListItems 0.1 as ListItem

Dialog {
    title: qsTr("Settings")

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
        ListItem.SimpleMenu {
            id: sortByMenu
            text: qsTr("Sort by:")
            model: [qsTr("Name"), qsTr("Date")]
            selectedIndex: folderModel.model.sortBy
            // Here we can't just toggle, we need to set the selected option
            onSelectedIndexChanged: folderModel.model.sortBy = selectedIndex

            Connections {
                target: folderModel.model
                onSortByChanged:
                    sortByMenu.selectedIndex = folderModel.model.sortBy
            }
        }
        ListItem.SimpleMenu {
            id: sortOrderMenu
            text: qsTr("Sort order:")
            model: [qsTr("Ascending"), qsTr("Descending")]
            onSelectedIndexChanged: folderModel.model.sortOrder = selectedIndex

            Connections {
                target: folderModel.model
                onSortOrderChanged:
                    sortOrderMenu.selectedIndex = folderModel.model.sortOrder
            }
        }
    }
}

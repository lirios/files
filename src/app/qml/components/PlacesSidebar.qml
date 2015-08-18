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
import QtGraphicalEffects 1.0
import Material 0.1
import Material.ListItems 0.1 as ListItem

Sidebar {
    id: placesSidebar

    Column {
        anchors {
            left: parent.left
            right: parent.right
        }

        ListItem.Subheader {
            text: qsTr("Places")
        }

        Repeater {
            model: folderModel.places

            delegate: ListItem.Standard {
                id: listItem

                action: Item {
                    anchors.fill: parent

                    Icon {
                        id: icon
                        anchors.centerIn: parent
                        name: folderModel.pathIcon(path)
                    }

                    ColorOverlay {
                        anchors.fill: icon
                        source: icon
                        color: Theme.primaryColor
                        visible: listItem.selected
                    }
                }
                text: folderModel.pathTitle(path)
                selected: folderModel.path == path

                onClicked: folderModel.goTo(path)
            }
        }
    }
}

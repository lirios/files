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
import QtGraphicalEffects 1.0
import Material 0.1
import Material.Extras 0.1
import Material.ListItems 0.1 as ListItem

PageSidebar {
    id: infoSidebar

    property var selectedFileIndex

    actionBar.backgroundColor: Palette.colors.blue["600"]
    width: Units.dp(320)

    showing: selectionManager.mode == 0 && selectionManager.counter == 1

    Connections {
        target: selectionManager
        onSelectionChanged: {
            selectedFileIndex = selectionManager.selectedIndexes()[0]
        }
    }

    actionBar.extendedContent: Item {
        height: Units.dp(72)
        width: parent.width

        ColumnLayout {
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width

            spacing: Units.dp(3)

            Label {
                Layout.fillWidth: true

                elide: Text.ElideRight

                text: infoSidebar.get_role_info("fileName")
                style: "subheading"
                color: Theme.dark.textColor
            }

            Label {
                Layout.fillWidth: true

                elide: Text.ElideRight
                text: qsTr("Edited ") + DateUtils.friendlyTime(infoSidebar.get_role_info("modifiedDate"))
                color: Theme.dark.subTextColor
            }
        }
    }

    actions: [
        Action {
            iconName: "social/share"
        },

        Action {
            iconName: "action/delete"
            onTriggered: confirmAction("", qsTr("Are you sure you want to permanently delete \"%1\"?")
                    .arg(infoSidebar.get_role_info("fileName")), qsTr("Delete")).done(function() {
                folderModel.model.removeIndex(selectedFileIndex)
            })
        }
    ]

    Column {
        anchors.fill: parent

        Image {
            fillMode: Image.PreserveAspectFit

            width: parent.width
            height: Math.min(width * sourceSize.height/sourceSize.width,
                             width)

            visible: infoSidebar.get_role_info("mimeType").indexOf("image/") == 0

            source: visible ? infoSidebar.get_role_info("filePath") : ""
        }

        ListItem.Subheader {
            text: qsTr("Info")
        }

        Item {
            id: infoItem

            height: infoGrid.height + Units.dp(16)
            width: parent.width

            GridLayout {
                id: infoGrid

                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - Units.dp(32)

                columns: 2
                columnSpacing: Units.dp(32)
                rowSpacing: Units.dp(16)

                Label {
                    text: qsTr("Location")
                }

                Label {
                    Layout.fillWidth: true

                    text: folderModel.path
                    color: Theme.light.subTextColor
                }

                Label {
                    text: qsTr("Type")
                }

                Label {
                    Layout.fillWidth: true

                    text: {
                        var description = infoSidebar.get_role_info("mimeTypeDescription")

                        return description.substring(0, 1).toUpperCase() +
                               description.substring(1)
                    }
                    color: Theme.light.subTextColor
                }

                Label {
                    text: infoSidebar.get_role_info("isDir")
                          ? qsTr("Contents") : qsTr("Size")
                }

                Label {
                    Layout.fillWidth: true

                    text:  infoSidebar.get_role_info("fileSize")
                    color: Theme.light.subTextColor
                }
            }
        }

        ThinDivider {}

    }

    function get_role_info(role) {
        return folderModel.model.data(infoSidebar.selectedFileIndex, role)
    }

    onShowingChanged: {
        if (showing)
            app.width = Math.max(app.width, Units.dp(1000))
    }
}

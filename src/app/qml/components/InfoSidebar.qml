/*
* This file is part of Liri.
 *
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
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.1
import Fluid.Core 1.0 as FluidCore
import Fluid.Controls 1.1 as FluidControls

FluidControls.PageSidebar {
    id: infoSidebar

    property var selectedFileIndex

    appBar.backgroundColor: Material.color(Material.Blue, Material.Shade600)

    width: 320

    appBar.extendedContent: Item {
        width: parent.width
        height: 72

        ColumnLayout {
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width

            spacing: 3

            FluidControls.SubheadingLabel {
                Layout.fillWidth: true

                elide: Text.ElideRight

                text: infoSidebar.get_role_info("fileName")
            }

            Label {
                Layout.fillWidth: true

                elide: Text.ElideRight
                text: qsTr("Edited ") + FluidCore.DateUtils.friendlyTime(infoSidebar.get_role_info("modifiedDate"), true)
                color: Material.secondaryTextColor
            }
        }
    }

    actions: [
        FluidControls.Action {
            icon.source: FluidControls.Utils.iconUrl("social/share")
            toolTip: qsTr("Share")
        },
        FluidControls.Action {
            icon.source: FluidControls.Utils.iconUrl("action/delete")
            toolTip: qsTr("Delete")
            onTriggered: {
                var callback = function() {
                    folderModel.model.removeIndex(selectedFileIndex);
                };
                confirmAction("", qsTr("Are you sure you want to permanently delete \"%1\"?")
                    .arg(infoSidebar.get_role_info("fileName")), qsTr("Delete"), callback);
            }
        }
    ]

    showing: selectionManager.mode === 0 && selectionManager.counter === 1

    Connections {
        target: selectionManager
        onSelectionChanged: {
            selectedFileIndex = selectionManager.selectedIndexes()[0];
        }
    }

    Column {
        anchors.fill: parent

        Image {
            fillMode: Image.PreserveAspectFit

            width: parent.width
            height: Math.min(width * sourceSize.height/sourceSize.width,
                             width)

            visible: infoSidebar.get_role_info("mimeType").indexOf("image/") === 0

            source: visible ? infoSidebar.get_role_info("filePath") : ""
        }

        FluidControls.Subheader {
            text: qsTr("Info")
        }

        Item {
            id: infoItem

            width: parent.width
            height: infoGrid.height + 16

            GridLayout {
                id: infoGrid

                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 32

                columns: 2
                columnSpacing: 32
                rowSpacing: 16

                Label {
                    text: qsTr("Location")
                }

                Label {
                    Layout.fillWidth: true

                    text: folderModel.path
                    color: Material.secondaryTextColor
                }

                Label {
                    text: qsTr("Type")
                }

                Label {
                    Layout.fillWidth: true

                    text: {
                        var description = infoSidebar.get_role_info("mimeTypeDescription");
                        if (!description)
                            return "";
                        return description.substring(0, 1).toUpperCase() +
                               description.substring(1);
                    }
                    color: Material.secondaryTextColor
                }

                Label {
                    text: infoSidebar.get_role_info("isDir")
                          ? qsTr("Contents") : qsTr("Size")
                }

                Label {
                    Layout.fillWidth: true

                    text: infoSidebar.get_role_info("fileSize") || ""
                    color: Material.secondaryTextColor
                }
            }
        }

        FluidControls.ThinDivider {}
    }

    function get_role_info(role) {
        var result = folderModel.model.data(infoSidebar.selectedFileIndex, role) || "";

        // Return valid URLs for filePath
        if (role === "filePath") {
            if (result.startsWith("/"))
                return "file://" + result;
        }

        return result;
    }
}

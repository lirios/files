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
import Fluid.Controls 1.0 as FluidControls

Item {
    id: folderListView

    property alias model: listView.model
    property alias delegate: listView.delegate

    Pane {
        id: header

        visible: listView.count > 0

        Material.elevation: 1

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        height: 48

        RowLayout {
            anchors {
                left: parent.left
                right: parent.right
                margins: 16
            }

            height: parent.height - 1
            spacing: 16

            Label {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true

                text: qsTr("Name")
                color: Material.secondaryTextColor
            }

            Label {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: 100

                text: qsTr("Type")
                color: Material.secondaryTextColor
            }

            Label {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: 100

                text: qsTr("Last modified")
                color: Material.secondaryTextColor
            }
        }
    }

    ListView {
        id: listView

        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }

        clip: true

        section.property: "isDir"
        section.criteria: ViewSection.FullString
        section.delegate: FluidControls.Subheader {
            text: section === "true" ? qsTr("Directories")
                                     : qsTr("Files")
        }

        ScrollBar.horizontal: ScrollBar {}
        ScrollBar.vertical: ScrollBar {}
    }

    Label {
        anchors.centerIn: parent

        text: qsTr("No files")
        color: Material.hintTextColor
        font.pixelSize: 25

        visible: listView.count == 0
    }
}

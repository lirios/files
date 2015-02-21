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
import Material 0.1
import Material.Extras 0.1
import Material.ListItems 0.1 as ListItem

Item {
    id: folderListView

    Rectangle {
        id: header

        z: 1
        color: Theme.backgroundColor

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        height: units.dp(48)

        ThinDivider {
            anchors.bottom: parent.bottom
        }

        RowLayout {
            anchors {
                left: parent.left
                right: parent.right
                margins: units.dp(16)
            }

            height: parent.height - units.dp(1)
            spacing: units.dp(16)

            Label {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true

                text: "Name"
                color: Theme.light.subTextColor
            }

            Label {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: units.dp(75)

                text: "Type"
                color: Theme.light.subTextColor
            }

            Label {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: units.dp(120)

                text: "Last modified"
                color: Theme.light.subTextColor
            }
        }
    }

    ListView {
        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }

        model: folderModel.model
        delegate: ListItem.Standard {
            property string path: folderModel.path + '/' + model.fileName

            RowLayout {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: units.dp(16)
                }

                height: parent.height - units.dp(1)
                spacing: units.dp(16)

                Label {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillWidth: true

                    text: folderModel.pathTitle(path)
                    style: "subheading"
                }

                Label {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: units.dp(75)

                    text: "Type"
                    color: Theme.light.subTextColor
                }

                Label {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: units.dp(120)

                    text: DateUtils.friendlyTime(model.modifiedDate, true)
                    color: Theme.light.subTextColor
                }
            }
        }
    }
}

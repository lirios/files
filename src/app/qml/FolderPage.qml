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
import Material 0.1
import Material.ListItems 0.1 as ListItem
import "backend"
import "components"

Page {
    id: folderPage

    actionBar.elevation: 0

    backAction: Action {
        iconName: "navigation/arrow_back"
        name: qsTr("Back")
        enabled: folderModel.canGoBack
        visible: true

        onTriggered: folderModel.goBack()
    }

    actions: [
        Action {
            iconName: "action/search"
            name: qsTr("Search")
        },

        Action {
            iconName: "action/list"
            name: qsTr("List mode")
        },

        Action {
            iconName: "content/add"
            name: qsTr("New folder")
        },

        Action {
            iconName: "action/visibility"
            name: qsTr("Properties")
        },

        Action {
            iconName: "action/open_in_new"
            name: qsTr("Open in Terminal")
        },

        Action {
            iconName: "action/settings"
            name: qsTr("Settings")
        }
    ]

    actionBar.customContent: Label {
        anchors.bottom: parent.bottom

        text: folderModel.title

        style: "title"
        color: Theme.dark.textColor
        elide: Text.ElideRight

        width: parent.width
        height: infoSidebar.showing ? Units.dp(72) : actionBar.implicitHeight

        verticalAlignment: Text.AlignVCenter

        Behavior on height {
            NumberAnimation { duration: MaterialAnimation.pageTransitionDuration }
        }
    }

    rightSidebar: InfoSidebar {
        id: infoSidebar
    }

    property var selectedFile

    onSelectedFileChanged: {
        if (selectedFile)
            app.width = Math.max(app.width, Units.dp(1000))
    }

    FolderListView {
        anchors {
            left: placesSidebar.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }

        Snackbar {
            id: snackbar
        }
    }

    PlacesSidebar {
        id: placesSidebar
    }

    FolderModel {
        id: folderModel
    }

    Keys.onEscapePressed: selectedFile = undefined
}

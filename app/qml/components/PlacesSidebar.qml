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
import Fluid.Controls 1.0 as FluidControls

FluidControls.Sidebar {
    id: placesSidebar

    Column {
        anchors {
            left: parent.left
            right: parent.right
        }

        FluidControls.Subheader {
            text: qsTr("Places")
        }

        Repeater {
            model: folderModel.places

            delegate: FluidControls.ListItem {
                id: listItem

                icon.source: FluidControls.Utils.iconUrl(folderModel.pathIcon(path))
                text: folderModel.pathTitle(path)
                highlighted: folderModel.path == path

                onClicked: folderModel.goTo(path)
            }
        }

        FluidControls.ListItem {
            id: trashItem

            icon.source: FluidControls.Utils.iconUrl("action/delete")
            text: qsTr("Trash")

            onClicked: pageStack.push(Qt.resolvedUrl("../TrashPage.qml"));
        }
    }
}

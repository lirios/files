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
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import Fluid.Core 1.0 as FluidCore
import Fluid.Controls 1.0 as FluidControls

FluidControls.ListItem {
    id: listItem

    // TODO : select the right icon for file type.
    icon.source: FluidControls.Utils.iconUrl(isDir ? "file/folder" : "editor/insert_drive_file")
    text: folderModel.pathTitle(filePath)
    subText: folderModel.fileType(mimeType, mimeTypeDescription)
    valueText: FluidCore.DateUtils.friendlyTime(modifiedDate, true)
    highlighted: isSelected

    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            if (mouse.button === Qt.RightButton)
                selectionManager.toggleIndex(index);
            else
                folderModel.model.openIndex(index);
        }

        onPressAndHold: {
            pageStack.push(Qt.resolvedUrl("../SelectionPage.qml"));
            selectionManager.toggleIndex(index);
        }

    }
}

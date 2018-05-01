/*
* This file is part of Liri.
 *
* Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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
import QtQuick.Controls.Material 2.0
import Fluid.Core 1.0 as FluidCore
import Fluid.Controls 1.1 as FluidControls

FluidControls.ListItem {
    id: listItem

    // TODO : select the right icon for file type.
    text: folderModel.pathTitle(filePath)

    onClicked: {
        selectionManager.toggleIndex(index);
    }

    leftItem: CheckBox {
        id: checkBox
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
        }
        checked: isSelected
        enabled: false
    }

    secondaryItem: Row {
        spacing: 16

        Label {
            width: 100
            text: folderModel.fileType(mimeType, mimeTypeDescription)
            elide: Text.ElideRight
            color: Material.secondaryTextColor
        }

        Label {
            width: 100
            text: FluidCore.DateUtils.friendlyTime(modifiedDate, true)
            elide: Text.ElideRight
            color: Material.secondaryTextColor
        }
    }
}

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
import Material.Extras 0.1
import Material.ListItems 0.1 as ListItem

ApplicationWindow {
    id: app

    title: i18n("Files")

    theme {
        primaryColor: Palette.colors.blue["500"]
    }

    function confirmAction(title, text, primaryButton, color) {
        confirmDialog.promise = new Promises.Promise()

        confirmDialog.title = title
        confirmDialog.text = text
        confirmDialog.positiveButtonText = primaryButton

        confirmDialog.show()

        return confirmDialog.promise
    }

    Component.onCompleted: visible = true

    initialPage: FolderPage {
        id: folderPage
    }

    Dialog {
        id: confirmDialog

        property var promise

        onAccepted: promise.resolve()
        onRejected: promise.reject()
    }
}

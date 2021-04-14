import QtQuick 2.14
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.0

Dialog {
    visible: true
    title: "Here is a title"
    width: 100
    height: 100

    Text {
	anchors.centerIn: parent
	text: "Hello, World!"
    }
}

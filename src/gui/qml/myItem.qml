import QtQuick 2.14
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.0

Dialog {
    visible: true
    title: "Data Manager"
    width: 300
    height: 300

    Column {
	Text {
	    id: label1
	    text: "Hello, Foo!"
	    height: 50
	}
	Text {
	    id: label2
	    text: "Do More"
	    height: 50
	}
	Button {
	    text: "+"
	}
    }
}

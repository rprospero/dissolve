import QtQuick 2.14
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.0
import DataManagerReferencePoints 1.0

Dialog {
    visible: true
    title: "Data Manager"
    width: 871
    height: 514

    DataManagerReferencePoints {
	id: tableModel
    }

    GroupBox {
	id: dataGroup
	anchors.right: referenceGroup.left
	anchors.top: parent.top
	anchors.left: parent.left
	anchors.bottom: parent.bottom
	title: "Available Data"
	Image {
	    anchors.right: simulationDataFilterEdit.left
	    anchors.top: parent.top
	    width: 20
	    height: 20
	    source: "qrc:/general/icons/general_filter.svg"
	}
	TextField {
	    anchors.right: parent.right
	    anchors.top: parent.top
	    id: simulationDataFilterEdit
	    placeholderText: "Filtering"
	    ToolTip.text: "Filter templates by name / description"
	    ToolTip.visible: hovered
	}
	TableView {
	    anchors.bottom: parent.bottom
	    anchors.left: parent.left
	    anchors.right: parent.right
	    anchors.top: simulationDataFilterEdit.bottom
	    columnSpacing: 1
	    rowSpacing: 1
	    clip: true

	    model: tableModel
	    delegate: Rectangle {
		implicitWidth: parent.width/2
		Text {
		    text: display
		}
	    }
	}
    }
    GroupBox {
	id: referenceGroup
	anchors.right: parent.right
	anchors.top: parent.top
	anchors.bottom: parent.bottom
	title: "Reference Points"
	/* Column { */
	Row {
	    id: toolbar
	    anchors.top: parent.top
	    anchors.right: parent.right
	    anchors.left: parent.left
	    ToolButton {
		icon.source: "qrc:/general/icons/general_remove.svg"
		text: "Remove"
	    }
	    ToolButton {
		icon.source: "qrc:/menu/icons/menu_open.svg"
		text: "open"
	    }
	    ToolButton {
		icon.source: "qrc:/menu/icons/menu_new.svg"
		text: "Create..."
	    }
	}
	TableView {
	    columnSpacing: 1
	    rowSpacing: 1
	    clip: true
	    anchors.bottom: parent.bottom
	    anchors.right: parent.right
	    anchors.left: parent.left
	    anchors.top: toolbar.bottom

	    model: tableModel
	    /* model: tableModel */

	    delegate: Rectangle {
		implicitWidth: parent.width / 2
		implicitHeight: 50
		Text {
		    text: display
		}
	    }
	}
    }
}

import QtQuick 2.14
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.0

Dialog {
    visible: true
    title: "Data Manager"
    width: 871
    height: 514

    Row {
	GroupBox {
	    title: "Available Data"
	    Row {
		Image {
		    width: 20
		    height: 20
		    source: "qrc:/general/icons/general_filter.svg"
		}
		TextField {
		    id: simulationDataFilterEdit
		    placeholderText: "Filtering"
		    ToolTip.text: "Filter templates by name / description"
		    ToolTip.visible: hovered
		}
	    }
	    TableView {
	    }
	}
	GroupBox {
	    title: "Reference Points"
	    Column {
		Row {
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
	    }
	    TableView {
	    }
	}
    }
}

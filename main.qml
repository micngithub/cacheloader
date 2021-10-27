import QtQuick 2.12
import QtQuick.Window 2.12
import CacheLoader 1.0

Window {
    id : window
    width: 1200
    height: 801
    visible: true
    title: qsTr("Cache Loader")

    property int count: 0
    Component.onCompleted: {
        timer.start()
    }

    Timer {
        id : timer
        interval: 200
        running: true
        repeat: true
        onTriggered: {
            window.count += 1
        }
    }

    Item {
        id : realParent
        anchors.fill: parent


    }

    CacheLoader {
        id : testLoader
        active: true
        asynchronous: true
        source :
            switch ( window.count % 3) {
            case 0: {
                "qrc:/TestQml.qml"
            }
                break;
            case 1: {
                "qrc:/TestQml2.qml"
            }
                break;
            case 2: {
                "qrc:/TestQml3.qml"
            }
                break;
            default :
                break;
            }
    }
}


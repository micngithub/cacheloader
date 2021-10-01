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

            if( window.count % 2 == 0) {
                testLoader.source = "qrc:/TestQml.qml"
            } else {
                testLoader.source = "qrc:/TestQml2.qml"
            }
        }
    }

    //    Loader {
    //        id : testLoader
    //        active: true
    //        source : "qrc:/TestQml.qml"
    //    }

    CacheLoader {
        id : testLoader
        active: true
        source : "qrc:/TestQml.qml"
    }
}

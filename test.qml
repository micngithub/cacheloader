import QtQuick 2.12
import QtQuick.Window 2.12
import CacheLoader 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    CacheLoader {
        id : testLoader
    }
}

import QtQuick 2.0
import QtWebKit 3.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import QtMultimedia 5.0

Rectangle {
    id: rectangle1
    width: 800
    height: 600
    color: "#ffffff"

    MouseArea {
        anchors.fill: parent
        onClicked: view.Close()
    }

    Image {
        id: image1
        x: 0
        y: 0
        width: 300
        height: 300
        fillMode: Image.PreserveAspectFit
        source: "earth.tunnel.jpg"
    }

    Text {
        id: text1
        x: 367
        y: 44
        width: 329
        height: 70
        color: "#040ead"
        text: qsTr("NewGene")
        font.bold: true
        style: Text.Normal
        font.family: "Verdana"
        textFormat: Text.PlainText
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 28
    }

    Text {
        id: text2
        x: 367
        y: 115
        width: 329
        height: 39
        color: "#040ead"
        text: qsTr("Cross Sectional Time Series Data Manager")
        textFormat: Text.PlainText
        font.bold: false
        horizontalAlignment: Text.AlignHCenter
        font.family: "Verdana"
        font.pixelSize: 16
        style: Text.Normal
        verticalAlignment: Text.AlignVCenter
    }

    Text {
        id: text3
        x: 367
        y: 154
        width: 329
        height: 39
        color: "#040ead"
        text: qsTr("Data Management for the Social Sciences")
        font.italic: true
        textFormat: Text.PlainText
        font.bold: false
        horizontalAlignment: Text.AlignHCenter
        font.family: "Verdana"
        style: Text.Normal
        font.pixelSize: 16
        verticalAlignment: Text.AlignVCenter
    }

    Text {
        id: text4
        x: 367
        y: 283
        color: "#0a15e3"
        text: qsTr("D. Scott Bennett")
        font.family: "Verdana"
        font.pixelSize: 14
    }

    Text {
        id: text5
        x: 507
        y: 283
        color: "#0a15e3"
        text: "Paul Poast"
        font.family: "Verdana"
        font.pixelSize: 14
    }

    Text {
        id: text6
        x: 600
        y: 283
        color: "#0a15e3"
        text: "Allan C. Stam"
        font.family: "Verdana"
        font.pixelSize: 14
    }

    Text {
        id: text7
        x: 655
        y: 550
        text: qsTr("Version 1.0 - Nov 15, 2014")
        font.pixelSize: 12
    }

    Text {
        id: text8
        x: 721
        y: 570
        text: qsTr("<html><style type='text/css'></style><a href='http://google.com'>Release notes</a></html>")
        font.pixelSize: 12
        onLinkActivated: Qt.openUrlExternally(link)
    }

    WebView
    {
        id: webview
        x: 27
        y: 343
        width: 247
        height: 203

        url: "http://www.youtube.com/embed/gZxvSs36JoI"

        onNavigationRequested: {
            // detect URL scheme prefix, most likely an external link
            var schemaRE = /^\w+:/;
            if (schemaRE.test(request.url)) {
                request.action = WebView.AcceptRequest;
            } else {
                request.action = WebView.IgnoreRequest;
                // delegate request.url here
            }
        }
    }

}

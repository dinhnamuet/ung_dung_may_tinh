import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
ApplicationWindow {
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("Ung dung may tinh do luong & dieu khien")
    header: Rectangle {
        height: 50
        color: "Brown"
        Text {
            anchors.centerIn: parent
            font {
                pointSize: 25
                bold: true
            }
            text: "MOTOR CONTROLLER"
        }
        border {
            width: 3
            color: "Yellow"
        }
    }
    /* end of header */
    footer: Rectangle {
        height: 15
        Text {
            anchors.centerIn: parent
            text: qsTr("Dinh Huu Nam")
            font.italic: true
        }
    }
    /* end of footer */
    background: Rectangle {
        color: "Grey"
    }
    /* end of background */

    ListModel {
        id: clock
        ListElement {cur_time: 0}
        ListElement {cur_time: 0}
        ListElement {cur_time: 0}
        ListElement {cur_time: 0}
        ListElement {cur_time: 0}
        ListElement {cur_time: 0}
        ListElement {cur_time: 0}
        ListElement {cur_time: 0}
        ListElement {cur_time: 0}
        ListElement {cur_time: 0}
    }
    ListModel {
        id: speed
        ListElement {cur_speed: 0}
        ListElement {cur_speed: 0}
        ListElement {cur_speed: 0}
        ListElement {cur_speed: 0}
        ListElement {cur_speed: 0}
        ListElement {cur_speed: 0}
        ListElement {cur_speed: 0}
        ListElement {cur_speed: 0}
        ListElement {cur_speed: 0}
        ListElement {cur_speed: 0}
    }
    Rectangle {
        id: sock_connect
        visible: true
        color: "Orange"
        anchors.fill: parent
        Column {
            width: parent.width /2
            height: parent.height /2
            anchors.centerIn: parent
            spacing: 5
            Text {
                font {
                    pointSize: 20
                    bold: true
                    family: "Timesnewroman"
                }
                text: "MONITOR SYSTEM"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            TextField {
                id: ip_server
                width: parent.width
                height: 30
                anchors.horizontalCenter: parent.horizontalCenter
                text: "192.168.1.18"
                placeholderText: "Type the ip of server!"
            }
            TextField {
                id: port_server
                width: parent.width
                height: 30
                anchors.horizontalCenter: parent.horizontalCenter
                text: "2000"
                placeholderText: "Type the port of server!"
            }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                text: "Connect to Board"
                onClicked: {
                    Check_Time.connectToServer(ip_server.text, port_server.text)
                }
            }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                text: "Exit"
                onClicked: {
                    Qt.quit()
                }
            }
            Text {
                id: connect_status
                anchors.horizontalCenter: parent.horizontalCenter
                font.italic: true
                font.underline: true
                text: "status"
            }
        }
    }

    Column {
        id: noi_dung
        visible: !(sock_connect.visible)
        anchors.fill: parent
        Drawer {
            id: operating_option
            width: parent.width *2/3
            height: parent.height
            background: Image {
                source: "file:///C:/Users/nam/Pictures/Qt_icon/conan.jpg"
            }

            Column {
                anchors.fill: parent
                spacing: 3
                Rectangle {
                    width: parent.width
                    height: 50
                    color: "Orange"
                    Text {
                        anchors.centerIn: parent
                        font.pointSize: 25
                        text: "OPTION"
                    }
                }
                Button {
                    width: parent.width
                    height: 40
                    text: "Continue"
                    onClicked: {
                        operating_option.close()
                    }
                }
                Button {
                    width: parent.width
                    height: 40
                    text: "Exit"
                    icon.source: "file:///C:/Users/nam/Pictures/Qt_icon/exit.png"
                    onClicked: {
                        request_exit.open()
                    }
                }

            }
        }
        Row {
            id: drawer_controller
            width: parent.width
            height: 30
            TabButton {
                height: parent.height
                width: height
                icon.source: "file:///C:/Users/nam/Pictures/Qt_icon/drawer.png"
                onClicked: {
                    operating_option.open()
                }
            }
            Rectangle {
                width: parent.width - parent.height
                height: parent.height
                color: "pink"
                Text {
                    id: information
                    anchors.centerIn: parent
                    font {
                        pointSize: 20
                        italic: true
                    }
                    text: "Speed Chart"
                }
            }
        }

        Rectangle {
            id: content_to_show
            /* content */
            width: parent.width
            height: parent.height *2/3
            anchors.horizontalCenter: parent.horizontalCenter
            Column {
                anchors.fill: parent
                spacing: 3
                ListView {
                    id: chart
                    width: parent.width
                    height: parent.height *0.9
                    anchors.horizontalCenter: parent.horizontalCenter
                    orientation: Qt.Horizontal
                    model: speed
                    delegate: Rectangle {
                        width: root.width/10
                        height: model.cur_speed
                        anchors.bottom: parent.bottom
                        border {
                            width: 1
                            color: "Black"
                        }
                        Text {
                            anchors.centerIn: parent
                            text: parent.height + "rpm"
                        }

                        color: "Yellow"
                    }
                }
                ListView {
                    width: parent.width
                    height: parent.height - chart.height
                    anchors.horizontalCenter: parent.horizontalCenter
                    orientation: Qt.Horizontal
                    model: clock
                    delegate: Rectangle {
                        height: parent.height
                        width: root.width/10
                        Text {
                            text: model.cur_time + "s"
                            anchors.centerIn: parent
                        }
                        border {
                            width: 1
                            color: "Black"
                        }
                    }
                }
            }
        }
        Row {
            id: setpoint
            width: parent.width
            height: 20
            TextField {
                id: set_point
                width: parent.width *2/3
                height: 20
                placeholderText: "Type the set point"
            }
            Button {
                width: parent.width *1/9
                height: 20
                text: "Forward"
                onClicked: {
                    var data = "Forward "+set_point.text
                    Check_Time.send_to_Server(data);
                }
            }
            Button {
                width: parent.width *1/9
                height: 20
                text: "Reverse"
                onClicked: {
                    var data = "Reverse "+set_point.text
                    Check_Time.send_to_Server(data);
                }
            }
            Button {
                width: parent.width *1/9
                height: 20
                text: "Stop"
                onClicked: {
                    var data = "Stop "+set_point.text
                    Check_Time.send_to_Server(data);
                    set_point.text = '0'
                }
            }
        }
        Row {
            width: parent.width
            height: parent.height - drawer_controller.height - content_to_show.height - setpoint.height
            Rectangle {
                height: parent.height
                width: height
                Image {
                    id: fan
                    height: parent.height
                    width: height
                    source: "file:///C:/Users/nam/Pictures/Qt_icon/fan.png"
                }
            }

            Rectangle {
                width: parent.width - fan.width
                height: parent.height
                Text {
                    id: motor_direction
                    anchors.centerIn: parent
                    font {
                        pointSize: 25
                        bold: true
                    }
                    text: Check_Time.readDirection()
                }
            }
        }
    }
    RotationAnimator {
        id: fan_rotate
        target: fan
        from: 0
        to: 360
        duration: 1000
        running: false
        loops: Animation.Infinite
    }
    Connections {
        target: Check_Time
        function onTimeChanged() {
            var cur_timing = parseInt(Check_Time.readTime())
            for(var i=0; i<10; i++)
            {
                if(i === 9)
                    clock.set(9, { cur_time: cur_timing })
                else
                {
                    var next_value = clock.get(i + 1).cur_time
                    clock.set(i, { cur_time: next_value })
                }
            }
            information.text = "Setpoint: "+set_point.text+", "+"Speed: "+speed.get(9).cur_speed
        }
        function onConnectionChanged() {
            if(Check_Time.read_connection() === true)
            {
                sock_connect.visible = false
            }
            else
            {
                ip_server.text = ""
                port_server.text = ""
                connect_status.color = "Red"
                connect_status.text = "Username or password does not match!"
            }
        }
        function onSpeedChanged() {
            var cur_sp = parseInt(Check_Time.readSpeed())
            if(cur_sp === 0)
            {
                fan_rotate.running = false
                fan.rotation = 0
            }
            else
            {
                fan_rotate.running = true
            }

            for(var i=0; i<10; i++)
            {
                if(i === 9)
                    speed.set(9, { cur_speed: cur_sp })
                else
                {
                    var next_var = speed.get(i + 1).cur_speed
                    speed.set(i, { cur_speed: next_var })
                }
            }
        }
        function onDirectionChanged() {
            motor_direction.text = Check_Time.readDirection()
            if(motor_direction.text === "forward")
            {
                fan_rotate.running = false
                fan_rotate.from = 0
                fan_rotate.to = 360
            }
            else if(motor_direction.text === "reverse")
            {
                fan_rotate.running = false
                fan_rotate.from = 360
                fan_rotate.to = 0
            }
        }
    }
    Dialog {
        id: request_exit
        anchors.centerIn: parent
        title: "Exit"
        Text {
            text: qsTr("Do you really want to exit?")
        }
        standardButtons: Dialog.Yes| Dialog.No
        onAccepted: {
            Check_Time.exit_socket()
            ip_server.text = ""
            port_server.text = ""
            connect_status.text = "status"
            connect_status.color = "Black"
            sock_connect.visible = true
            operating_option.close()
        }
        onRejected: {
            operating_option.close()
        }
    }
}

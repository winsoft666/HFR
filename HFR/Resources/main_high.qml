import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import HFR.IDCard 1.0     // C++中IDCardReader类注册为QML对象
import HFR.VideoItem 1.0  // C++中VideoItem类注册为QML对象


Window {
    id: root;  // id必须全文件唯一
    objectName: "root";  // objectName必须全文件唯一，C++中findChild函数使用的是objectName
    visible: true;
    width: 1024;
    height: 576;
    minimumWidth: 1024;
    minimumHeight: 576;
    //maximumWidth: 1024;
    //maximumHeight: 576;
    title: qsTr("人脸识别系统"); // 窗口标题

    // 矩形容器
    Rectangle {
        id: rectPreview;       // 唯一id
        anchors.fill: parent;  // 使用anchor锚点方式布局
        z: 0;                  // z顺序，值越小，元素就越垫底

        VideoItem {
            id: video;
            objectName: "video";
            visible:true;
            anchors.fill: parent;
        }

        GaussianBlur {
           id: videoBlurMask;
           anchors.fill: rectPreview;
           source: video;
           radius: 8;
           cached: true;
           visible: false;
        }

        // 定义一个函数
        // 调用示例：rectPreview.showBlurMask(true);
        function showBlurMask(bVisible) {
            videoBlurMask.visible = bVisible;
        }
    }

    Rectangle {
        id: rectBasicInfoBK;
        width: 220;
        height: 340;
        anchors.leftMargin: 20;
        anchors.topMargin: 20;
        anchors.left: parent.left;
        anchors.top: parent.top;
        color: "#000000";         // 填充色
        opacity: 0.4;             // 透明度 0~1.0
        visible: false;
    }

    Rectangle {
        id: rectBasicInfo;
        width: 220;
        height: 340;
        anchors.leftMargin: 20;
        anchors.topMargin: 20;
        anchors.left: parent.left;
        anchors.top: parent.top;
        color: "transparent";
        visible: false;

        Image {
            id: imgIdCard;
            width: 102;
            height: 126;
            anchors.top: parent.top;
            anchors.topMargin: 36;
            anchors.horizontalCenter: parent.horizontalCenter;
            fillMode: Image.PreserveAspectFit;
            cache: false;
            visible: false;
        }

        Rectangle{
             id: mask0;
             width: 102;
             height: 126;
             anchors.top: parent.top;
             anchors.topMargin: 36;
             anchors.horizontalCenter: parent.horizontalCenter;
             radius: 3;
             visible: false;
         }

        OpacityMask {
             width: 102;
             height: 126;
             anchors.top: parent.top;
             anchors.topMargin: 36;
             anchors.horizontalCenter: parent.horizontalCenter;
             source: imgIdCard;
             maskSource: mask0;
         }

        Rectangle {
            id: gridBasicInfo;
            anchors.top: imgIdCard.bottom;
            anchors.topMargin: 24;
            anchors.left: parent.left;
            anchors.leftMargin: 20;

            Text{
                id: txtNameTitle;
                anchors.top: parent.top;
                anchors.left: parent.left;
                horizontalAlignment: Text.AlignRight;
                verticalAlignment: Text.AlignVCenter;
                font.family: "微软雅黑";   // 字体名称
                font.pixelSize: 12;      // 字体大小
                text: qsTr("姓名：");     // 文本
                color: "#bbbbbb";        // 文本颜色
            }

            Text {
                id: txtName;
                anchors.top: txtNameTitle.top;
                anchors.left: txtNameTitle.right;
                anchors.leftMargin: 5;
                horizontalAlignment: Text.AlignLeft;
                verticalAlignment: Text.AlignVCenter;
                font.family: "微软雅黑";
                font.pixelSize:12;
                color: "#ffffff";
                text: "";
            }

            Text{
                id: txtSexTitle;
                anchors.top: txtNameTitle.bottom;
                anchors.topMargin: 4;
                anchors.left: parent.left;
                horizontalAlignment: Text.AlignRight;
                verticalAlignment: Text.AlignVCenter;
                font.family: "微软雅黑";
                font.pixelSize: 12;
                text: qsTr("性别：");
                color: "#bbbbbb";
            }

            Text {
                id: txtSex;
                anchors.top: txtSexTitle.top;
                anchors.left: txtSexTitle.right;
                anchors.leftMargin: 5;
                horizontalAlignment: Text.AlignLeft;
                verticalAlignment: Text.AlignVCenter;
                font.family: "微软雅黑";
                font.pixelSize: 12;
                color: "#ffffff";
                text: "";
            }

            Text{
                id: txtNationTitle;
                anchors.top: txtSexTitle.bottom;
                anchors.topMargin: 4;
                anchors.left: parent.left;
                horizontalAlignment: Text.AlignRight;
                verticalAlignment: Text.AlignVCenter;
                font.family: "微软雅黑";
                font.pixelSize: 12;
                text: qsTr("民族：");
                color: "#bbbbbb";
            }

            Text {
                id: txtNation;
                anchors.top: txtNationTitle.top;
                anchors.left: txtNationTitle.right;
                anchors.leftMargin: 5;
                horizontalAlignment: Text.AlignLeft;
                verticalAlignment: Text.AlignVCenter;
                font.family: "微软雅黑";
                font.pixelSize: 12;
                color: "#ffffff";
                text: "";
            }

            Text{
                id: txtBirthdayTitle;
                anchors.top: txtNationTitle.bottom;
                anchors.topMargin: 4;
                anchors.left: parent.left;
                horizontalAlignment: Text.AlignRight;
                verticalAlignment: Text.AlignVCenter;
                font.family: "微软雅黑";
                font.pixelSize: 12;
                text: qsTr("出生：");
                color: "#bbbbbb";
            }

            Text {
                id: txtBirthday;
                anchors.top: txtBirthdayTitle.top;
                anchors.left: txtBirthdayTitle.right;
                anchors.leftMargin: 5;
                horizontalAlignment: Text.AlignLeft;
                verticalAlignment: Text.AlignVCenter;
                font.family: "微软雅黑";
                font.pixelSize: 12;
                color: "#ffffff";
                text: "";
            }

            Text{
                id: txtCardNumTitle;
                anchors.top: txtBirthdayTitle.bottom;
                anchors.topMargin: 4;
                anchors.left: parent.left;
                horizontalAlignment: Text.AlignRight;
                verticalAlignment: Text.AlignVCenter;
                font.family: "微软雅黑";
                font.pixelSize: 12;
                text: qsTr("公民身份号码：");
                color: "#bbbbbb";
            }

            Text {
                id: txtCardNum;
                anchors.top: txtCardNumTitle.bottom;
                anchors.topMargin: 4;
                anchors.left: parent.left;
                horizontalAlignment: Text.AlignLeft;
                verticalAlignment: Text.AlignVCenter;
                font.family: "微软雅黑";
                font.pixelSize: 14;
                color: "#ffffff";
                text:"";
            }
        }
    }

    GroupBox {
        id: grpFunction;
        title: qsTr("高级");
        anchors.left: parent.left;
        anchors.bottom: parent.bottom;
        height: 150;
        width: 220;
        visible: true;

        // 列布局
        ColumnLayout {
            spacing: 4;

            CheckBox {
                id: btnPreview;
                text: qsTr("摄像头预览");
                checked: cameraPreview.enabled; // 是否选中
            }

            CheckBox {
                id: btnRealTimeDetect;
                checked: cameraPreview.liveFaceDetect;
                text: qsTr("实时人脸检测");
            }

            Button {
                id: btnGrapImage;
                Layout.minimumWidth: 120;
                text: qsTr("手动抓图");
            }

            Button {
                id: btnGrapFaceImage;
                Layout.minimumWidth: 120;
                text: qsTr("手动抓人脸图");

                BusyIndicator {
                    id: busyGrapFaceImage;
                    anchors.left: parent.left;
                    anchors.leftMargin: 3;
                    anchors.top: parent.top;
                    anchors.topMargin: (btnGrapFaceImage.height - height) / 2;

                    running: false;

                    style: BusyIndicatorStyle {
                        indicator: Image {
                            visible: control.running
                            source: "qrc:/HFR/Resources/image/Transfer.png"
                            RotationAnimator on rotation {
                                running: control.running
                                loops: Animation.Infinite
                                duration: 2000
                                from: 0 ; to: 360
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: rectGrapImageBK;
        anchors.top: parent.top;
        anchors.bottom: parent.bottom;
        anchors.right: parent.right;
        width: 180;
        color: "#000000";
        opacity: 0.4;
    }

    Rectangle {
        id: rectGrapImage;
        anchors.top: parent.top;
        anchors.bottom: parent.bottom;
        anchors.right: parent.right;
        anchors.topMargin: 25;
        anchors.rightMargin: 20;
        width: 140;
        radius: 5;
        color: "transparent";

        Component {
            id: grapImageDelegate;

            Rectangle {
                id: wrapper;
                width: 140;
                height: 90;
                color: "transparent";

                Image {
                    id: imgGrap;
                    anchors.fill: parent;
                    smooth: true;
                    fillMode: Image.PreserveAspectFit;
                    source: imgUri;
                    visible: false;
                }

                Rectangle{
                     id: mask1;
                     anchors.fill: parent;
                     radius: 2;
                     visible: false;
                 }

                 OpacityMask {
                     anchors.fill: parent;
                     source: imgGrap;
                     maskSource: mask1;
                 }

                MouseArea {
                      hoverEnabled: true;
                      anchors.fill: imgGrap;

                      onEntered: {
                          rectPreview.showBlurMask(true);
                          imgBigGrapWrapper.setPos(this);
                          imgBigGrap.source = imgGrap.source;
                          imgBigGrapWrapper.visible = true;
                      }

                      onExited: {
                          rectPreview.showBlurMask(false);
                          imgBigGrapWrapper.visible = false;
                      }
                  }
            }
        }

        ScrollView {
            horizontalScrollBarPolicy: Qt.ScrollBarAsNeeded;
            clip: true;
            anchors.fill: parent;

            ListView {
                id: listCapture;
                anchors.fill: parent;

                spacing: 15;
                focus: true;

                delegate: grapImageDelegate;
                model: grapImageListModel;
            }
        }
    }

    Item {
        id: imgBigGrapWrapper;
        width: 550;
        height: 310;
        z: 100;
        anchors.rightMargin: 5;
        anchors.right: rectGrapImageBK.left;
        visible: false;

        Image {
            id: imgBigGrap;
            anchors.fill: parent;
            smooth: true;
            sourceSize: Qt.size(parent.width, parent.height);
            fillMode: Image.PreserveAspectFit;
            visible: false;
        }

        RectangularGlow {
            anchors.fill: imgBigGrapWrapper;
            glowRadius: 6;
            spread: 0.2;
            color: "gray";
            cornerRadius: 5;
        }

        Rectangle{
             id: mask;
             anchors.fill: parent;
             radius: 6;
             visible: false;
         }

         OpacityMask {
             anchors.fill: parent;
             source: imgBigGrap;
             maskSource: mask;
         }

         function setPos(object) {
             var globalCoordinares = object.mapToItem(parent, 0, 0);
             var yValue = globalCoordinares.y - (imgBigGrapWrapper.height - object.height) / 2;
             if(yValue < 0) {
                 yValue = 30;
             }
             else if(yValue + imgBigGrapWrapper.height > parent.height) {
                 yValue = parent.height - imgBigGrapWrapper.height - 30;
             }
             imgBigGrapWrapper.y = yValue;
         }
    }

    Text{
        id: fpsText;
        anchors.top: parent.top;
        anchors.right: parent.right;
        anchors.topMargin: 3;
        width: 80;
        height: 20;

        font.family: "Arial";
        font.pixelSize: 12;
        color: "white";
        text: "";
    }


    Rectangle {
        id: rectMask;
        anchors.fill: parent;
        color: "#000000";
        visible: false;
        opacity: 0.6;
        z: 199;
    }

    Rectangle {
        id: rectCamaraNotConnTips;
        anchors.fill: parent;
        visible: false;
        color: "transparent";
        z: 200;

        Text {
            id: txtNoCameraSignal;
            anchors.centerIn: parent;
            font.family: "微软雅黑";
            font.pixelSize: 50;
            font.bold: false;
            color: "#ffffff"
            horizontalAlignment: Text.AlignHCenter;
            verticalAlignment: Text.AlignVCenter;

            text: "无视频输入信号";
        }

        Button {
            id: btnReopenCamera;
            anchors.horizontalCenter: parent.horizontalCenter;
            anchors.top: txtNoCameraSignal.bottom;
            anchors.topMargin: 8;
            text: "重试";
        }
    }

    Rectangle {
        id: rectRecognizeResult;
        anchors.fill: parent;
        color: "transparent";
        visible: false;
        z: 200;

        Image {
            id: imgResult;
            anchors.horizontalCenter: parent.horizontalCenter;
            anchors.verticalCenter: parent.verticalCenter;
        }

        Text {
            id: txtResult;
            anchors.top: imgResult.bottom;
            anchors.topMargin: 20;
            anchors.horizontalCenter: parent.horizontalCenter;
            font.family: "微软雅黑";
            font.pixelSize: 28;
            font.bold: false;
            color: "#ffffff";
            horizontalAlignment: Text.AlignHCenter;
            verticalAlignment: Text.AlignVCenter;

            text: "";
        }

        MouseArea {
            anchors.fill: parent;
            onClicked: {
                timerHide.stop();
                allHide();
            }
        }

        Timer {
            id: timerHide;
            interval: 5000;
            running: false;
            repeat: false;
            onTriggered: {
                allHide();
            }
        }

        function show(bSuccess, strText) {
            if(bSuccess) {
                imgResult.source = "qrc:/HFR/Resources/image/success.png";
            }
            else {
                imgResult.source = "qrc:/HFR/Resources/image/failed.png";
            }

            txtResult.text = strText;

            rectRecognizeResult.visible = true;

            timerHide.start();
        }
    }

    Rectangle {
        id: rectMsg;
        anchors.fill: parent;
        color: "transparent";
        visible: false;
        z: 200;

        Text {
            id: txtMsg;
            anchors.centerIn: parent;
            font.family: "微软雅黑";
            font.pixelSize: 38;
            font.bold: false;
            color: "#ffffff";
            horizontalAlignment: Text.AlignHCenter;
            verticalAlignment: Text.AlignVCenter;

            text: "";
        }

        MouseArea {
            anchors.fill: parent;
            onClicked: {
                timerHideMsg.stop();
                allHide();
            }
        }

        Timer {
            id: timerHideMsg;
            interval: 5000;
            running: false;
            repeat: false;
            onTriggered: {
                allHide();
            }
        }

        function show(strText) {
            txtMsg.text = strText;
            rectMsg.visible = true;
            timerHideMsg.start();
        }
    }

    Rectangle {
        id: rectProgressTipMask;
        anchors.bottom: parent.bottom;
        anchors.bottomMargin: 10;
        anchors.horizontalCenter: parent.horizontalCenter;
        radius: 5;
        color: "#000000";
        visible: false;
        opacity: 0.6;
        width: 500;
        height: 60;
        z: 199;
    }

    Rectangle {
        id: rectProgressTip;
        anchors.bottom: parent.bottom;
        anchors.bottomMargin: 10;
        anchors.horizontalCenter: parent.horizontalCenter;
        color: "transparent";
        width: 500;
        height: 60;
        z: 200;

        Text {
            id: txtProgressTip;
            anchors.centerIn: parent;
            font.family: "微软雅黑";
            font.pixelSize: 30;
            font.bold: false;
            color: "#ffffff";
            horizontalAlignment: Text.AlignHCenter;
            verticalAlignment: Text.AlignVCenter;

            text: "";
        }

        function show(strTip) {
            txtProgressTip.text = strTip;
            rectProgressTip.visible = true;
        }
    }

    function allHide() {
        rectCamaraNotConnTips.visible = false;
        rectRecognizeResult.visible = false;
        rectMsg.visible = false;
        rectMask.visible = false;

        rectProgressTipMask.visible = false;
        rectProgressTip.visible = false;
    }

    function showCamaraNotConnTips(bShow) {
        allHide();
        rectCamaraNotConnTips.visible = bShow;
        rectMask.visible = bShow;
    }

    function showResult(strMsg, bPass) {
        allHide();

        rectRecognizeResult.show(bPass, strMsg);
        rectMask.visible = true;
    }

    function showMsg(strMsg) {
        allHide();

        rectMsg.show(strMsg);
        rectMask.visible = true;
    }

    function showProgress(strTip) {
        allHide();
        rectProgressTip.show(strTip);
        rectProgressTipMask.visible = true;
    }

    Connections {
        target: camera;

        // 将C++中OpenCVCamera类的信号enabledChanged，绑定到QML中的槽函数
        // 函数名需要遵循如下规则：on + 信号名（首字母大写）
        onEnabledChanged: {
            if(bEnable) {
                showCamaraNotConnTips(false);
            }
        }

        onBeginRecognize: {
            allHide();
            grapImageListModel.clearGrapImage();
        }

        onBeginGrapFaceImage: {
            btnGrapFaceImage.enabled = false;
            btnGrapFaceImage.text = qsTr("抓取人脸中...");
            busyGrapFaceImage.running = true;

            showProgress("正在抓取人脸，请对准摄像头");
        }

        onEndGrapFaceImage: {
            busyGrapFaceImage.running = false;
            btnGrapFaceImage.text = qsTr("手动抓人脸图");
            btnGrapFaceImage.enabled = true;

            showProgress("正在对比人脸数据...");
        }

        onNoVideoInputSignal: {
            camera.enabled = false;
            cameraPreview.enabled = false;

            showCamaraNotConnTips(true);
        }
    }

    Connections {
        target: video;
        onFpsChanged: {
            fpsText.text = fps + " fps";
        }
    }

    Connections {
        target: btnGrapFaceImage;
        onClicked: {
            camera.grapImage(true, 5);
        }
    }

    Connections {
        target: btnGrapImage;
        onClicked: {
            camera.grapImage(false, 1);
        }
    }

    Connections {
        target: btnPreview;
        onClicked: {
            cameraPreview.enabled = btnPreview.checkedState == Qt.Checked;
        }
    }

    Connections {
        target: btnRealTimeDetect;
        onClicked: {
            cameraPreview.liveFaceDetect = (btnRealTimeDetect.checkedState == Qt.Checked);
        }
    }

    Connections {
        target: btnReopenCamera;
        onClicked: {
            camera.enabled = true;
            cameraPreview.enabled = true;
        }
    }

    Connections {
        target: idCardReader;
        onNewIdCard: {
            var imgPath = "file:/" + idCardReader.photoPath;
            imgPath.replace("\\", "/");

            imgIdCard.source = "";
            imgIdCard.source = imgPath;
            txtName.text = idCardReader.name;
            txtSex.text = idCardReader.sex;
            txtNation.text = idCardReader.nation;
            txtBirthday.text = idCardReader.birthday;
            txtCardNum.text = idCardReader.cardNumber;

            rectBasicInfo.visible = true;
            rectBasicInfoBK.visible = true;

            camera.recognize(idCardReader.name,
                             idCardReader.sex,
                             idCardReader.nation,
                             idCardReader.birthday,
                             idCardReader.photoPath,
                             idCardReader.address,
                             idCardReader.cardNumber
                             );
        }
    }


    IDCardReader {
        id: idCardReader;
    }
}

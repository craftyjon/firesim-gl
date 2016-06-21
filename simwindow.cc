#include "simwindow.h"

SimWindow::SimWindow(quint16 udpPort)
{
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3,3);

    setFormat(format);

    _blur = false;
    _fullscreen = false;

    _socket = new QUdpSocket();
    _socket->bind(udpPort);
    connect(_socket, SIGNAL(readyRead()), this, SLOT(processDatagrams()));
}

SimWindow::~SimWindow()
{
}

void SimWindow::processDatagrams()
{
    while (_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(_socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        _socket->readDatagram(datagram.data(), datagram.size(),
                              &sender, &senderPort);

        _scene->processDatagram(datagram);
    }
}

void SimWindow::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_F) {
        _fullscreen = !_fullscreen;
        setWindowState(_fullscreen ? Qt::WindowFullScreen : Qt::WindowNoState);
    } else if (ev->key() == Qt::Key_B) {
        _blur = !_blur;
        update();
    }
}

void SimWindow::resizeGL(int w, int h)
{
    /*
        center = self.scene.center
        extents = self.scene.extents

        # multiply scene coordinates by this to get screen coordinates
        scale = min((width/2) / min(center[0], extents[0] - center[0]),
                    (height/2) / min(center[1], extents[1] - center[1]))

        # then add this (x,y tuple) to get the coordinates in the right place
        translate = (width/2 - center[0]*scale, height/2 - center[1]*scale)

        for strand_id, strand in self.scene.strands.items():
            if strand is None: continue

            self.locations[strand_id] = locs = []
            for x, y in strand.all_locations:
                locs.append((x*scale + translate[0], y*scale + translate[1]))
     */
}

void SimWindow::paintGL()
{
    /*
        from PyQt5.QtGui import QBrush, QPen, QColor, QPainter
        from PyQt5.QtCore import QPointF

        painter = QPainter(self)
        painter.setPen(QPen(QColor(0, 0, 0, 0), 0))
        painter.fillRect(0, 0, self.width(), self.height(), QColor(0, 0, 0))

        for strand_id, strand in self.scene.strands.items():
            if strand is None: continue

            spacing = 4 if self._blur else 1
            instructions = list(zip(self.locations[strand_id][::spacing],
                                    strand.all_contents[::spacing]))

            if self._blur:
                for (x, y), (r, g, b) in instructions:
                    painter.setBrush(QColor(r, g, b, 50))
                    painter.drawEllipse(QPointF(x, y), 16, 16)

            for (x, y), (r, g, b) in instructions:
                painter.setBrush(QColor(r, g, b, 50))
                painter.drawEllipse(QPointF(x, y), 6, 6)
                painter.setBrush(QColor(r, g, b, 255))
                painter.drawEllipse(QPointF(x, y), 3, 3)
    */
}

/*
def process_datagrams(self):
        while self.socket.hasPendingDatagrams():
            (datagram, sender, sport) = self.socket.readDatagram(
                self.socket.pendingDatagramSize())
            self.scene.process_datagram(datagram)
*/

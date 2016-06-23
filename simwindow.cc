#include <algorithm>
#include <QPainter>
#include <QPen>
#include <QColor>

#include "simwindow.h"

SimWindow::SimWindow(quint16 udpPort, Scene *scene) : QOpenGLWindow()
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setAlphaBufferSize(8);

    setFormat(format);

    _blur = false;
    _fullscreen = false;
    _scene = scene;

    foreach (Strand *s, _scene->getStrands()) {
        if (s) {
            _locations.insert(s->id, QList<QPointF>());
        }
    }

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
    QPoint center = _scene->getCenter();
    QPoint extents = _scene->getExtents();

    // multiply scene coordinates by this to get screen coordinates
    double scale = std::min((w / 2.0) / std::min(center.x(), extents.x() - center.x()),
                            (h / 2.0) / std::min(center.y(), extents.y() - center.y()));

    // then add this (x,y tuple) to get the coordinates in the right place
    QPointF translate = QPointF((w / 2.0) - center.x() * scale,
                                (h / 2.0) - center.y() * scale);

    foreach (Strand *s, _scene->getStrands()) {
        if (s) {
            _locations[s->id].clear();

            foreach (QPointF l, s->getAllLocations()) {
                QPointF lScreen = QPointF(l.x() * scale + translate.x(),
                                          l.y() * scale + translate.y());
                _locations[s->id].append(lScreen);
            }
        }
    }
}

void SimWindow::paintGL()
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(0, 0, 0, 0), 0));
    painter.fillRect(0, 0, width(), height(), QColor(0, 0, 0));

    foreach (Strand *s, _scene->getStrands()) {
        if (s) {
            int spacing = _blur ? 4 : 1;
            QList<RGBColor> colors = s->getAllContents();

            for (int i = 0; i < _locations[s->id].length(); i += spacing) {
                RGBColor c = colors[i];
                QPointF p = _locations[s->id][i];

                if (_blur) {
                    painter.setBrush(QColor(c.r, c.g, c.b, 50));
                    painter.drawEllipse(p, 16, 16);
                }

                painter.setBrush(QColor(c.r, c.g, c.b, 50));
                painter.drawEllipse(p, 6, 6);
                painter.setBrush(QColor(c.r, c.g, c.b, 255));
                painter.drawEllipse(p, 3, 3);
            }
        }
    }
}

#include "scene.h"

Fixture::Fixture(QJsonObject data)
{
    length = data.value("pixels").toInt();
    strand_id = data.value("strand").toInt();
    address = data.value("address").toInt();

    for (quint32 i = 0; i < length; i++) {
        contents.append(RGBColor {0, 0, 0});
    }
}

QList<QPoint> Fixture::locations()
{
    QList<QPoint> ret;

    for (quint32 i = 0; i < length; i++) {
        ret.append(QPoint(pos1.x() + i * pixel_dx, pos1.y() + i * pixel_dy));
    }

    return ret;
}

void Strand::update(QByteArray contents)
{

}

Scene::Scene(const QString &filename)
{
    _filename = filename;
    _in_update = false;
}

void Scene::processDatagram(QByteArray datagram)
{
    if (datagram[0] == 'B') {
        _in_update = true;

    } else if (!_in_update) {
        return;

    } else if (datagram[0] == 'E') {
        QMap<quint8, QByteArray>::iterator it;
        for (it = _pending_contents.begin(); it != _pending_contents.end(); it++) {
            if (haveStrand(it.key())) {
                _strands[it.key()]->update(it.value());
            }
        }
        _in_update = false;
        _pending_contents.clear();
        emit frameReceived();

    } else if (datagram[0] == 'S') {
        if (datagram.length() < 4 || ((datagram.length() - 4) % 3 != 0)) {
            qWarning("Malformed strand update received");
        } else {
            _pending_contents[datagram[1]] = datagram.right(4);
        }
    }
}

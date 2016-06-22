#include <algorithm>

#include "scene.h"

Fixture::Fixture(QJsonObject data)
{
    length = data.value("pixels").toInt();
    strand_id = data.value("strand").toInt();
    address = data.value("address").toInt();

    QJsonArray arr = data.value("pos1").toArray();
    pos1 = QPoint(arr[0].toInt(), arr[1].toInt());
    arr = data.value("pos2").toArray();
    pos2 = QPoint(arr[0].toInt(), arr[1].toInt());

    total_dx = pos2.x() - pos1.x();
    total_dy = pos2.y() - pos1.y();
    pixel_dx = total_dx / length;
    pixel_dy = total_dy / length;

    for (quint32 i = 0; i < length; i++) {
        contents.append(RGBColor {0, 0, 0});
    }
}

QList<QPointF> Fixture::locations()
{
    if (_locations.length() == 0) {
        for (quint32 i = 0; i < length; i++) {
            _locations.append(QPointF(pos1.x() + i * pixel_dx, pos1.y() + i * pixel_dy));
        }
    }

    return _locations;
}

QList<QPointF> Strand::getAllLocations()
{
    QList<QPointF> ret;
    foreach (Fixture *f, fixtures) {
        ret.append(f->locations());
    }
    return ret;
}

QList<RGBColor> Strand::getAllContents()
{
    QList<RGBColor> ret;
    foreach (Fixture *f, fixtures) {
        ret.append(f->contents);
    }
    return ret;
}

void Strand::finalize()
{
    std::sort(fixtures.begin(), fixtures.end(), [](Fixture *a, Fixture *b) {
        return a->address < b->address;
    });
}

void Strand::update(QByteArray contents)
{
    int idx = 0;

    foreach (Fixture *f, fixtures) {
        for (auto it = f->contents.begin(); it != f->contents.end(); it++) {
            (*it).r = contents[idx];
            (*it).g = contents[idx + 1];
            (*it).b = contents[idx + 2];
            idx += 3;
        }
    }
}

Scene::Scene(const QString &filename)
{
    _filename = filename;
    _in_update = false;

    _load();
}

void Scene::_load()
{
    QFile sceneFile(_filename);

    if (!sceneFile.open(QIODevice::ReadOnly)) {
        qCritical("Couldn't open scene file!");
        return;
    }

    QByteArray sceneRawData = sceneFile.readAll();
    _data = QJsonDocument::fromJson(sceneRawData).object();

    if (_data.value("file-type") != "scene") {
        qCritical("Invalid scene file type!");
        return;
    }

    QJsonArray arr = _data.value("center").toArray();
    _center = QPoint(arr[0].toInt(), arr[1].toInt());

    arr = _data.value("extents").toArray();
    _extents = QPoint(arr[0].toInt(), arr[1].toInt());

    foreach (const QJsonValue &o, _data.value("strand-settings").toArray()) {
        quint8 id = o.toObject().value("id").toInt();
        Strand *s = new Strand(id);
        _strands.append(s);
    }

    foreach (const QJsonValue &o, _data.value("fixtures").toArray()) {
        quint8 strand_id = o.toObject().value("strand").toInt();
        if (haveStrand(strand_id)) {
            Fixture *f = new Fixture(o.toObject());
            _strands[strand_id]->addFixture(f);
        }
    }
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
            _pending_contents[datagram[1]] = datagram.mid(4);
        }
    }
}

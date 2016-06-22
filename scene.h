#ifndef _SCENE_H
#define _SCENE_H

#include <cassert>

#include <QObject>
#include <QString>
#include <QPoint>
#include <QMap>
#include <QList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>


struct RGBColor
{
    quint8 r;
    quint8 g;
    quint8 b;

    RGBColor() : r(0), g(0), b(0) {}
    RGBColor(quint8 _r, quint8 _g, quint8 _b) : r(_r), g(_g), b(_b) {}
};


class Fixture
{
  public:
    Fixture(QJsonObject data);

    QList<QPoint> locations();

    quint32 length;
    quint8 strand_id;
    quint8 address;
    QPoint pos1;
    QPoint pos2;
    double total_dx;
    double total_dy;
    double pixel_dx;
    double pixel_dy;

    QList<RGBColor> contents;

  private:
    QList<QPoint> _locations;
};

class Strand
{
  public:
    Strand(int id) : id(id) {}

    void addFixture(Fixture *fixture) {
        assert(!finalized);
        fixtures.append(fixture);
    }
    void finalize();
    void update(QByteArray contents);
    QList<QPoint> getAllLocations();
    QList<RGBColor> getAllContents();

    int id;

  private:
    bool finalized = false;
    QList<Fixture *> fixtures;
};

class Scene : public QObject
{
  Q_OBJECT

  public:
    Scene(const QString &filename);

    QPoint getCenter() const { return _center; }
    QPoint getExtents() const { return _extents; }
    //QPoint getCorner() const { return _corner; }

    bool haveStrand(int id) { return id < _strands.size() && _strands[id]; }
    const Strand* getStrand(int id) const { return _strands[id]; }
    QList<Strand *> getStrands() const { return _strands; }

  signals:
    frameReceived();

  public slots:
    void processDatagram(QByteArray datagram);

  private:
    void _load(void);

    QPoint _center;
    QPoint _extents;    // TODO: kind of wrong. this isn't a point.
    QList<Strand *> _strands;

    bool _in_update;
    QString _filename;
    QMap<quint8, QByteArray> _pending_contents;
    QJsonObject _data;
};

#endif

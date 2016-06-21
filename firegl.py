#!/usr/local/bin/python3.5

import time
import sys
import os
import json
from PyQt5 import QtCore, QtGui, QtWidgets, QtNetwork

class Fixture:
    def __init__(self, data):
        self.length = data["pixels"]
        self.strand_id = data["strand"]
        self.address = data["address"]
        self.contents = [(0,0,0)] * self.length
        self.pos1 = tuple(data["pos1"])
        self.pos2 = tuple(data["pos2"])
        self.total_dx = self.pos2[0] - self.pos1[0]
        self.total_dy = self.pos2[1] - self.pos1[1]
        self.pixel_dx = self.total_dx / self.length
        self.pixel_dy = self.total_dy / self.length

    @property
    def locations(self):
        return [(self.pos1[0] + i*self.pixel_dx, self.pos1[1] + i*self.pixel_dy)
                for i in range(self.length)]

class Strand:
    def __init__(self, strand_id):
        self.strand_id = strand_id
        self.fixtures = []

    def append(self, fixture):
        assert self.strand_id == fixture.strand_id
        self.fixtures.append(fixture)

    def finalize(self):
        self.fixtures.sort(key=lambda fix: fix.address)

    def update(self, contents):
        idx = 0
        for fixture in self.fixtures:
            for pixel in range(fixture.length):
                fixture.contents[pixel] = (contents[idx], contents[idx+1],
                                           contents[idx+2])
                idx += 3

    @property
    def all_locations(self):
        return sum((fixture.locations for fixture in self.fixtures), [])

    @property
    def all_contents(self):
        return sum((fixture.contents for fixture in self.fixtures), [])

class Scene(QtCore.QObject):
    frame_received = QtCore.pyqtSignal()

    def __init__(self, name):
        super().__init__()
        self.name = name
        self.filename = os.path.join(
            os.path.dirname(__file__),
            "firesim/data/scenes/{}.json".format(name))
        self.strands = {}
        self.extents = None
        self.center = None
        self._in_update = False
        self._pending_contents = {}
        self._load()

    def _load(self):
        with open(self.filename) as fp:
            data = json.loads(fp.read())

        assert data["file-type"] == "scene"
        self.center = tuple(data["center"])
        self.extents = tuple(data["extents"])

        for strand_data in data["strand-settings"]:
            strand_id = strand_data["id"]
            if not strand_data.get("enabled", True):
                self.strands[strand_id] = None
            else:
                self.strands[strand_id] = Strand(strand_id)

        for fixture_data in data["fixtures"]:
            strand_id = fixture_data["strand"]
            if strand_id not in self.strands:
                # hack to work around missing settings for strand 7 in firefly15
                self.strands[strand_id] = Strand(strand_id)
            elif self.strands[strand_id] is None:
                continue
            self.strands[strand_id].append(Fixture(fixture_data))

        for strand in self.strands.values():
            if strand is not None:
                strand.finalize()

    def process_datagram(self, dgram):
        if dgram[0] == ord('B'):
            self._in_update = True

        elif not self._in_update:
            return

        elif dgram[0] == ord('E'):
            for strand_id, contents in self._pending_contents.items():
                if self.strands.get(strand_id):
                    self.strands[strand_id].update(contents)
            self._in_update = False
            self._pending_contents.clear()
            self.frame_received.emit()

        elif dgram[0] == ord('S'):
            if len(dgram) < 4 or (len(dgram) - 4) % 3 != 0:
                sys.stderr.write("malformed strand update received\n")
            else:
                self._pending_contents[dgram[1]] = dgram[4:]

        else:
            sys.stderr.write("received datagram of unknown type "
                             "{}".format(dgram[0]))

class SimWindow(QtGui.QOpenGLWindow):
    def __init__(self, scene):
        super().__init__()
        self.scene = scene

        # strand ID -> list of (x,y) pixel locations in screen coordinates
        self.locations = {}

        self.socket = QtNetwork.QUdpSocket(self)
        self.socket.readyRead.connect(self.process_datagrams)
        self.socket.bind(3020, QtNetwork.QUdpSocket.ShareAddress |
                               QtNetwork.QUdpSocket.ReuseAddressHint)

        fmt = QtGui.QSurfaceFormat()
        fmt.setDepthBufferSize(24)
        fmt.setStencilBufferSize(8)
        self.setFormat(fmt)

        self._blur = False
        self._fullscreen = False

    def keyPressEvent(self, ev):
        if ev.key() == QtCore.Qt.Key_F:
            self._fullscreen = not self._fullscreen
            self.setWindowState(
                QtCore.Qt.WindowFullScreen if self._fullscreen else
                QtCore.Qt.WindowNoState)

        elif ev.key() == QtCore.Qt.Key_B:
            self._blur = not self._blur
            self.update()

    def resizeGL(self, width, height):
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

    def paintGL(self):
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

    def process_datagrams(self):
        while self.socket.hasPendingDatagrams():
            (datagram, sender, sport) = self.socket.readDatagram(
                self.socket.pendingDatagramSize())
            self.scene.process_datagram(datagram)

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    scene = Scene("firefly2015")
    window = SimWindow(scene)
    window.resize(640, 480)
    window.show()
    window.update()
    scene.frame_received.connect(window.update)
    app.exec_()

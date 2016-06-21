#ifndef SIMWINDOW_H
#define SIMWINDOW_H

#include <QOpenGLWindow>
#include <QUdpSocket>
#include <QKeyEvent>

#include "scene.h"


class SimWindow : public QOpenGLWindow
{
    Q_OBJECT

public:
    SimWindow(quint16 udpPort);
    ~SimWindow();

    void keyPressEvent(QKeyEvent *ev);

    void resizeGL(int w, int h);
    void paintGL();

public slots:
    void processDatagrams();

private:
    bool _blur;
    bool _fullscreen;
    QUdpSocket *_socket;
    Scene *_scene;
};

#endif // SIMWINDOW_H

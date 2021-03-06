#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QSettings>

class Options : public QObject
{
    Q_OBJECT
private:
    explicit Options();
    ~Options();
public:
    static Options options;
    quint16 getFPS();
    bool getLimitedFPS();
    quint16 getFinalFPS();
    bool getZoomEnabled();
signals:
    void newFPS(const quint16 &fps);
    void newLimitedFPS(const bool &fpsLimited);
    void newFinalFPS(const quint16 &fps);
    void newZoomEnabled(const bool &zoom);
public slots:
    void setFPS(const quint16 &fps);
    void setLimitedFPS(const bool &limitedFPS);
    void setZoomEnabled(const bool &zoom);
private:
    QSettings *settings;
    quint16 fps;
    bool limitedFPS;
    bool zoom;
};

#endif // OPTIONS_H

#ifndef _INACTIVITY_DETECTOR_
#define _INACTIVITY_DETECTOR_

#include <QObject>

namespace Controller {
    class NinjamController;
}

class BlinkableButton;

class InactivityDetector : public QObject
{
    Q_OBJECT

public:
    InactivityDetector(QWidget *parent, BlinkableButton *button, uint intervalsBeforeWarning);

public slots:
    void reset();
    void initialize(Controller::NinjamController *controller);
    void deinitialize();

signals:
    void userIsInactive();

private slots:
    void processIntervalBegin();

private:

    uint intervalsWithoutTransmit; // count how many intervals the xmit button is unchecked
    uint intervalsBlinking;
    uint intervalsBeforeWarning;

    BlinkableButton *button;

    Controller::NinjamController *controller;

};

#endif

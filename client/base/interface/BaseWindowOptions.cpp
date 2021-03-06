#include "BaseWindow.h"
#include "ui_BaseWindow.h"
#include "../Options.h"

using namespace CatchChallenger;

void BaseWindow::on_checkBoxZoom_toggled(bool checked)
{
    Options::options.setZoomEnabled(checked);
    if(Options::options.getZoomEnabled())
        MapController::mapController->setScale(2);
    else
        MapController::mapController->setScale(1);
}

void BaseWindow::on_checkBoxLimitFPS_toggled(bool checked)
{
    Options::options.setLimitedFPS(checked);
    MapController::mapController->setTargetFPS(Options::options.getFinalFPS());
}

void BaseWindow::on_spinBoxMaxFPS_editingFinished()
{
    Options::options.setFPS(ui->spinBoxMaxFPS->value());
    MapController::mapController->setTargetFPS(Options::options.getFinalFPS());
}

void BaseWindow::loadSettings()
{
    ui->checkBoxZoom->setChecked(Options::options.getZoomEnabled());
    ui->checkBoxLimitFPS->setChecked(Options::options.getLimitedFPS());
    ui->spinBoxMaxFPS->setValue(Options::options.getFPS());
    if(Options::options.getZoomEnabled())
        MapController::mapController->setScale(2);
    MapController::mapController->setTargetFPS(Options::options.getFinalFPS());
}

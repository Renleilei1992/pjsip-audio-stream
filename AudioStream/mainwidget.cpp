#include <QMessageBox>

#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    m_audio.SetAudioDir(AUDIO_DIR_ENCODING_DECODING);

    ui->stopBtn->setEnabled(false);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::on_startBtn_clicked()
{
    if (PJ_SUCCESS != m_audio.SoundStreamCreate(ui->localPortEdt->text().toUInt(),
                                                ui->remoteIPEdt->text().toStdString().c_str(),
                                                ui->remotePortEdt->text().toUInt()))
    {
        QMessageBox::warning(this, tr("AudioStream"), "启动失败", QMessageBox::Ok);
        return;
    }
    ui->stopBtn->setEnabled(true);
    ui->startBtn->setEnabled(false);
}

void MainWidget::on_stopBtn_clicked()
{
    m_audio.SoundStreamDestroy();
    ui->stopBtn->setEnabled(false);
    ui->startBtn->setEnabled(true);
}

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

#include "CAudioUtil.h"

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void on_startBtn_clicked();

    void on_stopBtn_clicked();

private:
    Ui::MainWidget *ui;

    CAudioUtil m_audio;
};

#endif // MAINWIDGET_H

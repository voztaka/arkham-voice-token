#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tokenmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCallButtonClicked();
    void onTokenCountChanged(int value);

private:
    void initializeUI();
    void updateStatistics();

    Ui::MainWindow *ui;
    TokenManager *m_tokenManager;
};
#endif // MAINWINDOW_H
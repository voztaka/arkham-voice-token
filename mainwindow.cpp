#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tokenManager(new TokenManager(this))
{
    ui->setupUi(this);
    initializeUI();
    updateStatistics();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Token count controls
    QGroupBox *tokenGroup = new QGroupBox("Token Counts", centralWidget);
    QGridLayout *tokenLayout = new QGridLayout(tokenGroup);

    const auto& tokenCounts = m_tokenManager->getAllTokenCounts();
    QStringList tokens;
    for (const auto& [token, _] : tokenCounts) {
        tokens.append(QString::fromStdString(token));
    }
    
    int row = 0, col = 0;
    for (const QString &token : tokens) {
        QLabel *label = new QLabel(token, tokenGroup);
        QSpinBox *spinBox = new QSpinBox(tokenGroup);
        spinBox->setRange(0, 99);
        spinBox->setValue(m_tokenManager->getTokenCount(token.toStdString()));
        spinBox->setProperty("token", token);
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
                this, &MainWindow::onTokenCountChanged);
        
        tokenLayout->addWidget(label, row, col * 2);
        tokenLayout->addWidget(spinBox, row, col * 2 + 1);
        col++;
        if (col >= 2) {
            col = 0;
            row++;
        }
    }

    // Call button
    QPushButton *callButton = new QPushButton("Call Token", centralWidget);
    connect(callButton, &QPushButton::clicked, this, &MainWindow::onCallButtonClicked);

    // Statistics display
    QGroupBox *statsGroup = new QGroupBox("Statistics", centralWidget);
    QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);
    QTextEdit *statsText = new QTextEdit(statsGroup);
    statsText->setReadOnly(true);
    statsText->setObjectName("statsText");
    statsLayout->addWidget(statsText);

    mainLayout->addWidget(tokenGroup);
    mainLayout->addWidget(callButton);
    mainLayout->addWidget(statsGroup);

    setCentralWidget(centralWidget);
    setWindowTitle("Arkham Voice Token");
    resize(600, 500);
}

void MainWindow::onCallButtonClicked() {
    QString token = QString::fromStdString(m_tokenManager->getRandomToken());
    if (!token.isEmpty()) {
        // Fix path construction to use proper separators
        QString audioFile = QCoreApplication::applicationDirPath() + 
                           "/resources/" + 
                           token.replace(" ", "_") + ".mp3";  // Replace spaces with underscores
        
        // For debugging
        qDebug() << "Playing file:" << audioFile;
        
        // Check if the file exists
        QFileInfo fileInfo(audioFile);
        if (!fileInfo.exists()) {
            qWarning() << "Audio file not found:" << audioFile;
            return;
        }
        
        // Use PlaySound instead of mciSendString for more reliable playback
        PlaySoundW(audioFile.toStdWString().c_str(), NULL, SND_FILENAME | SND_SYNC);
        
        updateStatistics();
    }
}


void MainWindow::onTokenCountChanged(int value)
{
    QSpinBox *spinBox = qobject_cast<QSpinBox*>(sender());
    if (spinBox) {
        QString token = spinBox->property("token").toString();
        m_tokenManager->setTokenCount(token.toStdString(), value);
    }
}

void MainWindow::updateStatistics()
{
    QTextEdit *statsText = findChild<QTextEdit*>("statsText");
    if (!statsText) return;

    QString stats;
    auto usages = m_tokenManager->getAllTokenUsages();
    auto counts = m_tokenManager->getAllTokenCounts();

    for (const auto& [token, usage] : usages) {
        stats += QString("%1:\n  Count: %2\n  Times Drawn: %3\n\n")
                .arg(QString::fromStdString(token))
                .arg(counts[token])
                .arg(usage);
    }

    statsText->setText(stats);
}
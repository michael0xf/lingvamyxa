#include "MainWindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "core.lm1.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      resultLabel(new QLabel(this)),
      button(new QPushButton("Call pure C core", this)) {

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    auto *title = new QLabel("Lingvamyxa Qt shell", this);
    resultLabel->setText(QString("Core version: %1").arg(lm_version()));

    layout->addWidget(title);
    layout->addWidget(resultLabel);
    layout->addWidget(button);

    setCentralWidget(central);
    setWindowTitle("Lingvamyxa");

    connect(button, &QPushButton::clicked, this, [this]() {
        const int result = lm_add(2, 3);
        resultLabel->setText(QString("lm_add(2, 3) = %1").arg(result));
    });
}

#include "MainWindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      resultLabel(new QLabel(this)),
      button(new QPushButton("Ready", this)) {

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    auto *title = new QLabel("Lingvamyxa Qt shell", this);
    resultLabel->setText("Parser tools are built separately.");

    layout->addWidget(title);
    layout->addWidget(resultLabel);
    layout->addWidget(button);

    setCentralWidget(central);
    setWindowTitle("Lingvamyxa");

    connect(button, &QPushButton::clicked, this, [this]() {
        resultLabel->setText("Ready");
    });
}

#include "comicbookreader.h"

#include "ui_comicbookreader.h"

ComicBookReader::ComicBookReader(QWidget* parent) : QMainWindow(parent), ui(new Ui::ComicBookReader) {
	ui->setupUi(this);
	ui->testingLabel->setText("Qt6 + CMake + C++ working 100%!");
}

ComicBookReader::~ComicBookReader() {
	delete ui;
}

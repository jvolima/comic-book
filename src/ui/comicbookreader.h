#pragma once

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
	class ComicBookReader;
}
QT_END_NAMESPACE

class ComicBookReader : public QMainWindow {
	Q_OBJECT

public:
	explicit ComicBookReader(QWidget* parent = nullptr);
	~ComicBookReader();

private:
	Ui::ComicBookReader* ui;
	QString currentFile;
};

#include <QApplication>

#include "archives/builtin_archive_formats.hpp"
#include "comicbookreader.h"

int main(int argc, char* argv[]) {
	registerBuiltinArchiveFormats();
	QApplication a(argc, argv);
	ComicBookReader w;
	w.show();
	return a.exec();
}

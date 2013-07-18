#include "ui/ArxProfiler.h"
#include <QApplication>

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

INT WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, INT) {
	
	QApplication app(__argc, __argv);
	
#else

int main(int argc, char **argv) {
	
	QApplication app(argc, argv);
	
#endif

	ArxProfiler w;
	w.show();

	return app.exec();
}

#include "MainWindow.h"

int main()
{
	MainWindow MainWindow;
	int init_value = MainWindow.Initialisation();
	if (init_value != 0) {
		// There was a problem during the initialization
		// imediately quit the application
		return init_value;
	}

	return MainWindow.RenderLoop();
}
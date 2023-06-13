#include <Application.h>

int main(int argc, char** argv)
{
	Dashlane::CApplication application;

	if (application.Initialize())
	{
		return application.Run(argc, argv);
	}

	return -1;
}
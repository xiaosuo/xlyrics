#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

#include <audacious/plugin.h>
#include <audacious/dbus.h>
#include <audacious/audctrl.h>

static int child;
void start_plugin(void);
void cleanup(void);

GeneralPlugin gp = {
	NULL,
	NULL,
	-1,
	"Xlyrics Plugin",
	start_plugin,
	NULL,
	NULL,
	cleanup
};

GeneralPlugin * get_gplugin_info(void)
{
	return &gp;
}

void start_plugin(void)
{
	child = fork();
	if(child == 0)
	{
		execlp("xlyrics", "xlyrics", "liblyrics_audacious.so", 0);
	} else if(child > 0){
		waitpid(child, NULL, WNOHANG);
	}
}

void cleanup(void)
{
	if(child != 0)
		kill(child, SIGKILL);
}

static GeneralPlugin xlyrics_plugin =
{
	.description = "Xlryics",
	.init = start_plugin,
	.cleanup = cleanup,
};

GeneralPlugin *xlyrics_gplist[] = { &xlyrics_plugin, NULL };
SIMPLE_GENERAL_PLUGIN(xlyrics, xlyrics_gplist);


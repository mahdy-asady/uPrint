#include <gcc-plugin.h>
#include "plugin-version.h"

// GCC GPL Compatibile verification signature
int plugin_is_GPL_compatible;

#define PLUGIN_NAME     "uPrint"


/* Additional information about the plugin. Used by --help and --version. */
static struct plugin_info pluginRegisterInfo =
{
	.version = "0.1",
	.help = "Not implemented yet!",
};

int plugin_init ( struct plugin_name_args *pluginInfo, struct plugin_gcc_version *version) {
	// tell to GCC some info about the plugin
	register_callback(PLUGIN_NAME, PLUGIN_INFO, NULL, &pluginRegisterInfo);

	printf("%s plugin v%s loaded!\n", PLUGIN_NAME, pluginRegisterInfo.version);

	// done!
	return 0;
}

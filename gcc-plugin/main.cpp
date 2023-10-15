#include <gcc-plugin.h>
#include "plugin-version.h"

// GCC GPL Compatibile verification signature
int plugin_is_GPL_compatible;

/**
 * Name of this plugin
 */
#define PLUGIN_NAME         "uPrint"

/**
 * Version of this plugin
 */
#define PLUGIN_VERSION      "0.1"

/* Additional information about the plugin. Used by --help and --version. */
static struct plugin_info pluginRegisterInfo = {
    .version = PLUGIN_VERSION,
    .help    = "This plugin converts formatted output to binary output.",
};

int plugin_init ( struct plugin_name_args *pluginInfo, struct plugin_gcc_version *version) {
	// tell to GCC some info about the plugin
	register_callback(PLUGIN_NAME, PLUGIN_INFO, NULL, &pluginRegisterInfo);

	printf("%s plugin v%s loaded!\n", PLUGIN_NAME, pluginRegisterInfo.version);

	// done!
	return 0;
}

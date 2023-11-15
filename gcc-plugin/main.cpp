#include <gcc-plugin.h>
#include <tree.h>
#include <gimple.h>
#include <tree-pass.h>
#include <gimple-iterator.h>

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


#define INTERFACE_FN_NAME   "uPrint"

#define OUTPUT_FN_NAME      "uPrintHelper"

// -----------------------------------------------------------------------------
// GCC EXTERNAL DECLARATION
// -----------------------------------------------------------------------------

/**
 * Takes a tree node and returns the identifier string
 * @see https://gcc.gnu.org/onlinedocs/gccint/Identifiers.html
 */
#define FN_NAME(tree_fun) IDENTIFIER_POINTER (DECL_NAME (tree_fun))

/**
 * The global singleton context aka "g". The name is chosen to be easy to type
 * in a debugger. Represents the 'global state' of GCC
 *
 * GCC's internal state can be divided into zero or more "parallel universe" of
 * state; an instance of the class context is one such context of state
 *
 * @see Declared in context.h
 */
extern gcc::context *g;

// -----------------------------------------------------------------------------
// PLUGIN INSTRUMENTATION LOGICS
// -----------------------------------------------------------------------------

/**
 * Metadata for a pass, non-varying across all instances of a pass
 * @see Declared in tree-pass.h
 * @note Refer to tree-pass for docs about
 */
struct pass_data ins_pass_data =
{
    .type = GIMPLE_PASS,                                    // type of pass
    .name = PLUGIN_NAME,                                    // name of plugin
    .optinfo_flags = OPTGROUP_NONE,                         // no opt dump
    .tv_id = TV_NONE,                                       // no timevar (see timevar.h)
    .properties_required = PROP_gimple_any,                 // entire gimple grammar as input
    .properties_provided = 0,                               // no prop in output
    .properties_destroyed = 0,                              // no prop removed
    .todo_flags_start = 0,                                  // need nothing before
    .todo_flags_finish = TODO_update_ssa|TODO_cleanup_cfg   // need to update SSA repr after and repair cfg
};


extern void print_gimple_stmt(FILE * file, gimple* g, int spc, dump_flags_t flags);



/**
 * Definition of our instrumentation GIMPLE pass
 * @note Extends gimple_opt_pass class
 * @see Declared in tree-pass.h
 */
class ins_gimple_pass : public gimple_opt_pass {
public:

    /**
     * Constructor
     */
    ins_gimple_pass (const pass_data& data, gcc::context *ctxt) : gimple_opt_pass (data, ctxt) {}

    /**
     * This and all sub-passes are executed only if the function returns true
     * @note Defined in opt_pass father class
     * @see Defined in tree-pass.h
     */
    bool gate (function* gate_fun)
    {
        return true;
    }

    void replace_print_fn(gimple* curr_stmt) {
        fprintf(stderr, "   *** Before: "); print_gimple_stmt(stderr, curr_stmt, 0, TDF_NONE);

        // build function prototype
        tree proto = build_function_type_list(
                void_type_node,             // return type
                NULL_TREE                   // varargs terminator
            );

        // builds and returns function declaration with NAME and PROTOTYPE
        tree decl = build_fn_decl(OUTPUT_FN_NAME, proto);

        // Replace call function
        gimple_call_set_fndecl(curr_stmt, decl);

        fprintf(stderr, "   *** After: "); print_gimple_stmt(stderr, curr_stmt, 0, TDF_NONE);
        fprintf(stderr, "--------------------------------------------------\n");
    }

    /**
     * This is the code to run when pass is executed
     * @note Defined in opt_pass father class
     * @see Defined in tree-pass.h
     */
    unsigned int execute (function* exec_fun)
    {
        // get the FUNCTION_DECL of the function whose body we are reading
        tree fndef = current_function_decl;

        // print the function name
        fprintf(stderr, "\nInspecting function '%s'\n", FN_NAME(fndef));

        basic_block bb;
        gimple_stmt_iterator it;

        FOR_EACH_BB_FN(bb, cfun) {
            for (it = gsi_start_bb(bb); !gsi_end_p(it); gsi_next(&it)) {
                gimple* stmt = gsi_stmt(it);
                if (is_gimple_call(stmt)) {
                    tree function_name = gimple_call_fndecl(stmt);
                    if (function_name) {
                        const char *name_str = IDENTIFIER_POINTER(DECL_NAME(function_name));
                        if(strcmp(name_str, INTERFACE_FN_NAME) == 0) {
                            replace_print_fn(stmt);
                        }
                    }
                }
            }
        }

        // done!
        return 0;
    }
};

// instanciate a new instrumentation GIMPLE pass
ins_gimple_pass inst_pass = ins_gimple_pass(ins_pass_data, g);

// -----------------------------------------------------------------------------
// PLUGIN INITIALIZATION
// -----------------------------------------------------------------------------

/**
 * Initializes the plugin. Returns 0 if initialization finishes successfully.
 */
int plugin_init ( struct plugin_name_args *pluginInfo, struct plugin_gcc_version *version) {
	// tell to GCC some info about the plugin
	register_callback(PLUGIN_NAME, PLUGIN_INFO, NULL, &pluginRegisterInfo);

	printf("%s plugin v%s loaded!\n", PLUGIN_NAME, pluginRegisterInfo.version);


    // new pass that will be registered
    struct register_pass_info pass;

    // insert inst pass into the struct used to register the pass
    pass.pass = &inst_pass;

    // and get called after GCC has produced SSA representation
    pass.reference_pass_name = "ssa";

    // after the first opt pass to be sure opt will not throw away our stuff
    pass.ref_pass_instance_number = 1;
    pass.pos_op = PASS_POS_INSERT_AFTER;

    // add our pass hooking into pass manager
    register_callback(PLUGIN_NAME, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);

	// done!
	return 0;
}

#include <gcc-plugin.h>
#include <tree.h>
#include <gimple.h>
#include <tree-pass.h>
#include <gimple-iterator.h>

#include <print-tree.h>

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


    uint8_t saveFormatStringToDb(tree Param) {
        static uint8_t num = 1;
        tree node = TREE_OPERAND (Param, 0);
        if (unsigned nbytes = TREE_STRING_LENGTH (node)) {
            printf("STR id: %d: %s\n", num, TREE_STRING_POINTER (node));
        }
        return num++;
    }

    /**
     * @brief Generates format string used in uPrintHelper() to generate data stream.
     * The format string is just a string variable that stores length of each variable. 
     * For example in a particular system, int may stored in 4 bytes, so we will place '4' in its place to show that when picking data, pick 4 bytes.
     * For string (char *) variables we set it place holder as '0' that shows it is a string pointer
     * 
     * @param stmt 
     * @param format_str: returning format string
     * @return int length of format string
     */
    int generateFormatString(gimple* stmt, char **format_str) {
        int arg_count = gimple_call_num_args(stmt) - 2;
        *format_str = XNEWVEC(char, arg_count + 1);

        // Set default place holder
        memset(*format_str, ' ', arg_count);

        for(int i = 0; i < arg_count; i++) {
            tree arg = gimple_call_arg(stmt, i + 2);
            enum tree_code arg_code = TREE_CODE (arg);

            switch (arg_code)
            {
                // Variables
                case SSA_NAME:
                    {
                        tree arg_type = TREE_TYPE(arg);
                        enum tree_code type_code = TREE_CODE (arg_type);
                        // Integer type variable
                        if(type_code == INTEGER_TYPE) {
                            tree size_unit = TYPE_SIZE_UNIT(arg_type);
                            (*format_str)[i] = '0' + *(wi::to_wide(size_unit).get_val());
                        }
                    }
                    break;

                case INTEGER_CST:
                    {
                        tree arg_type = TREE_TYPE(arg);
                        tree size_unit = TYPE_SIZE_UNIT(arg_type);
                        (*format_str)[i] = '0' + *(wi::to_wide(size_unit).get_val());
                    }
                    break;

                // Address reference. Shall we treat it only as string pointer?
                case ADDR_EXPR:
                    (*format_str)[i] = '0';
                    break;

                default:
                    fprintf(stderr, "*********************************************************\n");
                    fprintf(stderr, "Unknown Variable type (%d)!\nDebug Info:\n", arg_code);
                    debug_tree(arg);
                    fprintf(stderr, "*********************************************************\n");
            }
        }

        (*format_str)[arg_count] ='\0';
        return (arg_count + 1);
    }


    void replace_print_fn(gimple* curr_stmt) {
        char *fmt_str;
        int fmt_length;
        vec<tree> args;

        // We have 3 arguments at minimum
        args.create(3);

        // Save format string to database & get Its ID
        uint8_t recordId = saveFormatStringToDb(gimple_call_arg(curr_stmt, 1));

        // Generate custom format string
        fmt_length = generateFormatString(curr_stmt, &fmt_str);

        // Transfer first argument(pointer of callback) as is
        args.safe_push(gimple_call_arg(curr_stmt, 0));

        // Set second arg
        tree recordIdArg = build_int_cst(NULL_TREE, recordId);
        args.safe_push(recordIdArg);

        // Set third arg
        tree fmt_arg = build_string_literal(fmt_length, fmt_str);
        XDELETEVEC(fmt_str);
        args.safe_push(fmt_arg);


        int arg_count = gimple_call_num_args(curr_stmt) - 2;

        for(int i = 0; i < arg_count; i++) {
            args.safe_push(gimple_call_arg(curr_stmt, i + 2));
        }

        // build function prototype
        tree proto = build_function_type_list(
                void_type_node,             // return type
                NULL_TREE                   // varargs terminator
            );

        // builds and returns function declaration with NAME and PROTOTYPE
        tree decl = build_fn_decl(OUTPUT_FN_NAME, proto);

        // tree decl = builtin_decl_implicit (as_builtin_fn (fn));
        gcall *stmt = gimple_build_call_vec (decl, args);

        gimple_stmt_iterator gsi = gsi_for_stmt(curr_stmt);

        gsi_replace(&gsi, stmt, true);

        args.release();
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

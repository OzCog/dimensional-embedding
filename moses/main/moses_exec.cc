/** moses-exec.cc --- 
 *
 * Copyright (C) 2010 OpenCog Foundation
 *
 * Author: Nil Geisweiller <ngeiswei@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "moses_exec.h"

#include <opencog/util/numeric.h>
#include <opencog/util/log_prog_name.h>

namespace opencog { namespace moses {

static const unsigned int max_filename_size = 255;

/**
 * Display error message about unspecified combo tree and exit
 */
void unspecified_combo_exit() {
    std::cerr << "error: you must specify which combo tree to learn (option -y)"
              << std::endl;
    exit(1);
}

/**
 * Display error message about unsupported type and exit
 */
void unsupported_type_exit(const type_tree& tt) {
    std::cerr << "error: type " << tt << " currently not supported" << std::endl;
    exit(1);
}
void unsupported_type_exit(type_node type) {
    unsupported_type_exit(type_tree(type));
}

/**
 * Display error message about ill formed combo tree and exit
 */
void illformed_exit(const combo_tree& tr) {
    std::cerr << "error: apparently the combo tree " 
              << tr << "is not well formed" << std::endl;
    exit(1);
}

/**
 * Display error message about unsupported problem and exit
 */
void unsupported_problem_exit(const string& problem) {
    std::cerr << "error: problem " << problem 
              << " unsupported for the moment" << std::endl;
    exit(1);
}

/**
 * Display error message about not recognized combo operator and exist
 */
void not_recognized_combo_operator(const string& ops_str) {
    std::cerr << "error: " << ops_str
              << " is not recognized as combo operator" << std::endl;
    exit(1);
}

/**
 * determine the initial exemplar of a given type
 */
combo_tree type_to_exemplar(type_node type) {
    switch(type) {
    case id::boolean_type: return combo_tree(id::logical_and);
    case id::contin_type: return combo_tree(id::plus);
    case id::ill_formed_type:
        std::cerr << "The data type is incorrect, perhaps it has not been"
                  << " possible to infer it from the input table." << std::endl;
        exit(1);
    default:
        unsupported_type_exit(type);
    }
    return combo_tree();
}

combo_tree ann_exemplar(arity_t arity) {
    combo_tree ann_tr(ann_type(0, id::ann));
    // ann root
    combo_tree::iterator root_node = ann_tr.begin();
    // output node
    combo_tree::iterator output_node =
        ann_tr.append_child(root_node, ann_type(1, id::ann_node));
    // input nodes
    for(arity_t i = 0; i <= arity; ++i) 
        ann_tr.append_child(output_node, ann_type(i + 2, id::ann_input));
    // input nodes' weights
    ann_tr.append_children(output_node, 0.0, arity + 1);
    
    return ann_tr;
}

/**
 * determine the alphabet size given the type_tree of the problem and
 * the of operator that are ignored
 */
int alphabet_size(const type_tree& tt, const vertex_set ignore_ops) {
    arity_t arity = type_tree_arity(tt);
    type_node output_type = *type_tree_output_type_tree(tt).begin();
    if(output_type == id::boolean_type) {
        return 3 + arity;
    } else if(output_type == id::contin_type) {
        // set alphabet size, 8 is roughly the number of operators
        // in contin formula, it will have to be adapted
        return 8 + arity - ignore_ops.size();
    } else if(output_type == id::ann_type) {
        return 2 + arity*arity; // to account for hidden neurones, very roughly
    } else {
        unsupported_type_exit(tt);
        return 0;
    }
}

// combo_tree
combo_tree str_to_combo_tree(const string& combo_str) {
    stringstream ss;
    combo_tree tr;
    ss << combo_str;
    ss >> tr;
    return tr;
}

// infer the arity of the problem
arity_t infer_arity(const string& problem,                    
                    unsigned int problem_size,
                    const string& input_table_file,
                    const string& combo_str) {
    if(problem == it || problem == ann_it)
        return dataFileArity(input_table_file);
    else if(problem == cp || problem == ann_cp) {
        if(combo_str.empty())
            unspecified_combo_exit();
        // get the combo_tree and infer its type
        combo_tree tr = str_to_combo_tree(combo_str);
        type_tree tt = infer_type_tree(tr);
        if(is_well_formed(tt)) {
            return type_tree_arity(tt);
        } else {
            illformed_exit(tr);
            return -1;
        }
    } else if(problem == pa || problem == dj || problem == mux) {
        return problem_size;
    } else if(problem == sr) {
        return 1;
    } else {
        unsupported_problem_exit(problem);
        return -1;
    }
}

// returns n such that a = n+2^n
arity_t multiplex_arity(arity_t a) {
    unsigned nearest_arity = 1;
    for(unsigned n = 1; n <= integer_log2(a); ++n) {
        nearest_arity = n + pow2(n);
        if(nearest_arity == (unsigned)a)
            return n;
    }
    // not found, exit
    std::cerr << "Error: for multiplex the arity " << a
              << " must be equal to n+2^n, but no such n exists."
              << " However, the arity " << nearest_arity << " would work."
              << std::endl;
    exit(1);
    return -1;
}

int moses_exec(int argc, char** argv) { 
    // for(int i = 0; i < argc; ++i)
    //     std::cout << "arg = " << argv[i] << std::endl;

    // program options, see options_description below for their meaning
    unsigned long rand_seed;
    string input_data_file;
    string target_feature;
    string problem;
    string combo_str;
    unsigned int problem_size;
    int nsamples;
    float min_rand_input;
    float max_rand_input;
    unsigned long max_evals;
    long result_count;
    bool output_score;
    bool output_complexity;
    bool output_bscore;
    bool output_eval_number;
    bool output_with_labels;
    string output_file;
    int max_gens;
    string log_level;
    string log_file;
    bool log_file_dep_opt;
    float variance;
    float prob;
    vector<string> include_only_ops_str;
    vector<string> ignore_ops_str;
    string opt_algo; //optimization algorithm
    vector<string> exemplars_str;
    int reduct_candidate_effort;
    int reduct_knob_building_effort;
    bool enable_cache;
    vector<string> jobs_str;
    bool weighted_accuracy;
    // metapop_param
    int max_candidates;
    bool reduce_all;
    bool revisit = false;
    bool include_dominated;
    // optim_param
    double pop_size_ratio;
    double max_score;
    double max_dist_ratio;
    // hc_param
    bool hc_terminate_if_improvement;
    // continuous optimization
    vector<contin_t> discretize_thresholds;

    // Declare the supported options.
    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Produce help message.\n")
        (opt_desc_str(rand_seed_opt).c_str(),
         value<unsigned long>(&rand_seed)->default_value(1),
         "Random seed.\n")
        (opt_desc_str(max_evals_opt).c_str(),
         value<unsigned long>(&max_evals)->default_value(10000),
         "Maximum number of fitness function evaluations.\n")
        (opt_desc_str(result_count_opt).c_str(),
         value<long>(&result_count)->default_value(10),
         "The number of non-dominated best results to return ordered according to their score, if negative then returns all of them.\n")
        (opt_desc_str(output_score_opt).c_str(),
         value<bool>(&output_score)->default_value(true),
         "If 1, outputs the score before each candidate (at the left of the complexity).\n")
        (opt_desc_str(output_complexity_opt).c_str(),
         value<bool>(&output_complexity)->default_value(false),
         "If 1, outputs the complexity before each candidate (at the right of the score).\n")
        (opt_desc_str(output_bscore_opt).c_str(),
         value<bool>(&output_bscore)->default_value(false),
         "If 1, outputs the bscore below each candidate.\n")
        (opt_desc_str(output_eval_number_opt).c_str(),
         value<bool>(&output_eval_number)->default_value(false),
         "If 1, outputs the actual number of evaluations.\n")
        (opt_desc_str(output_with_labels_opt).c_str(),
         value<bool>(&output_with_labels)->default_value(false),
         "If 1, outputs the candidates with their labels instead of place holders. for instance *(\"#price\" \"#temprature\") instead of *(#1 #2). This only works for data fitting problems where the data file contains labels in its header\n")
        (opt_desc_str(output_file_opt).c_str(),
         value<string>(&output_file)->default_value(""),
         "File where to save the results. If empty then it outputs on the stdout.\n")
        (opt_desc_str(max_gens_opt).c_str(),
         value<int>(&max_gens)->default_value(-1),
         "Maximum number of demes to generate and optimize, negative means no generation limit.\n")
        (opt_desc_str(input_data_file_opt).c_str(),
         value<string>(&input_data_file),
         "Input table file in DSL format (where the delimiters are comma, whitespace and tabulation), the maximum number of samples is the number of rows in the file.\n")
        (opt_desc_str(target_feature_opt).c_str(),
         value<string>(&target_feature),
         "Label of the target feature to fit. If none is given the first one is used.\n")
        (opt_desc_str(problem_opt).c_str(),
         value<string>(&problem)->default_value("it"),
         string("Problem to solve, supported problems are\n"
                "regression based on input table (").append(it).
         append("),\n" "regression based on input table using ann (").append(ann_it).
         append("),\n" "regression based on combo program (").append(cp).
         append("),\n" "even parity (").append(pa).
         append("),\n" "disjunction (").append(dj).
         append("),\n" "multiplex (").append(mux).
         append("),\n" "regression of f(x)_o = sum_{i={1,o}} x^i (").append("sr").
         append(").\n").c_str())
        (opt_desc_str(combo_str_opt).c_str(),
         value<string>(&combo_str),
         string("Combo program to learn, use when the problem ").append(cp).append(" is selected (option -").append(problem_opt.second).append(").\n").c_str())
        (opt_desc_str(problem_size_opt).c_str(),
         value<unsigned int>(&problem_size)->default_value(5),
         string("For even parity (").append(pa).
         append("), disjunction (").append(dj).
         append(") and multiplex (").append(mux).
         append(") the problem size corresponds to the arity.").
         append(" Note that for multiplex (").append(mux).
         append(") the problem size must be equal to n+2^n.").
         append(" For regression of f(x)_o = sum_{i={1,o}} x^i (").append(sr).
         append(") the problem size corresponds to the order o.\n").c_str())
        (opt_desc_str(nsamples_opt).c_str(),
         value<int>(&nsamples)->default_value(-1),
         "Number of samples to describ the problem. If nsample is negative, null or larger than the maximum number of samples allowed it is ignored. If the default problem size is larger than the value provided with that option then the dataset is subsampled randomly to reach the target size.\n")
        (opt_desc_str(min_rand_input_opt).c_str(),
         value<float>(&min_rand_input)->default_value(0),
         "Min of an input value chosen randomly, only used when the problem takes continuous inputs.\n")
        (opt_desc_str(max_rand_input_opt).c_str(),
         value<float>(&max_rand_input)->default_value(1),
         "Max of an input value chosen randomly, only used when the problem takes continuous inputs.\n")
        (opt_desc_str(log_level_opt).c_str(),
         value<string>(&log_level)->default_value("DEBUG"),
         "Log level, possible levels are NONE, ERROR, WARN, INFO, DEBUG, FINE. Case does not matter.\n")
        (opt_desc_str(log_file_dep_opt_opt).c_str(),
         string("The name of the log is determined by the options, for instance if moses-exec is called with -r 123 -H pa the log name is moses_random-seed_123_problem_pa.log. Note that the name will be truncated in order not to be longer than ").append(lexical_cast<string>(max_filename_size)).append(" characters.\n").c_str())
        (opt_desc_str(log_file_opt).c_str(),
         value<string>(&log_file)->default_value(default_log_file),
         string("File name where to write the log. This option is overwritten by ").append(log_file_dep_opt_opt.first).append(".\n").c_str())
        (opt_desc_str(variance_opt).c_str(),
         value<float>(&variance)->default_value(0),
         "In the case of contin regression. variance of an assumed Gaussian around each candidate's output, useful if the data are noisy or to control an Occam's razor bias, 0 or negative means no Occam's razor, otherwise the higher v the stronger the Occam's razor.\n")
        (opt_desc_str(prob_opt).c_str(),
         value<float>(&prob)->default_value(0),
         "In the case of boolean regression, probability that an output datum is wrong (returns false while it should return true or the other way around), useful if the data are noisy or to control an Occam's razor bias, only values 0 < p < 0.5 are meaningful, out of this range it means no Occam's razor, otherwise the greater p the greater the Occam's razor.\n")
        (opt_desc_str(include_only_ops_str_opt).c_str(),
         value<vector<string> >(&include_only_ops_str),
         "Include only the operator in the solution, can be used several times, for the moment only plus, times, div, sin, exp, log and variables (#n) are supported. Note that variables and operators are decoralated (including only some operators still include all variables and including only some variables still include all operators). You may need to put variables under double quotes. This option does not work with ANN.\n")
        (opt_desc_str(ignore_ops_str_opt).c_str(),
         value<vector<string> >(&ignore_ops_str),
         string("Ignore the following operator in the program solution, can be used several times, for the moment only div, sin, exp, log and variables (#n) can be ignored. You may need to put variables under double quotes. This option has the priority over ").append(include_only_ops_str_opt.first).append(". That is if an operator is both be included and ignored, it is ignored. This option does not work with ANN.\n").c_str())
        (opt_desc_str(opt_algo_opt).c_str(),
         value<string>(&opt_algo)->default_value(hc),
         string("Optimization algorithm, supported algorithms are"
                " univariate (").append(un).
         append("), simulation annealing (").append(sa).
         append("), hillclimbing (").append(hc).append(").\n").c_str())
        (opt_desc_str(exemplars_str_opt).c_str(),
         value<vector<string> >(&exemplars_str),
         "Start the search with a given exemplar, can be used several times.\n")
        (opt_desc_str(max_candidates_opt).c_str(),
         value<int>(&max_candidates)->default_value(-1),
         "Maximum number of considered candidates to be added to the metapopulation after optimizing deme.\n")
        (opt_desc_str(reduce_all_opt).c_str(),
         value<bool>(&reduce_all)->default_value(true),
         "Reduce all candidates before being evaluated, otherwise there are only reduced before being added to the metapopulation. This option can be valuable if the cache is enabled to not re-evaluate duplicates.\n")
        (opt_desc_str(reduct_candidate_effort_opt).c_str(),         
         value<int>(&reduct_candidate_effort)->default_value(2),
         "Effort allocated for reduction of candidates, 0-3, 0 means minimum effort, 3 means maximum effort.\n")
        (opt_desc_str(reduct_knob_building_effort_opt).c_str(),
         value<int>(&reduct_knob_building_effort)->default_value(2),
         "Effort allocated for reduction during knob building, 0-3, 0 means minimum effort, 3 means maximum effort. The bigger the effort the lower the dimension of the deme.\n")
        (opt_desc_str(enable_cache_opt).c_str(),
         value<bool>(&enable_cache)->default_value(true),
         "Cache, so that identical candidates are not re-evaluated, the cache size is dynamically adjusted to fit in the RAM.\n")
        (opt_desc_str(jobs_opt).c_str(),
         value<vector<string> >(&jobs_str),
         string("Number of jobs allocated for deme optimization. Jobs can be executed on a remote machine as well, in such case the notation -j N:REMOTE_HOST is used. For instance one can enter the options -j 4 -j 16").append(job_seperator).append("my_server.org (or -j 16").append(job_seperator).append("user@my_server.org if wishes to run the remote jobs under a different user name), meaning that 4 jobs are allocated on the local machine and 16 jobs are allocated on my_server.org. The assumption is that moses-exec must be on the remote machine and is located in a directory included in the PATH environment variable. Beware that a lot of log files are gonna be generated when using this option.\n").c_str())
        (opt_desc_str(weighted_accuracy_opt).c_str(),
         value<bool>(&weighted_accuracy)->default_value(false),
         "This option is useful in case of unbalanced data as it weights the score so that each class weights equally regardless of their proportion in terms of sample size.\n")
        (opt_desc_str(pop_size_ratio_opt).c_str(),
         value<double>(&pop_size_ratio)->default_value(20),
         "The higher the more effort is spent on a deme.\n")
        (opt_desc_str(max_score_opt).c_str(),
         value<double>(&max_score)->default_value(0),
         "The max score to reach, once reached MOSES halts. MOSES is sometimes able to calculate the max score that can be reached for a particular problem, in such case the max_score is automatically reset of the minimum between MOSES's calculation and the user's option.\n")
        (opt_desc_str(max_dist_ratio_opt).c_str(),
         value<double>(&max_dist_ratio)->default_value(1),
         "The max distance from the exemplar to explore a deme is determined by that value * log2(information_theoretic_bits(deme)).\n")
        (opt_desc_str(include_dominated_opt).c_str(),
         value<bool>(&include_dominated)->default_value(false),
         "Include dominated candidates (according behavioral score) when merging candidates in the metapopulation. Faster merging but results in a very large metapopulation.\n")
        (opt_desc_str(hc_terminate_if_improvement_opt).c_str(),
         value<bool>(&hc_terminate_if_improvement)->default_value(true),
         "Hillclimbing parameter. If 1 then deme search terminates when an improvement is found, if 0 it keeps searching until another termination condition is met.\n")
        (opt_desc_str(discretize_threshold_opt).c_str(),
         value<vector<contin_t> >(&discretize_thresholds),
         "If the domain is continuous, discretize the target feature. A unique used of that option produces 2 classes, x < thresold and x >= threshold. The option can be used several times (n-1) to produce n classes and the thresholds are automatically sorted.\n")
        ;

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    // set flags
    log_file_dep_opt = vm.count(log_file_dep_opt_opt.first) > 0;
    
    if (vm.count("help") || argc == 1) {
        cout << desc << "\n";
        return 1;
    }

    // set log
    if(log_file_dep_opt) {
        std::set<std::string> ignore_opt{log_file_dep_opt_opt.first};
        log_file = determine_log_name(default_log_file_prefix,
                                      vm, ignore_opt,
                                      std::string(".").append(default_log_file_suffix));
    }
    // remove log_file
    remove(log_file.c_str());
    logger().setFilename(log_file);
    logger().setLevel(logger().getLevelFromString(log_level));
    logger().setBackTraceLevel(Logger::ERROR);

    // init random generator
    MT19937RandGen rng(rand_seed);

    // infer arity
    arity_t arity = infer_arity(problem, problem_size, input_data_file, combo_str);

    // convert include_only_ops_str to the set of actual operators to
    // ignore
    vertex_set ignore_ops;
    if(vm.count(include_only_ops_str_opt.first.c_str())) {
        bool ignore_arguments = false;
        bool ignore_operators = false;
        foreach(const string& s, include_only_ops_str) {
            vertex v;
            if(builtin_str_to_vertex(s, v)) {
                if(!ignore_operators) {
		  ignore_ops = {id::plus, id::times, id::div,
				id::exp, id::log, id::sin};
                    ignore_operators = true;
                }
                ignore_ops.erase(v);
            } else if(argument_str_to_vertex(s, v)) {
                if(!ignore_arguments) {
                    for(arity_t arg = 1; arg <= arity; ++arg)
                        ignore_ops.insert(argument(arg));
                    ignore_arguments = true;
                }
                ignore_ops.erase(v);
            } else not_recognized_combo_operator(s);
        }
    }

    // convert ignore_ops_str to the set of actual operators to ignore
    foreach(const string& s, ignore_ops_str) {
        vertex v;
        if(builtin_str_to_vertex(s, v) || argument_str_to_vertex(s, v))
            ignore_ops.insert(v);
        else not_recognized_combo_operator(s);
    }

    // set the initial exemplars
    vector<combo_tree> exemplars;
    foreach(const string& exemplar_str, exemplars_str) {
        exemplars.push_back(str_to_combo_tree(exemplar_str));
    }

    // fill jobs
    jobs_t jobs{{localhost, 1}}; // by default the localhost has 1 job
    bool only_local = true;
    foreach(const string& js, jobs_str) {
        size_t pos = js.find(job_seperator);
        if(pos != string::npos) {
            unsigned int nj = boost::lexical_cast<unsigned int>(js.substr(0, pos));
            string host_name = js.substr(pos + 1);
            jobs[host_name] = nj;
            only_local = false;
        } else {
            jobs[localhost] = boost::lexical_cast<unsigned int>(js);
        }
    }

    // set metapopulation parameters
    metapop_parameters meta_params(max_candidates, reduce_all,
                                   revisit, include_dominated, jobs[localhost]);

    // set optim_parameters
    optim_parameters opt_params(pop_size_ratio, max_score, max_dist_ratio);

    // set moses_parameters
    moses_parameters moses_params(max_evals, max_gens, max_score, ignore_ops);

    // find the position of the target feature of the data file if any
    int target_pos = 0;
    if(!target_feature.empty() && !input_data_file.empty())
        target_pos = findTargetFeaturePosition(input_data_file, target_feature);

    // read labels on data file
    vector<string> labels;
    if(output_with_labels && !input_data_file.empty())
        labels = readInputLabels(input_data_file, target_pos);

    // set metapop_moses_results_parameters
    metapop_moses_results_parameters mmr_pa(result_count,
                                            output_score, output_complexity,
                                            output_bscore, output_eval_number,
                                            output_with_labels,
                                            enable_cache, labels,
                                            output_file, jobs, only_local,
                                            hc_terminate_if_improvement);

    if(problem == it) { // regression based on input table
        
        // infer the type of the input table
        type_node inferred_type = inferDataType(input_data_file);

        // determine the default exemplar to start with
        if(exemplars.empty())
            exemplars.push_back(type_to_exemplar(inferred_type));

        type_node output_type = 
            *(get_output_type_tree(*exemplars.begin()->begin()).begin());
        if(output_type == id::unknown_type) {
            output_type = inferred_type;
        }

        OC_ASSERT(output_type == inferred_type);

        unique_ptr<ifstream> in(open_data_file(input_data_file));

        if(output_type == id::boolean_type) {
            // read input_data_file file
            logger().debug("Read data file %s", input_data_file.c_str());
            truth_table table(*in, target_pos);
            if(nsamples>0)
                subsampleTable(table.input, table.output, nsamples, rng);
            ctruth_table ctable = table.compress();

            stringstream ss;
            ss << std::endl;
            ostreamCTable(ss, ctable);
            logger().debug(ss.str());

            type_tree tt = declare_function(output_type, arity);

            int as = alphabet_size(tt, ignore_ops);

            occam_ctruth_table_bscore bscore(ctable, prob, as, rng);
            metapop_moses_results(rng, exemplars, tt,
                                  logical_reduction(reduct_candidate_effort),
                                  logical_reduction(reduct_knob_building_effort),
                                  bscore, opt_algo,
                                  opt_params, meta_params, moses_params,
                                  vm, mmr_pa);
        }
        else if(output_type == id::contin_type) {
            // read input_data_file file
            contin_input_table it;
            contin_output_table ot;
            istreamTable(*in, it, ot, target_pos);
            if(nsamples>0)
                subsampleTable(it, ot, nsamples, rng);

            type_tree tt = declare_function(output_type, arity);
            int as = alphabet_size(tt, ignore_ops);

            // if no exemplar has been provided in option use the default
            // contin_type exemplar (+)
            if(exemplars.empty()) {            
                exemplars.push_back(type_to_exemplar(id::contin_type));
            }

            if(discretize_thresholds.empty()) {
                occam_contin_bscore bscore(ot, it, variance, as, rng);
                metapop_moses_results(rng, exemplars, tt,
                                      contin_reduction(ignore_ops, rng),
                                      contin_reduction(ignore_ops, rng),
                                      bscore, opt_algo,
                                      opt_params, meta_params, moses_params,
                                      vm, mmr_pa);
            } else {
                discretize_contin_bscore bscore(ot, it, discretize_thresholds,
                                                weighted_accuracy, rng);
                metapop_moses_results(rng, exemplars, tt,
                                      contin_reduction(ignore_ops, rng),
                                      contin_reduction(ignore_ops, rng),
                                      bscore, opt_algo,
                                      opt_params, meta_params, moses_params,
                                      vm, mmr_pa);                
            }
        } else {
            unsupported_type_exit(output_type);
        }
    } else if(problem == cp) { // regression based on combo program
        // get the combo_tree and infer its type
        combo_tree tr = str_to_combo_tree(combo_str);
        type_tree tt = infer_type_tree(tr);

        type_node output_type = *type_tree_output_type_tree(tt).begin();
        // if no exemplar has been provided in option use the default one
        if(exemplars.empty()) {
            exemplars.push_back(type_to_exemplar(output_type));
        }
        if(output_type == id::boolean_type) {
            // @todo: Occam's razor and nsamples is not taken into account
            logical_bscore bscore(tr, arity);
            metapop_moses_results(rng, exemplars, tt,
                                  logical_reduction(reduct_candidate_effort),
                                  logical_reduction(reduct_knob_building_effort),
                                  bscore, opt_algo,
                                  opt_params, meta_params, moses_params,
                                  vm, mmr_pa);                
        }
        else if (output_type == id::contin_type) {
            // @todo: introduce some noise optionally
            if(nsamples<=0)
                nsamples = default_nsamples;
            
            contin_input_table it(nsamples, arity, rng,
                                  max_rand_input, min_rand_input);
            contin_output_table table_outputs(tr, it, rng);
            
            int as = alphabet_size(tt, ignore_ops);
            
            occam_contin_bscore bscore(table_outputs, it,
                                       variance, as, rng);
            metapop_moses_results(rng, exemplars, tt,
                                  contin_reduction(ignore_ops, rng),
                                  contin_reduction(ignore_ops, rng),
                                  bscore, opt_algo,
                                  opt_params, meta_params, moses_params,
                                  vm, mmr_pa);
        } else {
            unsupported_type_exit(tt);
        }
    } else if(problem == pa) { // even parity
        even_parity func;

        // if no exemplar has been provided in option use the default
        // contin_type exemplar (and)
        if(exemplars.empty()) {
            exemplars.push_back(type_to_exemplar(id::boolean_type));
        }

        type_tree tt = declare_function(id::boolean_type, arity);
        logical_bscore bscore(func, arity);
        metapop_moses_results(rng, exemplars, tt,
                              logical_reduction(reduct_candidate_effort),
                              logical_reduction(reduct_knob_building_effort),
                              bscore, opt_algo,
                              opt_params, meta_params, moses_params,
                              vm, mmr_pa);
    } else if(problem == dj) { // disjunction
        // @todo: for the moment occam's razor and partial truth table are ignored
        disjunction func;

        // if no exemplar has been provided in option use the default
        // contin_type exemplar (and)
        if(exemplars.empty()) {            
            exemplars.push_back(type_to_exemplar(id::boolean_type));
        }

        type_tree tt = declare_function(id::boolean_type, arity);
        logical_bscore bscore(func, arity);
        metapop_moses_results(rng, exemplars, tt,
                              logical_reduction(reduct_candidate_effort),
                              logical_reduction(reduct_knob_building_effort),
                              bscore, opt_algo,
                              opt_params, meta_params, moses_params,
                              vm, mmr_pa);
    } else if(problem == mux) { // multiplex
        // @todo: for the moment occam's razor and partial truth table are ignored
        multiplex func(multiplex_arity(arity));

        // if no exemplar has been provided in option use the default
        // contin_type exemplar (and)
        if(exemplars.empty()) {            
            exemplars.push_back(type_to_exemplar(id::boolean_type));
        }

        type_tree tt = declare_function(id::boolean_type, arity);
        logical_bscore bscore(func, arity);
        metapop_moses_results(rng, exemplars, tt,
                              logical_reduction(reduct_candidate_effort),
                              logical_reduction(reduct_knob_building_effort),
                              bscore, opt_algo,
                              opt_params, meta_params, moses_params,
                              vm, mmr_pa);
    } else if(problem == sr) { // simple regression of f(x)_o = sum_{i={1,o}} x^i
        // if no exemplar has been provided in option use the default
        // contin_type exemplar (+)
        if(exemplars.empty()) {            
            exemplars.push_back(type_to_exemplar(id::contin_type));
        }
        
        type_tree tt = declare_function(id::contin_type, arity);

        contin_input_table rands((nsamples>0? nsamples : default_nsamples),
                                  arity, rng);

        int as = alphabet_size(tt, ignore_ops);

        occam_contin_bscore bscore(simple_symbolic_regression(problem_size),
                                   rands, variance, as, rng);
        metapop_moses_results(rng, exemplars, tt,
                              contin_reduction(ignore_ops, rng),
                              contin_reduction(ignore_ops, rng),
                              bscore, opt_algo,
                              opt_params, meta_params, moses_params,
                              vm, mmr_pa);
    //////////////////
    // ANN problems //
    //////////////////
    } else if(problem == ann_it) { // regression based on input table using ann
        unique_ptr<ifstream> in(open_data_file(input_data_file));
        contin_input_table it;
        contin_output_table ot;
        // read input_data_file file
        istreamTable(*in, it, ot, target_pos);
        // if no exemplar has been provided in option insert the default one
        if(exemplars.empty()) {
            exemplars.push_back(ann_exemplar(arity));
        }

        // subsample the table
        if(nsamples>0)
            subsampleTable(it, ot, nsamples, rng);

        type_tree tt = declare_function(id::ann_type, 0);
        
        int as = alphabet_size(tt, ignore_ops);

        occam_contin_bscore bscore(ot, it, variance, as, rng);
        metapop_moses_results(rng, exemplars, tt,
                              ann_reduction(),
                              ann_reduction(),
                              bscore, opt_algo,
                              opt_params, meta_params, moses_params,
                              vm, mmr_pa);
    } else if(problem == ann_cp) { // regression based on combo program using ann
        // get the combo_tree and infer its type
        combo_tree tr = str_to_combo_tree(combo_str);

        // if no exemplar has been provided in option use the default one
        if(exemplars.empty()) {
            exemplars.push_back(ann_exemplar(arity));
        }
        
        // @todo: introduce some noise optionally
        if(nsamples<=0)
            nsamples = default_nsamples;
        
        contin_input_table it(nsamples, arity, rng,
                              max_rand_input, min_rand_input);
        contin_output_table table_outputs(tr, it, rng);
        
        type_tree tt = declare_function(id::ann_type, 0);
        
        int as = alphabet_size(tt, ignore_ops);
        
        occam_contin_bscore bscore(table_outputs, it,
                                   variance, as, rng);
        metapop_moses_results(rng, exemplars, tt,
                              contin_reduction(ignore_ops, rng),
                              contin_reduction(ignore_ops, rng),
                              bscore, opt_algo,
                              opt_params, meta_params, moses_params,
                              vm, mmr_pa);
    }
    else unsupported_problem_exit(problem);
    return 0;
}

int moses_exec(const vector<string>& argvs) {
    char** argv = new char*[argvs.size()];
    for(size_t i = 0; i < argvs.size(); ++i) {
        argv[i] = const_cast<char*>(argvs[i].c_str());
    }
    int res = moses_exec(argvs.size(), argv);
    delete argv;
    return res;
}

} // ~namespace moses
} // ~namespace opencog

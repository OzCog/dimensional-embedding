/** moses_exec_def.h --- 
 *
 * Copyright (C) 2010 Novamente LLC
 *
 * Author: Nil Geisweiller
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


#ifndef _OPENCOG_MOSES_EXEC_DEF_H
#define _OPENCOG_MOSES_EXEC_DEF_H

#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

#include <opencog/util/iostreamContainer.h>

namespace opencog { namespace moses {

using namespace std;

// a map btw host name and number of jobs allocated
typedef map<string, unsigned> jobs_t;

// number of evals string
static const string number_of_evals_str = "n_evals";

// program option names and abbreviations
// for their meanings see options_description in moses-exec.cc
static const pair<string, string> rand_seed_opt("random-seed", "r");
static const pair<string, string> input_data_file_opt("input-file", "i");
static const pair<string, string> target_feature_opt("target-feature", "u");
static const pair<string, string> problem_opt("problem", "H");
static const pair<string, string> combo_str_opt("combo-program", "y");
static const pair<string, string> problem_size_opt("problem-size", "k");
static const pair<string, string> nsamples_opt("nsamples", "b");
static const pair<string, string> min_rand_input_opt("min-rand-input", "q");
static const pair<string, string> max_rand_input_opt("max-rand-input", "w");
static const pair<string, string> max_evals_opt("max-evals", "m");
static const pair<string, string> result_count_opt("result-count", "c");
static const pair<string, string> output_score_opt("output-score", "S");
static const pair<string, string> output_complexity_opt("output-complexity", "x");
static const pair<string, string> output_bscore_opt("output-bscore", "t");
static const pair<string, string> output_eval_number_opt("output-eval-number", "V");
static const pair<string, string> output_with_labels_opt("output-with-labels", "W");
static const pair<string, string> output_file_opt("output-file", "o");
static const pair<string, string> max_gens_opt("max-gens", "g");
static const pair<string, string> log_level_opt("log-level", "l");
static const pair<string, string> log_file_opt("log-file", "f");
static const string default_log_file_prefix = "moses";
static const string default_log_file_suffix = "log";
static const string default_log_file = default_log_file_prefix + "." + default_log_file_suffix;
static const pair<string, string> log_file_dep_opt_opt("log-file-dep-opt", "F");
static const pair<string, string> noise_opt("noise", "p");
static const pair<string, string> include_only_ops_str_opt("include-only-operator", "N");
static const pair<string, string> ignore_ops_str_opt("ignore-operator", "n");
static const pair<string, string> opt_algo_opt("opt-algo", "a");
static const pair<string, string> exemplars_str_opt("exemplar", "e");
static const pair<string, string> reduct_candidate_effort_opt("reduct-candidate-effort", "E");
static const pair<string, string> reduct_knob_building_effort_opt("reduct-knob-building-effort", "B");
static const pair<string, string> reduce_all_opt("reduce-all", "d");
static const pair<string, string> enable_cache_opt("enable-cache", "s");
static const pair<string, string> jobs_opt("jobs", "j");
static const string job_seperator(":");
static const string localhost("localhost");
static const pair<string, string> weighted_accuracy_opt("weighted-accuracy", "G");
static const pair<string, string> pop_size_ratio_opt("pop-size-ratio", "P");
static const pair<string, string> max_score_opt("max-score", "A");
static const pair<string, string> max_dist_ratio_opt("max-dist-ratio", "D");
static const pair<string, string> max_candidates_opt("max-candidates", "M");
static const pair<string, string> include_dominated_opt("include-dominated", "I");
static const pair<string, string> discretize_threshold_opt("discretize-threshold", "R");

// options specific to a particular fitness function
static const pair<string, string> ip_kld_weight_opt("ip_kld_weight", "K");
static const pair<string, string> ip_skewness_weight_opt("ip_skewness_weight", "J");
static const pair<string, string> ip_stdU_weight_opt("ip_stdU_weight", "U");
static const pair<string, string> ip_skew_U_weight_opt("ip_skew_U_weight", "X");
static const pair<string, string> ip_log_entropy_weight_opt("ip_log_entropy_weight", "O");

static const pair<string, string> it_alpha_opt("it_alpha", "Q");

// options specific to a particular optimization algorithm
static const pair<string, string> hc_terminate_if_improvement_opt("hc-terminate-if-improvement", "T");

string opt_desc_str(const pair<string, string>& opt);

} // ~namespace moses
} // ~namespace opencog

#endif // _OPENCOG_MOSES_OPTIONS_NAMES_H

/*
 * opencog/learning/PatternMiner/PatternMinerCentralServer.cc
 *
 * Copyright (C) 2012 by OpenCog Foundation
 * All Rights Reserved
 *
 * Written by Shujing Ke <rainkekekeke@gmail.com> in 2015
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

#include <math.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>
#include <sstream>
#include <thread>
#include <set>
#include <string>
#include <functional>


#include <opencog/atomspace/ClassServer.h>
#include <opencog/atomspace/Handle.h>
#include <opencog/atomspace/atom_types.h>
#include <opencog/spacetime/atom_types.h>
#include <opencog/embodiment/AtomSpaceExtensions/atom_types.h>
#include <opencog/query/BindLinkAPI.h>
#include <opencog/util/Config.h>
#include <opencog/util/StringManipulator.h>

#include "HTree.h"
#include "PatternMiner.h"

#include <cpprest/http_listener.h>
#include <cpprest/http_msg.h>
#include <cpprest/json.h>


using namespace opencog::PatternMining;
using namespace opencog;

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;


void PatternMiner::launchCentralServer()
{
    run_as_central_server = true;

    centralServerPort = config().get("PMCentralServerPort");

    serverListener = new http_listener( utility::string_t("http://localhost:" + centralServerPort +"/PatternMinerServer") );

    serverListener->support(methods::GET, std::bind(&PatternMiner::handleGet, this,  std::placeholders::_1));

    centralServerListeningThread = std::thread([this]{this->centralServerStartListening();});

    cout <<"\nPattern Miner central server started!\n";
    // centralServerListeningThread.join();

}

void PatternMiner::centralServerStartListening()
{
    try
    {
       serverListener->open()
          .then([](pplx::task<void> t){})
          .wait();

       while (true);
    }
    catch (exception const & e)
    {
       cout << e.what() << endl;
    }
}

void PatternMiner::handleGet(http_request request)
{
    cout << "Got request: \n" << request.to_string() << std::endl;
    string path = request.relative_uri().path();
    if (path == "/RegisterNewWorker")
    {
        handleRegisterNewWorker(request);
    }
    else
    {
        json::value answer = json::value::object();

        answer["msg"] = json::value("Unknown request command!");

        request.reply(status_codes::NotFound, answer);
    }


}


void PatternMiner::handleRegisterNewWorker(http_request request)
{

   cout << "Got request to RegisterNewWorker: \n" << request.to_string() << std::endl;
   json::value answer = json::value::object();

   answer["msg"] = json::value("Worker registered successfully!");

   request.reply(status_codes::OK, answer);

}

void PatternMiner::handleReportWorkerStop(http_request request)
{


}

void PatternMiner::handleFindANewPattern(http_request request)
{

}

void PatternMiner::centralServerEvaluateInterestingness()
{
//    if (enable_Frequent_Pattern)
//    {
//        std::cout<<"Debug: PatternMiner:  done frequent pattern mining for 1 to "<< MAX_GRAM <<"gram patterns!\n";

//        for(unsigned int gram = 1; gram <= MAX_GRAM; gram ++)
//        {
//            // sort by frequency
//            std::sort((patternsForGram[gram-1]).begin(), (patternsForGram[gram-1]).end(),compareHTreeNodeByFrequency );

//            // Finished mining gram patterns; output to file
//            std::cout<<"gram = " + toString(gram) + ": " + toString((patternsForGram[gram-1]).size()) + " patterns found! ";

//            OutPutFrequentPatternsToFile(gram);

//            std::cout<< std::endl;
//        }
//    }


//    if (enable_Interesting_Pattern)
//    {
//        for(cur_gram = 2; cur_gram <= MAX_GRAM - 1; cur_gram ++)
//        {
//            cout << "\nCalculating interestingness for " << cur_gram << " gram patterns by evaluating " << interestingness_Evaluation_method << std::endl;
//            cur_index = -1;
//            threads = new thread[THREAD_NUM];
//            num_of_patterns_without_superpattern_cur_gram = 0;

//            for (unsigned int i = 0; i < THREAD_NUM; ++ i)
//            {
//                threads[i] = std::thread([this]{this->evaluateInterestingnessTask();}); // using C++11 lambda-expression
//            }

//            for (unsigned int i = 0; i < THREAD_NUM; ++ i)
//            {
//                threads[i].join();
//            }

//            delete [] threads;

//            std::cout<<"Debug: PatternMiner:  done (gram = " + toString(cur_gram) + ") interestingness evaluation!" + toString((patternsForGram[cur_gram-1]).size()) + " patterns found! ";
//            std::cout<<"Outputting to file ... ";

//            if (interestingness_Evaluation_method == "Interaction_Information")
//            {
//                // sort by interaction information
//                std::sort((patternsForGram[cur_gram-1]).begin(), (patternsForGram[cur_gram-1]).end(),compareHTreeNodeByInteractionInformation);
//                OutPutInterestingPatternsToFile(patternsForGram[cur_gram-1], cur_gram);
//            }
//            else if (interestingness_Evaluation_method == "surprisingness")
//            {
//                // sort by surprisingness_I first
//                std::sort((patternsForGram[cur_gram-1]).begin(), (patternsForGram[cur_gram-1]).end(),compareHTreeNodeBySurprisingness_I);
//                OutPutInterestingPatternsToFile(patternsForGram[cur_gram-1], cur_gram,1);

//                vector<HTreeNode*> curGramPatterns = patternsForGram[cur_gram-1];

//                // and then sort by surprisingness_II
//                std::sort(curGramPatterns.begin(), curGramPatterns.end(),compareHTreeNodeBySurprisingness_II);
//                OutPutInterestingPatternsToFile(curGramPatterns,cur_gram,2);

//                // Get the min threshold of surprisingness_II
//                int threshold_index_II;
//                threshold_index_II = SURPRISINGNESS_II_TOP_THRESHOLD * (float)(curGramPatterns.size() - num_of_patterns_without_superpattern_cur_gram);
//                int looptimes = 0;
//                while (true)
//                {

//                    surprisingness_II_threshold = (curGramPatterns[threshold_index_II])->nII_Surprisingness;
//                    if (surprisingness_II_threshold <= 0.00000f)
//                    {
//                        if (++ looptimes > 8)
//                        {
//                            surprisingness_II_threshold = 0.00000f;
//                            break;
//                        }

//                        threshold_index_II = ((float)threshold_index_II) * SURPRISINGNESS_II_TOP_THRESHOLD;
//                    }
//                    else
//                        break;
//                }


//                cout<< "surprisingness_II_threshold for " << cur_gram << " gram = "<< surprisingness_II_threshold;

//                // go through the top N patterns of surprisingness_I, pick the patterns with surprisingness_II higher than threshold
//                int threshold_index_I = SURPRISINGNESS_I_TOP_THRESHOLD * (float)(curGramPatterns.size());
//                for (int p = 0; p <= threshold_index_I; p ++)
//                {
//                    HTreeNode* pNode = (patternsForGram[cur_gram-1])[p];

//                    // for patterns have no superpatterns, nII_Surprisingness == -1.0, which should be taken into account
//                    if ( (pNode->nII_Surprisingness < 0 ) || (pNode->nII_Surprisingness > surprisingness_II_threshold ) )
//                        finalPatternsForGram[cur_gram-1].push_back(pNode);
//                }

//                // sort by frequency
//                std::sort((finalPatternsForGram[cur_gram-1]).begin(), (finalPatternsForGram[cur_gram-1]).end(),compareHTreeNodeByFrequency );

//                OutPutFinalPatternsToFile(cur_gram);

//            }

//            std::cout<< std::endl;
//        }
//    }
}
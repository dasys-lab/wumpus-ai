#include "aspkb/Extractor.h"
#include "aspkb/IncrementalProblem.h"
#include "aspkb/TermManager.h"
#include <FileSystem.h>
#include <engine/AlicaClock.h>
#include <reasoner/asp/AnnotatedValVec.h>
#include <reasoner/asp/ExtensionQuery.h>
#include <reasoner/asp/IncrementalExtensionQuery.h>
#include <reasoner/asp/Term.h>
#include <reasoner/asp/Variable.h>
#include <wumpus/WumpusWorldModel.h>

#define EXTRACTOR_DEBUG

namespace aspkb
{
std::mutex Extractor::mtx;
std::mutex Extractor::incrementalMtx;
std::mutex Extractor::reusableMtx;
bool Extractor::wroteHeaderReusable = false;
bool Extractor::wroteHeaderIncremental = false;

Extractor::Extractor()
        : timesWritten(0)
{
    this->solver = TermManager::getInstance().getSolver();
    // TODO only for getSolution stats
    this->resultsDirectory = (*essentials::SystemConfig::getInstance())["WumpusEval"]->get<std::string>("TestRun.resultsDirectory", NULL);
    this->filenameIncremental = std::to_string(essentials::SystemConfig::getOwnRobotID()) + "_getSolutionStatsIncremental.csv";
    this->filenameReusable = std::to_string(essentials::SystemConfig::getOwnRobotID()) + "_getSolutionStatsReusable.csv";
}

Extractor::~Extractor()
{
    //    reasoner::asp::IncrementalExtensionQuery::clear();
}

// TODO this should be moved into the query itself as different incremental queries might require different solving strategies
std::vector<std::string> Extractor::solveWithIncrementalExtensionQuery(const std::shared_ptr<aspkb::IncrementalProblem>& inc, int overrideHorizon)
{
//    std::cout << "Extractor: SOlve with incremental extension query override horizon is " << overrideHorizon << std::endl;
    std::vector<std::string> ret;
    auto vars = std::vector<reasoner::asp::Variable*>();
    auto tmpVar = std::make_shared<::reasoner::asp::Variable>();
    vars.push_back(tmpVar.get());
    auto terms = std::vector<reasoner::asp::Term*>();
    std::vector<reasoner::asp::AnnotatedValVec*> results;
    bool sat = false;
    long timeElapsed = 0;
    int horizon = inc->startHorizon;
    if (!inc->keepBase) {
        inc->activateBase();
    }

    while (!sat && (overrideHorizon > 0 ? horizon <= overrideHorizon : horizon <= inc->maxHorizon)) {
        std::lock_guard<std::mutex> lock(TermManager::queryMtx);
        auto registeredQueries = this->solver->getRegisteredQueries(); // there are new queries added in each iteration
        terms.clear();

        //        if (inc->stepTermsByHorizon.find(horizon) != inc->stepTermsByHorizon.end()) {
        //            inc->activateStep(horizon);
        //        } else {
        //            auto stepTerm = inc->addStep(horizon);
        //            terms.push_back(stepTerm);
        //        }

        inc->activateStep(horizon);

        /*
        // experimenting with check term
        // todo make reusable
        //        auto checkTerm = TermManager::getInstance().requestCheckTerm(horizon);
        //        terms.push_back(checkTerm);

        //        if(horizon > 1) { //TODO lifetime? whose responsibility is it to reactivate queries?
        //            this->checkQueries.at(horizon-1)->removeExternal();
        //        }

        //         if(this->checkQueries.find(horizon) != this->checkQueries.end()) {
        //        auto checkTerm = TermManager::getInstance().requestCheckTerm(horizon);
        //        terms.push_back(checkTerm);
         */

        if (horizon > 1) {
            inc->deactivateCheck(horizon - 1);
            //#ifdef EXTRACTOR_DEBUG
            //            for (const auto& i : inc->inquiryPredicates) {
            //                if (i.find("Complete") != std::string::npos) {
            //                    std::cout << std::endl;
            ////                    std::cout << "EXTRACTOR: CALLING GETSOLUTION WITH ACTIVE STEP FOR HORIZON " << horizon << std::endl;
            //                    this->solver->getSolution(vars, terms, results);
            //                    std::cout << std::endl;
            //                    break;
            //                }
            //            }

            //#endif
            //            for (const auto& query : registeredQueries) {
            //                if (query->getTerm()->getQueryId() ==
            //                        this->checkTerms.at(horizon - 1)->getQueryId()) { // TODO lifetime? whose responsibility is it to reactivate queries?
            //                    //                this->checkQueries.at(horizon - 1)->removeExternal();
            //                    query->removeExternal();
            //                    break;
            //                }
            //            }
        }
        //        std::cout << "Extractor: activating check term for horizon " << horizon << std::endl;
        inc->activateCheckTerm(horizon);
        //        if (this->checkTerms.find(horizon) != this->checkTerms.end()) {
        //            for (const auto& query : registeredQueries) {
        //                if (query->getTerm()->getQueryId() == this->checkTerms.at(horizon)->getQueryId()) {
        //                    std::dynamic_pointer_cast<::reasoner::asp::ReusableExtensionQuery>(query)->reactivate();
        //                    break;
        //                }
        //            }
        //        } else {
        //            auto checkTerm = TermManager::getInstance().requestCheckTerm(horizon);
        //            terms.push_back(checkTerm);
        //            //            auto checkQuery = std::make_shared<reasoner::asp::ReusableExtensionQuery>(this->solver, checkTerm);
        //            //            this->solver->registerQuery(checkQuery);
        //            //            this->checkQueries.emplace(horizon, checkQuery);
        //            this->checkTerms.emplace(horizon, checkTerm);
        //        }

        // Create Term belonging to a FilterQuery to be part of the terms in the getSolution call
        for (auto inquiry : inc->inquiryPredicates) {
            for (int j = 0; j <= horizon; ++j) {
                if (j == 0) {
                    bool found = false;
                    for (const auto& i : inc->inquiryPredicates) {
                        if (i.find("Complete") != std::string::npos) {
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        continue;
                    }
                } // FIXME hack - review
                auto term = inc->activateInquiryTerm(inquiry, j);
                // this should collect results - validate!
                terms.push_back(term);
                //                inquiryTerm->setLifeTime(1);
                //                inquiryTerm->setType(::reasoner::asp::QueryType::Filter);
                //                // wrap query rule to match extension query
                //                auto wrappedQueryRule = std::string("incquery") + std::to_string(j) + "(" + inquiry + ")";
                //                //                std::cout << "adding inquiry term" << std::endl;
                //                inquiryTerm->setQueryRule(wrappedQueryRule);
                //                //                std::cout << "inquiry - 2 " << std::endl;
                //                terms.push_back(inquiryTerm);
            }
        }

        // add terms to terms passed in getSolution
        auto timeBefore = wumpus::WumpusWorldModel::getInstance()->getEngine()->getAlicaClock()->now().inMilliseconds();
        sat = this->solver->getSolution(vars, terms, results);
        auto solveTime = wumpus::WumpusWorldModel::getInstance()->getEngine()->getAlicaClock()->now().inMilliseconds() - timeBefore;
        timeElapsed = timeElapsed + solveTime;

        this->writeGetSolutionStatsReusable("pathActionsIncQuery" + std::to_string(horizon), solveTime, horizon);

        //        std::cout << "PROBLEM IS " << (sat ? "SATISFIABLE" : "NOT SATISFIABLE") << std::endl; // inquiryTerm->getQueryRule() << std::endl;
        ++horizon;
    }

    // FIXME loop in reverse
    std::lock_guard<std::mutex> lock(TermManager::queryMtx);
    //    auto registeredQueries = this->solver->getRegisteredQueries(); // there are new queries added in each iteration
    //    for (const auto& term : this->checkTerms) {
    //        for (const auto& query : registeredQueries) {
    //            if (query->getTerm()->getQueryId() == term.second->getQueryId()) {
    //                query->removeExternal();
    //                break;
    //            }
    //        }
    //    }

    inc->falsifyAllStepTerms();
    inc->deactivateAllChecks();

    if (!inc->keepBase) {
        inc->deactivateBase();
    }

    /*
    // new run should start without active step queries
    //        if(sat) {
    //            for(auto query : this->checkQueries) {
    //                query.second->removeExternal();
    //            }
    //        }
    */

    // TODO something is buggy here (way too many result entries)
    for (auto res : results) {
        for (auto& factQueryValue : res->factQueryValues) {
            for (auto elem : factQueryValue) {
                if (std::find(ret.begin(), ret.end(), elem) == ret.end()) {
                    ret.push_back(elem);
                } else {
                }
            }
        }
        delete res;
    }
    return ret;
}
/*
 * TODO what to do about reusable query (lifetime etc.)
 * TODO time measurements
 */
std::vector<std::string> Extractor::extractReusableTemporaryQueryResult(
        const std::vector<std::string>& inquiryPredicates, const std::string& queryIdentifier, const std::vector<std::string>& additionalRules)
{
    std::vector<std::string> ret;
    auto vars = std::vector<reasoner::asp::Variable*>();
    auto tmpVar = std::make_shared<::reasoner::asp::Variable>();
    vars.push_back(tmpVar.get());
    auto terms = std::vector<reasoner::asp::Term*>();
    std::vector<reasoner::asp::AnnotatedValVec*> results;
    long timeElapsed = 0;

    int id = TermManager::getInstance().activateReusableExtensionQuery(queryIdentifier, additionalRules);

    // TODO make reusable as well
    //    if (alreadyHandledQueries.find(id) == alreadyHandledQueries.end()) {
    for (auto inquiry : inquiryPredicates) {
        auto inquiryTerm = TermManager::getInstance().requestTerm();
        inquiryTerm->setLifeTime(1);
        inquiryTerm->setType(::reasoner::asp::QueryType::Filter);
        // wrap query rule to match extension query
        auto wrappedQueryRule = std::string("query") + std::to_string(id) + "(" + inquiry + ")";
        inquiryTerm->setQueryRule(wrappedQueryRule);
        terms.push_back(inquiryTerm);
        alreadyHandledQueries.insert(id);
    }
    //    }

    auto timeBefore = wumpus::WumpusWorldModel::getInstance()->getEngine()->getAlicaClock()->now().inMilliseconds();
    auto sat = this->solver->getSolution(vars, terms, results);
    auto solveTime = wumpus::WumpusWorldModel::getInstance()->getEngine()->getAlicaClock()->now().inMilliseconds() - timeBefore;
    timeElapsed = timeElapsed + solveTime;
    this->writeGetSolutionStatsReusable(queryIdentifier, timeElapsed);
    //    std::cout << "PROBLEM IS " << (sat ? "SATISFIABLE" : "NOT SATISFIABLE") << ", " << std::endl; // inquiryTerm->getQueryRule() << std::endl;
    for (auto res : results) {
        for (auto& varQueryValue : res->variableQueryValues) {
            for (auto elem : varQueryValue) {
                ret.push_back(elem);
            }
        }
        delete res;
    }

    TermManager::getInstance().deactivateReusableExtensionQuery(queryIdentifier);

    return ret;
}

void Extractor::writeGetSolutionStatsIncremental(int horizon, long timeElapsed)
{
    //    if(this->timesWritten >= 5) {
    //        return;
    //    }
//    std::lock_guard<std::mutex> lock(Extractor::incrementalMtx);
//    std::ofstream fileWriter;
//    std::string separator = ";";
//    if (!aspkb::Extractor::wroteHeaderIncremental) {
//        aspkb::Extractor::wroteHeaderIncremental = true;
//        this->writeHeader(true);
//    }
//
//    auto folder = essentials::FileSystem::combinePaths(getenv("HOME"), this->resultsDirectory);
//    fileWriter.open(essentials::FileSystem::combinePaths(folder, this->filenameIncremental), std::ios_base::app);
//    fileWriter << std::to_string(horizon);
//    fileWriter << separator;
//    fileWriter << std::to_string(timeElapsed);
//    fileWriter << std::endl;
//    //    this->timesWritten++;
//    fileWriter.close();
}

void Extractor::writeGetSolutionStatsReusable(const std::string& queryIdentifier, long timeElapsed, int horizon)
{
    if (this->timesWritten >= 5) {
        return;
    }
//    std::lock_guard<std::mutex> lock(Extractor::reusableMtx);
//    std::ofstream fileWriter;
//    std::string separator = ";";
//
//    if (!aspkb::Extractor::wroteHeaderReusable) {
//        aspkb::Extractor::wroteHeaderReusable = true;
//        this->writeHeader(false);
//    }
//
//    auto folder = essentials::FileSystem::combinePaths(getenv("HOME"), this->resultsDirectory);
//    fileWriter.open(essentials::FileSystem::combinePaths(folder, this->filenameReusable), std::ios_base::app);
//
//    fileWriter << queryIdentifier;
//    fileWriter << separator;
//    fileWriter << std::to_string(timeElapsed);
//    fileWriter << separator;
//    if (horizon != -1) {
//        fileWriter << horizon;
//    }
//    fileWriter << std::endl;
//    fileWriter.close();
}

inline void Extractor::writeHeader(bool incremental)
{

    std::ofstream fileWriter;
    std::string separator = ";";
    std::string fileName;
    //    if (incremental) {
    //        fileName = this->filenameIncremental;
    //    } else {
    fileName = this->filenameReusable;
    //    }

    auto folder = essentials::FileSystem::combinePaths(getenv("HOME"), this->resultsDirectory);
    fileWriter.open(essentials::FileSystem::combinePaths(folder, fileName), std::ios_base::app);

    fileWriter << "QueryIdentifier";
    fileWriter << separator;
    fileWriter << "TimeElapsed";
    fileWriter << separator;
    fileWriter << "Horizon";
    fileWriter << std::endl;
    fileWriter.close();
}

} /* namespace aspkb */
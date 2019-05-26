#include "wumpus/wm/PlanningModule.h"
#include "wumpus/WumpusWorldModel.h"
#include <asp_solver_wrapper/ASPSolverWrapper.h>
#include <aspkb/Extractor.h>
#include <aspkb/Integrator.h>
#include <aspkb/Strategy.h>
#include <engine/AlicaEngine.h>
#include <iostream>
#include <sstream>
#include <wumpus/model/Agent.h>

namespace wumpus
{
namespace wm
{

PlanningModule::PlanningModule(wumpus::WumpusWorldModel* wm)
        : wm(wm)
{
    this->extractor = new aspkb::Extractor();

    // TODO reduce code
    this->sc = essentials::SystemConfig::getInstance();
    auto filePath = (*sc)["KnowledgeManager"]->get<std::string>("goalGenerationRulesFilePath", NULL);
    this->loadAdditionalRules(filePath, this->goalGenerationRules);
    filePath = (*sc)["KnowledgeManager"]->get<std::string>("actionsGenerationRulesFilePath", NULL);
    this->loadAdditionalRules(filePath, this->actionsGenerationRules);
    filePath = (*sc)["KnowledgeManager"]->get<std::string>("actionsGenWumpusFilePath", NULL);
    this->loadAdditionalRules(filePath, this->actionsGenWumpusRules);
    filePath = (*sc)["KnowledgeManager"]->get<std::string>("pathValidationRulesFilePath", NULL);
    this->loadAdditionalRules(filePath, this->pathValidationRules);
    filePath = (*sc)["KnowledgeManager"]->get<std::string>("objectiveRulesFilePath", NULL);
    this->loadAdditionalRules(filePath, this->objectiveRules);
    filePath = (*sc)["KnowledgeManager"]->get<std::string>("shootFromRulesFilePath", NULL);
    this->loadAdditionalRules(filePath, this->shootFromRules);
    filePath = (*sc)["KnowledgeManager"]->get<std::string>("stepRulesFilePath", NULL);
    this->loadAdditionalRules(filePath, this->stepRules);
    filePath = (*sc)["KnowledgeManager"]->get<std::string>("baseRulesFilePath", NULL);
    this->loadAdditionalRules(filePath, this->baseRules);
    filePath = (*sc)["KnowledgeManager"]->get<std::string>("checkRulesFilePath", NULL);
    this->loadAdditionalRules(filePath, this->checkRules);
}

PlanningModule::~PlanningModule()
{
    delete this->extractor;
}

/**
 *
 * @param agent
 * @return
 */
std::pair<int, std::shared_ptr<std::vector<WumpusEnums::actions>>> PlanningModule::processNextActionRequest(std::shared_ptr<wumpus::model::Agent> agent)
{

    // TODO FIXME new logic broke this, quick fixed

    this->wm->wumpusSimData.setIntegratedFromSimulator(false);

    auto actions = std::make_shared<std::vector<WumpusEnums::actions>>();
    auto params = std::vector<std::string>();
    std::stringstream ss;

    std::pair<std::vector<std::pair<std::string, std::string>>, std::vector<std::string>> fieldsActionsPair;
    std::string goal;

    // determine objective
    this->determineObjective();
    // determine actions

    if (agent->objective == wumpus::model::Objective::EXPLORE) {

        fieldsActionsPair = this->determinePathAndActions({}, -1);

        // Incremental approach couldn't find a solution - try to shoot wumpus or allow unsafe moves
        if (fieldsActionsPair.second.empty()) {

            if (this->wm->playground->getAgentById(essentials::SystemConfig::getOwnRobotID())->hasArrow) {

                if (this->determineWumpusBlocksSafeMoves()) {


                    this->wm->changeHandler->integrator->integrateInformationAsExternal(
                            "wumpusBlocksMoves", "wumpusMoves", aspkb::Strategy::FALSIFY_OLD_VALUES);
                    this->determineObjective();
                    this->determinePosToShootFrom();
                    fieldsActionsPair = this->determinePathAndActions({}, -1);
                } else { // no blocking wumpi found
                    this->wm->changeHandler->integrator->integrateInformationAsExternal(
                            "unsafeMovesAllowed", "unsafeMoves", aspkb::Strategy::FALSIFY_OLD_VALUES);
                    fieldsActionsPair = this->determinePathAndActions({}, -1);
                }
            } else {
                // TODO needs to be resetted after actions
                this->wm->changeHandler->integrator->integrateInformationAsExternal("unsafeMovesAllowed", "unsafeMoves", aspkb::Strategy::FALSIFY_OLD_VALUES);
                fieldsActionsPair = this->determinePathAndActions({}, -1);
            }
        }

    } else if (agent->objective == wumpus::model::Objective::COLLECT_GOLD || agent->objective == wumpus::model::Objective::SHOOT ||
               agent->objective == wumpus::model::Objective::LEAVE) {
        //        std::cout << "DETERMINE ACTIONS NO MOVEMENT) " << std::endl;
        fieldsActionsPair.second = this->determineActionsNoMovement();
    } else {

        if (this->lastPathAndActions.second.size() > 0) {
            // minus first action (should have been performed)
            //            std::cout << "REMOVING ACTION" << std::endl;
            this->lastPathAndActions.second.erase(this->lastPathAndActions.second.begin());
        }

        if (agent->replanNecessary) {
            //            std::cout << "***********************CHOOSING NEW GOAL AND ACTIONS" << std::endl;
            // choose a goal field and register it as external
            goal = this->determineGoal();

            // plan movement to goal, interesting aspects are the fields to be visited and the actual action sequence
            fieldsActionsPair = this->determinePathAndActions(this->actionsGenerationRules, -1);
        } else {
            //            std::cout << "**************************REUSING GOAL AND ACTIONS" << std::endl;

            fieldsActionsPair = this->lastPathAndActions;
            goal = this->lastGoal;
        }
    }

    this->determineObjective();
    if (agent->objective == wumpus::model::Objective::COLLECT_GOLD || agent->objective == wumpus::model::Objective::SHOOT ||
            agent->objective == wumpus::model::Objective::LEAVE) {
        //        std::cout << "DETERMINE ACTIONS NO MOVEMENT) " << std::endl;
        fieldsActionsPair.second = this->determineActionsNoMovement();
    }

    this->lastPathAndActions = fieldsActionsPair;
    this->lastGoal = goal;
    auto result = fieldsActionsPair.second;
    WumpusEnums::actions actionArray[result.size()];
    for (int i = 0; i < result.size(); ++i) {
        auto tmp = result.at(i);
        auto index = i;

        if (tmp.find("move") != std::string::npos) {
            // std::cout << "MOVE!" << std::endl;
            actionArray[index] = WumpusEnums::move;
        } else if (tmp.find("turnRight") != std::string::npos) {
            // std::cout << "RIGHT!" << std::endl;
            actionArray[index] = WumpusEnums::turnRight;
        } else if (tmp.find("turnLeft") != std::string::npos) {
            // std::cout << "LEFT!" << std::endl;
            actionArray[index] = WumpusEnums::turnLeft;
        } else if (tmp.find("shoot") != std::string::npos) {
            std::cout << "SHOOT!" << std::endl;
            actionArray[index] = WumpusEnums::shoot;
            // reset goal and goalHeading

            this->wm->changeHandler->integrator->integrateInformationAsExternal("", "goal", aspkb::Strategy::FALSIFY_OLD_VALUES);
            this->wm->changeHandler->integrator->integrateInformationAsExternal("", "goalHeading", aspkb::Strategy::FALSIFY_OLD_VALUES);
        } else if (tmp.find("pickup") != std::string::npos) {
            std::cout << "PICKUP!" << std::endl;
            actionArray[index] = WumpusEnums::pickUpGold;
        } else if (tmp.find("leave") != std::string::npos) {
            std::cout << "LEAVE!" << std::endl;
            actionArray[index] = WumpusEnums::leave;
        } else {
            std::cout << "KM: UNKNOWN ACTION! " << tmp << std::endl;
            // TODO REMOVE
            actionArray[0] = WumpusEnums::move;
        }
    }
    for (int i = 0; i < result.size(); ++i) {
        actions->push_back(actionArray[i]);
    }

    // this->wm->changeHandler->integrator->integrateInformationAsExternal("", "blacklist", aspkb::Strategy::FALSIFY_OLD_VALUES);

    agent->replanNecessary = false;
    // fixme
    return std::make_pair(0, actions);
}

/**
 *
 * @return String representation of goal predicate
 */
std::string PlanningModule::determineGoal()
{

    std::string goalQueryHeadValue = "suggestedGoal(wildcard,wildcard)";

    auto paramsPair = std::make_pair<std::string, std::string>("", "");
    auto params = std::vector<std::string>();

    // auto result = this->extractor->extractTemporaryQueryResult({goalQueryHeadValue}, this->goalGenerationRules, paramsPair);
    auto result = this->extractor->extractReusableTemporaryQueryResult({goalQueryHeadValue}, "goal", this->goalGenerationRules);

    if (!result.empty()) {
        if (result.size() > 1) {
            std::cerr << "More than one suggested goal found. Using first entry. " << std::endl;
        }
        auto ext = this->extractBinaryPredicateParameters(result.at(0), "suggestedGoal");
        std::stringstream goalRep;
        goalRep << "goal(" << ext.first << ',' << ext.second << ')';
        auto goal = goalRep.str();

        this->wm->changeHandler->integrator->integrateInformationAsExternal(goal, "goal", aspkb::Strategy::FALSIFY_OLD_VALUES);

        return goal;
    }
    return "";
}

// TODO
/**
 * Extracts the current objective and registers it as an external along with agent Id
 * @return
 */
wumpus::model::Objective PlanningModule::determineObjective()
{

    // auto sol = this->extractor->extractTemporaryQueryResult({"objective(wildcard)"}, objectiveRules, {});
    auto sol = this->extractor->extractReusableTemporaryQueryResult({"objective(wildcard)"}, "objective", objectiveRules);
    auto result = sol.at(0);

    wumpus::model::Objective obj = wumpus::model::Objective::UNDEFINED;

    if (result.find("explore") != std::string::npos) {
        obj = wumpus::model::Objective::EXPLORE;
    } else if (result.find("huntWumpus") != std::string::npos) {
        obj = wumpus::model::Objective::HUNT_WUMPUS;
    } else if (result.find("collectGold") != std::string::npos) {
        obj = wumpus::model::Objective::COLLECT_GOLD;
    } else if (result.find("goHome") != std::string::npos) {
        obj = wumpus::model::Objective::GO_HOME;
    } else if (result.find("shoot") != std::string::npos) {
        std::cout << "objective shoot" << std::endl;
        obj = wumpus::model::Objective::SHOOT;
    } else if (result.find("leave") != std::string::npos) {
        obj = wumpus::model::Objective::LEAVE;
    } else if (result.find("moveToGoldField") != std::string::npos) {
        obj = wumpus::model::Objective::MOVE_TO_GOLD_FIELD;
    }

    this->wm->playground->getAgentById(essentials::SystemConfig::getInstance()->getOwnRobotID())->updateObjective(obj);

    return obj;
    // return wumpus::model::Objective::EXPLORE;
}

/**
 * Determine Path and Actions (path is currently not used but might be interesting to know for future logging)
 * @param generationRules
 * @param horizon
 * @return pair containing field coordinates of path and actions
 */
std::pair<std::vector<std::pair<std::string, std::string>>, std::vector<std::string>> PlanningModule::determinePathAndActions(
        const std::vector<std::string>& generationRules, int horizon)
{
    // TODO read maxHorizon factor from conf
    auto maxHorizon = 2 * this->wm->playground->getPlaygroundSize();

    std::string pathQueryValue = "holds(on(wildcard,wildcard), wildcard)";
    std::string actionsQueryValue = "occurs(wildcard,wildcard)";
    std::string pathPredicate = "on";
    std::string actionsPredicate = "occurs";

    auto result = this->extractor->solveWithIncrementalExtensionQuery(
            {pathQueryValue, actionsQueryValue}, this->baseRules, this->stepRules, this->checkRules, maxHorizon);

    auto path = std::vector<std::pair<std::string, std::string>>();
    auto actions = std::vector<std::string>();

    if (!result.empty()) {

        for (const auto& elem : result) {
            auto pathResult = this->extractBinaryPredicateParameters(elem, "on");
            if (pathResult.first != "" && pathResult.second != "") {
                path.push_back(std::make_pair(pathResult.first, pathResult.second));
            }
            auto actionsResult = this->extractBinaryPredicateParameters(elem, "occurs");
            if (actionsResult.first != "" && actionsResult.second != "") {
                if (std::stoi(actionsResult.second) + 1 > actions.size()) {
                    actions.resize(std::stoi(actionsResult.second) + 1);
                }
                actions[std::stoi(actionsResult.second)] = actionsResult.first;
            }
        }
    }

    return std::make_pair(path, actions);
}

/**
 * the feedback of this has been moved into the knowledge base
 * @return
 */
bool PlanningModule::determineWumpusBlocksSafeMoves()
{
    /*
    auto paramsPair = std::make_pair<std::string, std::string>("", "");
    // std::lock_guard<std::mutex> lock(this->queryMtx);
    auto ret = this->extractor->extractTemporaryQueryResult(
            {("wumpusBlocksSafeMoves")}, {"wumpusBlocksSafeMoves :- wumpus(X,Y),fieldAdjacent(X,Y,A,B), not visited(A,B)"}, paramsPair);
    if (ret.size() == 0) {
        return false;
    }
    return true;
     */
    return true;
}

void PlanningModule::determinePosToShootFrom()
{
    auto position = std::pair<std::string, std::string>();
    auto heading = std::string();
    auto shotAt = std::vector<std::pair<std::string, std::string>>();
    // auto result = this->extractor->extractTemporaryQueryResult(
    //      {"targetPos(wildcard,wildcard)", "targetHeading(wildcard)", "fieldIsAhead(wildcard,wildcard)"}, this->shootFromRules, paramsPair);
    auto result = this->extractor->extractReusableTemporaryQueryResult(
            {"targetPos(wildcard,wildcard)", "targetHeading(wildcard)", "fieldIsAhead(wildcard,wildcard)"}, "pos4Wumpus", this->shootFromRules);
    if (!result.empty()) {

        for (const auto& res : result) {
            if (res.find("targetPos") != std::string::npos) {
                position = this->extractBinaryPredicateParameters(res, "targetPos");
                std::cout << "position is " << position.first << ", " << position.second << std::endl;
            } else if (res.find("targetHeading") != std::string::npos) {
                heading = this->extractUnaryPredicateParameter(res);
                std::cout << "target heading is " << heading << std::endl;
            } else if (res.find("fieldIsAhead") != std::string::npos) {
                auto shotAtResult = this->extractBinaryPredicateParameters(res, "fieldIsAhead");
                shotAt.push_back(std::make_pair(shotAtResult.first, shotAtResult.second));
            }
        }

        std::stringstream ss;
        ss << "goal(" << position.first << ", " << position.second << ")";
        this->wm->changeHandler->integrator->integrateInformationAsExternal(ss.str(), "goal", aspkb::Strategy::FALSIFY_OLD_VALUES);
        ss.str("");
        ss << "goalHeading(" << heading << ")";
        this->wm->changeHandler->integrator->integrateInformationAsExternal(ss.str(), "goalHeading", aspkb::Strategy::FALSIFY_OLD_VALUES);
        ss.str("");
        for (const auto& field : shotAt) {
            ss << "shotAt(" << field.first << "," << field.second << ")";
            this->wm->changeHandler->integrator->integrateInformationAsExternal(ss.str(), "shotAt", aspkb::Strategy::INSERT_TRUE);
            ss.str("");
        }
        // reset wumpus blocks safe moves
        this->wm->changeHandler->integrator->integrateInformationAsExternal("", "wumpusMoves", aspkb::Strategy::FALSIFY_OLD_VALUES);
    } else {
        // std::cout << "NO SOLUTION" << std::endl;
    }
}

/**
 * Activates a query with simple rules for getting the next action when no movement is required
 * @return
 */
std::vector<std::string> PlanningModule::determineActionsNoMovement()
{
    auto result = std::vector<std::string>();
    auto inquiry = "occurs(wildcard)";
    std::vector<std::string> rules;
    auto rule = "occurs(pickup) :- on(X,Y), glitter(X,Y)";
    rules.push_back(rule);
    rule = "occurs(shoot) :- on(X,Y), goal(X,Y), heading(H), goalHeading(H)";
    rules.push_back(rule);
    rule = "occurs(leave) :- on(X,Y), haveGold(A), me(A), initial(X,Y)";
    rules.push_back(rule);

    result = this->extractor->extractReusableTemporaryQueryResult({inquiry}, "simpleAction", rules);
    return result;
}

/**
 * Helper method to extract the (not nested) parameters from a binary predicate
 * @param str here: result string from a query
 * @param predName name of queried predicate
 * @return vector containing the two predicates or empty vector if predName wasn't found
 */
std::pair<std::string, std::string> PlanningModule::extractBinaryPredicateParameters(const std::string& str, const std::string& predName)
{
    std::string x;
    std::string y;
    auto start = str.find(predName);
    if (start == std::string::npos) {
        return std::pair<std::string, std::string>();
    }
    auto sep = str.find(',', start);
    if (sep == std::string::npos) {
        return std::pair<std::string, std::string>();
    }

    auto end = str.find(')');
    x = str.substr(start + predName.length() + 1, sep - start - predName.length() - 1);
    y = str.substr(sep + 1, end - sep - 1);
    return std::pair<std::string, std::string>{x, y};
}

/**
 *Helper method to get a simple unary parameter
 */
std::string PlanningModule::extractUnaryPredicateParameter(const std::string& str)
{
    auto braceStart = str.find('(');
    auto braceEnd = str.find(')');
    return str.substr(braceStart + 1, braceEnd - braceStart - 1);
}
/**
 * Load additional rules to be added to terms in the form of (Extension-)Queries. These files must only contiain the rules, no comments etc.
 * @param filePath (relative to the domain config folder of the workspace)
 * @param ruleContainer
 */
void PlanningModule::loadAdditionalRules(const std::string& filePath, std::vector<std::string>& ruleContainer)
{
    auto path2etc = std::getenv("DOMAIN_CONFIG_FOLDER");
    std::ifstream input(std::string(path2etc) + '/' + filePath);
    std::string line;
    while (std::getline(input, line)) {
        ruleContainer.push_back(line);
    }
}
} /* namespace wm */
} /* namespace wumpus*/
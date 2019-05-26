#include "wumpus/model/Playground.h"
#include "wumpus/model/Agent.h"
#include "wumpus/model/Field.h"
#include <iostream>
namespace wumpus
{
namespace model
{

Playground::Playground(wumpus::wm::ChangeHandler* ch)
        : DomainElement(ch)
{
    this->playgroundSize = -1;
    this->turnCounter = 0;
}

Playground::~Playground() = default;

void Playground::addAgent(std::shared_ptr<wumpus::model::Agent> agent)
{
    std::lock_guard<std::mutex> lock(this->agentMtx);
    this->agents[agent->id] = agent;
}
std::shared_ptr<wumpus::model::Agent> Playground::getAgentById(int id)
{
    std::lock_guard<std::mutex> lock(this->agentMtx);
    auto it = this->agents.find(id);
    if (it != this->agents.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<wumpus::model::Field> Playground::getField(int x, int y)
{
    std::lock_guard<std::mutex> lock(this->fieldMtx);
    auto it = this->fields.find(std::make_pair(x, y));

    if (it != this->fields.end()) {
        return it->second;
    }

    // playground not initialized yet
    auto field = std::make_shared<wumpus::model::Field>(this->ch, x, y);
    this->fields[std::make_pair(x, y)] = field;

    return field;
}

/**
 * Sets field size and creates fields
 */
void Playground::initializePlayground(int size)
{
    std::lock_guard<std::mutex> lock(this->fieldMtx);
    this->playgroundSize = size;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            auto coordinates = std::make_pair(i, j);
            // check if field already exists
            if (this->fields.find(coordinates) == this->fields.end()) {
                auto field = std::make_shared<wumpus::model::Field>(this->ch, i, j);
                this->fields[coordinates] = field;
            }
        }
    }
    this->ch->handleSetFieldSize(size);
}

void Playground::handleSilence() {
    this->ch->handleSilence();

}

void Playground::handleScream() {
    this->ch->handleScream();
}


int Playground::getPlaygroundSize()
{
    return this->playgroundSize;
}

int Playground::getNumberOfFields()
{
    return this->playgroundSize * this->playgroundSize;
}

int Playground::getNumberOfAgents() {
    return this->agents.size();
}
} /* namespace model */
} /* namespace wumpus */
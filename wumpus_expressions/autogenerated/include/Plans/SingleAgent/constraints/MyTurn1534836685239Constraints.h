#ifndef MyTurnCONSTRAINT_H_
#define MyTurn_H_
#include "engine/BasicConstraint.h"
#include <memory>

using namespace std;
using namespace alica;

namespace alica
{
    class ProblemDescriptor;
    class RunningPlan;
}

namespace alicaAutogenerated
{

    class Constraint1534925376501 : public BasicConstraint
    {
        void getConstraint(shared_ptr<ProblemDescriptor> c, shared_ptr<RunningPlan> rp);
    };

} /* namespace alica */

#endif /* MyTurnCONSTRAINT_H_ */

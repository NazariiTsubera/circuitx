//
// Created by Nazarii on 11/4/25.
//

#ifndef STATESERVICE_H
#define STATESERVICE_H


enum State {
    Play,
    Pause,
    Edit
};

class StateService {
private:
    State currentState;
public:
    StateService(State initial = State::Edit)
        : currentState(initial)
    {}

    void setCurrentState(State state) {
        currentState = state;
    }

    State getCurrentState() { return currentState; }
};



#endif //STATESERVICE_H

//
// Created by Nazarii on 11/4/25.
//

#ifndef STATESERVICE_H
#define STATESERVICE_H


enum State {
    Play,
    Pause,
    Edit,
    Settings
};


class StateService {
private:
    State currentState;

    std::vector<std::function<void(State, State)>> callbacks;
public:
    StateService(State initial = State::Edit)
        : currentState(initial)
    {}

    void setCurrentState(State state) {
        State oldState = currentState;
        currentState = state;
        for (auto callback : callbacks) {
            callback(oldState, currentState);
        }
    }

    State getCurrentState() { return currentState; }

    void addCallback(std::function<void(State, State)> callback) {
        callbacks.push_back(callback);
    }
};



#endif //STATESERVICE_H

#include <tuple>
#include <variant>
#include <iostream>
#include <utility>

template <typename... Handlers>
struct Will : Handlers...
{
    using Handlers::handle...;
};

struct Nothing
{
    template <typename Machine, typename State, typename Event>
    void execute(Machine &, State &, const Event &)
    {
    }
};

template <typename Action>
struct ByDefault
{
    template <typename Event>
    Action handle(const Event &) const
    {
        return Action{};
    }
};

template <typename... Actions>
class OneOf
{
public:
    template <typename T>
    OneOf(T &&arg)
        : options(std::forward<T>(arg))
    {
    }

    template <typename Machine, typename State, typename Event>
    void execute(Machine &machine, State &state, const Event &event)
    {
        std::visit([&machine, &state, &event](auto &action)
                   { action.execute(machine, state, event); },
                   options);
    }

private:
    std::variant<Actions...> options;
};

template <typename Action>
struct Maybe : public OneOf<Action, Nothing>
{
    using OneOf<Action, Nothing>::OneOf;
};

template <typename Event, typename Action>
struct On
{
    Action handle(const Event &) const
    {
        return Action{};
    }
};

template <typename TargetState>
class TransitionTo
{
public:
    template <typename Machine, typename State, typename Event>
    void execute(Machine &machine, State &prevState, const Event &event)
    {
        leave(prevState, event);
        TargetState &newState = machine.template transitionTo<TargetState>();
        enter(newState, event);
    }

private:
    void leave(...)
    {
    }

    template <typename State, typename Event>
    auto leave(State &state, const Event &event) -> decltype(state.onLeave(event))
    {
        return state.onLeave(event);
    }

    void enter(...)
    {
    }

    template <typename State, typename Event>
    auto enter(State &state, const Event &event) -> decltype(state.onEnter(event))
    {
        return state.onEnter(event);
    }
};

template <typename... States>
class StateMachine
{
public:
    StateMachine() = default;

    StateMachine(States... states)
        : states(std::move(states)...)
    {
    }

    template <typename State>
    State &transitionTo()
    {
        State &state = std::get<State>(states);
        currentState = &state;
        return state;
    }

    template <typename Event>
    void handle(const Event &event)
    {
        handleBy(event, *this);
    }

    template <typename Event, typename Machine>
    void handleBy(const Event &event, Machine &machine)
    {
        auto passEventToState = [&machine, &event](auto statePtr)
        {
            auto action = statePtr->handle(event);
            action.execute(machine, *statePtr, event);
        };
        std::visit(passEventToState, currentState);
    }

private:
    std::tuple<States...> states;
    std::variant<States *...> currentState{&std::get<0>(states)};
};

struct OpenEvent
{
};

struct CloseEvent
{
};

struct LockEvent
{
    uint32_t newKey;
};

struct UnlockEvent
{
    uint32_t key;
};

struct ClosedState;
struct OpenState;
class LockedState;

struct ClosedState : public Will<ByDefault<Nothing>,
                                 On<LockEvent, TransitionTo<LockedState>>,
                                 On<OpenEvent, TransitionTo<OpenState>>>
{
};

struct OpenState : public Will<ByDefault<Nothing>,
                               On<CloseEvent, TransitionTo<ClosedState>>>
{
};

class LockedState : public ByDefault<Nothing>
{
public:
    using ByDefault::handle;

    LockedState(uint32_t key)
        : key(key)
    {
    }

    void onEnter(const LockEvent &e)
    {
        key = e.newKey;
    }

    Maybe<TransitionTo<ClosedState>> handle(const UnlockEvent &e)
    {
        if (e.key == key)
        {
            std::cout << "Unlock Succeed!" << std::endl;
            return TransitionTo<ClosedState>{};
        }
        std::cout << "Unlock Failed!" << std::endl;

        return Nothing{};
    }

private:
    uint32_t key;
};

using Door = StateMachine<ClosedState, OpenState, LockedState>;

int main()
{
    Door door{ClosedState{}, OpenState{}, LockedState{0}};

    door.handle(LockEvent{1234});
    door.handle(UnlockEvent{2});
    door.handle(UnlockEvent{1234});
    return 0;
}
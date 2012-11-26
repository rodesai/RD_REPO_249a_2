#ifndef __ACTIVITY_H__
#define __ACTIVITY_H__

#include <string>
#include <functional>
#include <map>
#include <queue>

#include "fwk/Ptr.h"
#include "fwk/NamedInterface.h"

#include "Nominal.h"

using namespace std;

namespace Activity{

class Activity;
typedef Fwk::Ptr<Activity> ActivityPtr;
typedef Fwk::Ptr<Activity const> ActivityPtrConst;

class Manager;
typedef Fwk::Ptr<Manager> ManagerPtr;
typedef Fwk::Ptr<Manager const> ManagerPtrConst;

/* Define the type 'Time' */
class Time : public Ordinal<Time,double> {
public:
    Time(double time) : Ordinal<Time,double>(time)
    {}
    std::string str() {
        std::stringstream s;
        s.precision(2);
        s << std::fixed << value_;
        return s.str();
    }
};

class Activity : public Fwk::NamedInterface {

public:

    /* Notifiee class for Activities */
    class Notifiee : public virtual Fwk::NamedInterface::Notifiee{
    public:
        virtual void onNextTime() {}
        virtual void onStatus() {}
        inline ActivityPtr notifier(){ return notifier_; }
        void notifierIs(ActivityPtr notifier){ notifier_=notifier; }
    protected:
        Notifiee(){}
        virtual ~Notifiee(){}
        ActivityPtr notifier_;
    };
    typedef Fwk::Ptr<Notifiee> NotifieePtr;
    typedef Fwk::Ptr<Notifiee const> NotifieePtrConst;

    /* Current Execution State of Activity */
    enum Status {
        free_, executing_, nextTimeScheduled_, cancelled_
    };
    static Status free(){ return free_; }
    static Status executing(){ return executing_; }
    static Status nextTimeScheduled(){ return nextTimeScheduled_; }
    static Status cancelled(){ return cancelled_; }

    /* Accessors */
    inline Status status() const { return status_; }
    inline Time nextTime() const { return nextTime_; }
    inline NotifieePtr notifiee() { return notifiee_; }

    /* Mutators */
    void statusIs(Status s);    
    void nextTimeIs(Time t);
    void lastNotifieeIs(Notifiee* n);

private:
    Activity(string name, ManagerPtr manager); 
    string name_;
    friend class Manager;
    Status status_;
    Time nextTime_;
    Notifiee* notifiee_;
    ManagerPtr manager_;
};

//Comparison class for activities   
class ActivityComp : public binary_function<ActivityPtr, ActivityPtr, bool> {
public:
    ActivityComp() {}
    bool operator()(ActivityPtr a, ActivityPtr b) const {
        return (a->nextTime() > b->nextTime());
    }
};

class Manager : public Fwk::PtrInterface<Manager> {
public:
    /* Accessors */
    ActivityPtr activity(const string &name) const;
    inline Time now() const { return now_; }
    /* Mutators */
    ActivityPtr activityNew();
    ActivityPtr activityNew(const string &name);
    void activityDel(const string &name);
    void lastActivityIs(ActivityPtr);
    void nowIs(Time);
    static ManagerPtr ManagerIs(){ return new Manager(); }
private:
    Manager() : now_(0), activityName_(0){}
    priority_queue<ActivityPtr, vector<ActivityPtr>, ActivityComp> scheduledActivities_;
    map<string, ActivityPtr> activities_; 
    Time now_;
    uint32_t activityName_;
};

}

#endif

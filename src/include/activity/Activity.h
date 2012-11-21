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
    private:
        ActivityPtr notifier_;
    };
    typedef Fwk::Ptr<Notifiee> NotifieePtr;
    typedef Fwk::Ptr<Notifiee const> NotifieePtrConst;

    /* Current Execution State of Activity */
    enum Status {
        free, waiting, ready, executing, nextTimeScheduled, deleted
    };

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
    ActivityPtr activityNew(const string &name);
    void activityDel(const string &name);
    void lastActivityIs(ActivityPtr);
    void nowIs(Time);
private:
    Manager() : now_(0){}
    priority_queue<ActivityPtr, vector<ActivityPtr>, ActivityComp> scheduledActivities_;
    map<string, ActivityPtr> activities_; 
    Time now_;
};

#endif
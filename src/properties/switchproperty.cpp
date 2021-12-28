 #include "model/switchproperty.h"

void SwitchProperty::setSwitch(QString name,bool value)
{
    QList<SwitchValue*> switches=getSwitches();
    /*!< Only 1 switch of many can be ON (e.g. radio buttons) */
    if (getRule()==0) {
        for (int i = 0; i < switches.size(); ++i) {
            if (name==switches[i]->name()) {
                switches[i]->setState(true);
            } else {
                switches[i]->setState(false);
            }
        }
    }
    /*!< At most one switch can be ON, but all switches can be off. It is similar to ISR_1OFMANY with the exception that all switches can be off. */
    if (getRule()==1) {
        for (int i = 0; i < switches.size(); ++i) {
            if (name==switches[i]->name()) {
                if (switches[i]->switchState()) switches[i]->setState(false); else switches[i]->setState(true);
            } else {
                switches[i]->setState(false);
            }
        }
    }
    /*!< Any number of switches can be ON (e.g. check boxes) */
    if (getRule()==2) {
        for (int i = 0; i < switches.size(); ++i) {
            if (name==switches[i]->name()) {
                if (switches[i]->switchState()) switches[i]->setState(false); else switches[i]->setState(true);
            }
        }
    }

    setSwitches(switches);

}


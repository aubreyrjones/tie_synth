#include "Control.hpp"

namespace audio {

_ControlBase* firstControl = nullptr;
_ControlBase* lastRegisteredControl = nullptr;



_ControlBase* get_first_control() {
    return firstControl;
}


void run_all_control_updates() {
    auto control = audio::get_first_control();
    do {
        control->doUpdate();
    } while ( (control = control->nextControl()) );
}


void _ControlBase::register_new_control(_ControlBase* control) {
    if (!firstControl) {
        firstControl = lastRegisteredControl = control;
        return;
    }

    lastRegisteredControl->next = control;
    lastRegisteredControl = control;
}

}
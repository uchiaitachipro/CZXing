//
// Created by uchia on 2019-12-08.
//

#ifndef CZXING_HOOKBASE_H
#define CZXING_HOOKBASE_H

class HookBase {
public:
    typedef void (*HookFunction)(int, long, long,long);

    void setHookFunction(HookFunction f) {
        const_cast<const HookBase *>(this)->setHookFunction(f);
    }

    void setHookFunction(HookFunction f) const {
        this->_hook = f;
    }

protected:
    mutable HookFunction _hook = 0L;

    void notifyHook(int phrase, long p1, long p2,long p3 = 0) const {
        if (_hook == 0) {
            return;
        }
        _hook(phrase, p1, p2,p3);
    }

    void notifyHook(int phrase, long p1, long p2,long p3 = 0) {
        const_cast<const HookBase *>(this)->notifyHook(phrase, p1, p2,p3);
    }
};

#endif //CZXING_HOOKBASE_H

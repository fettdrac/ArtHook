//
// Created by root on 19-6-6.
//

#ifndef ARTHOOK_JITCOMPLIER_H
#define ARTHOOK_JITCOMPLIER_H


class JitCompiler {
public:
    static JitCompiler* GetInstance(){
        static JitCompiler instance=JitCompiler();
        return &instance;
    }

private:
    JitCompiler();
};


#endif //ARTHOOK_JITCOMPLIER_H

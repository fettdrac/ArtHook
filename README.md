# ArtHook
an android art hook framework
![ArtHook](mahua-logo.jpg)
## ArtHook是什么?(What hell is the ArtHook)
双关名字，art虚拟机的hook，也是art风格的hook（膨胀了，美术不及格的开始谈论艺术了）

一个算是`整合项目`的东西，自己原创的原理部分其实不多，只是稍微理解了几个hook框架，做个综合而已

ArtHook is a kind of combined thing with little my own work.

It is based on my shallow understanding of some hook frameworks and tourials and make their benefits toghther.

算是“重复”造轮子吧。离不开dalao们的实现思路（尤其是
[`whale`](https://github.com/asLody/whale)
那套把所有跳转都桥接到jni_entrypoint的操作可是真的秀）

(Especially,Whale's fancy operation of getting all entrypoint jump into jni_entrypoint really shocks me.)

## ArtHook可以拿来做什么(What can be done by using ArtHook)？

* `Android ART Hook`
    *  JNI-style Hook，支持调用原方法，实现思路参照的是whale的（不如说直接就是去掉BuildJNICloseure的重复轮子）
    *  如果dalao愿意加个libffi或者DexMaker做AOP的话就更好了
    *  全局注入（配合
    [静态注入](https://bbs.pediy.com/thread-224297.htm)
    技术，实现是PLT hook JNI_CreateJavaVM，这个时机够早了）
* `Native PLT Hook`
    * 直接采用了iqiyi开源的[xHook](https://github.com/iqiyi/xHook)
    * 后续考虑增加inline hook（学考后再说吧，ARM指令一看就不是边复习语数就可以学的）
    
    其实这里有一个思路，>=7.0用jit_compiler手动编译目标ArtMethod，这样入口就确定了
    直接inline hook这个quick_code入口就可以实现稳定hook，到了8.0以上入口替换就稳多了

* `JNI Dex Load`
    * 这个目前还没有完全实现，memory_dex会段错误，学考完填坑）

## ArtHook的优势在哪(What are the benefit of ArtHook)

* `和java层交互少，侵入性小`
    (Decrease the interaction with java world,which ensures less risks than load another dex).
* `采用art自有结构java/lang/Object内部方法确定偏移`
    (Use java/lang/Object's internalClone and clone to get the offsets of ArtMethod)

## 怎么用(How to use ArtHook)
其实写法有点类似于[AndHook](https://github.com/asLody/AndHook)，在项目的art/hook_module文件夹下添加如下类似代码,
即可实现简单的art hook操作
```cpp
#include "app_on_create.h"
#include "../ArtMethod.h"
#include "../../base/log.h"
#include "../../base/jni_helper.h"

static ArtMethod method= nullptr;
void my_CallApplicationOnCreate(JNIEnv *env,jobject thiz,jobject thisApp){
    LOGI("invoke hooked callApplicationOnCreate\n");
    jclass app_class=env->GetObjectClass(thisApp);
    jobjectArray params=env->NewObjectArray(1,app_class,thisApp);
    method.InvokeOriginal(env,thiz, params);
}

void hook_app_on_create() {//在合适的时候（比如hooked JNI_CreateJavaVM回调中调用这个方法）
    JNIEnv *env=GetJniEnv();
    jclass in_class=env->FindClass("android/app/Instrumentation");
    jmethodID jni_method=env->GetMethodID(in_class,"callApplicationOnCreate","(Landroid/app/Application;)V");
    method= ArtMethod(jni_method);
    method.Hook(reinterpret_cast<ptr_t>(my_CallApplicationOnCreate));
    LOGI("hook callApplicationOnCreate finished\n");
}
```

## 问题反馈
这套框架目前在Android 6.0 AOSP下工作良好。

It can work well as least now on Android 6.0(M_10) AOSP,but I cannot test it on more advanced editon.
So,if you get any problems, please tell me by `issue`.Thank you!!!

有些借鉴asLody大佬和杂七杂八看教程来适配更高的版本（7,8,9），条件有限，没能测试。

* 参考教程(Tourials refered)

    * 1、[源码简析之ArtMethod结构与涉及技术介绍](https://bbs.pediy.com/thread-248898.htm)
    * 2、[EdXposed & ART Hook 细节分享](https://bbs.pediy.com/thread-250601.htm)
    * 3、[一种简单的Android全局注入方案](https://bbs.pediy.com/thread-224191.htm)
    （这个过不了selinux，需要在init.rc里开机写setenforce 0，不过运行时注入实测可以，就是libinject不是很稳定）


* 有任何`兼容性问题`，就算是偶发的，也欢迎`[提issue]`，尤其是更高版下的情况，稳定复现的，`求一个复现APK`
    (Issues are especially welcomed,with a simple apk that can cause the problem would be even better!)


## 致谢和声明

* 1、看雪上SandHook作者的那篇文章，分析的很透彻，坑点细节也比较多。
* 2、借鉴了asLody大佬的whale框架的实现，这个项目更好点说是它的“优化”而不是一个独立的项目
    （不会pull request，kali下开ssh进来就是root风险有点大，
    代码也写得不是很优雅，注释只有中文，实在不好意思pull request）。
    如果dalao认为这套获取偏移的方法比手动注册两个reversed stub更好的话，`欢迎merge或者直接用`。
* 3、一个画风清奇的致谢，这篇MarkDown版本的README是[MaHua编辑器](http://mahua.jser.me/)写的，
    在线的MarkDown，难得

* 4、声明：这个项目的代码可以随便拿去用，但要注明一下出处。（应该没人会用吧
    (You can copy any code from the project but please write where it comes from in description. Thank you!!!)

## 关于作者

```javascript
  var ihubo = {
    nickName  : "fettdrac",
    self_introduction: "一枚准高三学生，平时不务正业写写C#、Java、JNI环境的cpp",
    site : "莫得网站，github挺好"
  }
```

## 编译方法
* 1、把整个cpp文件夹替换掉新Android Studio 包含JNI项目的cpp文件夹，在build.gradle里做如下配置

```java
externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
            version "3.10.2"//我的cmake版本是这个，没有加额外配置
        }
    }
```

* 2、或者自己写Android.mk，有cmake+NDK环境的cmake CMakeLists.txt+make完事
    （这年头做android开发不用android studio或者原生Intelj IDEA的挺少了吧，VS的android jni开发套件，55开吧

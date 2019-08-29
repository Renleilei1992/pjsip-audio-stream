# pjsip编译说明

1. [下载源码](https://www.pjsip.org/download.htm)
2. 编译
    将config_site.h复制到pjlib\include\pj中
    - win
        vs2017打开pjproject-vs14.sln，升级项目，项目属性修改win sdk版本和平台工具集为本机版本，编译pjproject即可

        在lib目录可以找到对应的lib文件
    - mac
        进入项目根目录，执行如下脚本
        ```
        ./configure --prefix=/Users/bytedance/Downloads/pjout
        make dep
        make
        make install
        ```
        在/Users/bytedance/Downloads/pjout可以找到对应的.a文件

exportlib：

  这是一个Linux 系统使用的共享库依赖导出工具，它能把可执行文件或者共享库的依赖库导出到当前目录下，方便程序的移植搬运。

  这是个非常简单的工具，使用到了ldd、ld-linux.so.2、或者C库中的函数，任选其一都可以，它能将程序的依赖库导出到当前目录中，
  
  注意，目前没有实现动态加载的共享库的导出，还有库于库之间的依赖，想要导出共享库依赖需要单独导出库依赖。

使用方法：
  Usage :
           Export Library List!
                ./exportlib [-c] [-x(0|1|2)] [-d] -p program             : 导出程序的依赖库到当前目录。

                ./exportlib -c : SymLink Use Copy Command![Optional]      ：使用符号链接。
                ./exportlib -x[0|1|2] : Export Use : {0:ldd|1: Built-in | 2: /lib/ld-linux.so.2 } [default : ldd] [Optional]    ： 导出的方式 ldd 或者 内置 或者 ld-linux.so.2。
                ./exportlib -d : Export Development Library![Optional]     ： 导出开发库，用于编译程序。
                ./exportlib -p Program : Export Runtime Library![Required]  ：导出运行库，用于执行程序。

!(https://github.com/ZhaoLiangJun/exportlib/blob/main/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202025-06-03%20123512.png))

# C++助手 | C-plus-plus Assistant

## 简介 | Introduction

一款用于C++应用程序开发的轻量级套件（SDK），包含一个库和若干个框架。
该套件如无必要，不重复造轮子，不盲目追求大而全以及跨平台。
以满足实际需求为主，只做别人没有的（或者暂未发现别人有的），
或者轮子已有，但体量太大太重，依赖关系太多太杂。
便利性第一，性能次之（但并非不重视）。

> A light-weight SDK for C++ application development, which includes a library and some frameworks. 
Unless necessary, this toolkit would not repeat others on some functionalities, 
neither does it aim at rich functionalities and multi-platform support. 
Instead, it provides some useful APIs that other SDKs do not provide, 
or that are provided by a heavy-weight SDK with large amounts of dependencies which is complex to handle. 
Convenience first, performance after it (not meaning that it's not important).

***码少才能生活好！***

> ***Less code, better world!***

***注***：该套件**在某些场景（例如嵌入式）仍略显笨重**，已停止更新，
请关注新项目“[懒编程秘笈](https://github.com/FooFooDamon/lazy_coding_skills.git)”。

> ***NOTE***：This SDK is **still a bit heavy for some scenarios (e.g., embedded environment)**
and thus no longer updated, please focus on a new one
"[Lazy Coding Skills](https://github.com/FooFooDamon/lazy_coding_skills.git)".

## 组成 | Compositions

### 基础库：libcpp_assistant

一个小型库，是套件的基础和核心，可独立使用，套件的其它部分也依赖它。
功能包括但不限于：字符串处理、命令行参数解析、浮点数转换、日志操作、
信号捕捉及处理、日期时间处理、TCP通信、XML配置文件操作等。

> A light-weight library, the base and the core of this SDK,
which can be used independently and is needed by other parts of the SDK.
Its functionalities include but are not limited to: string handling,
command line parameters parsing, floating-point number conversion,
logging operations, signal capturing and handling, date time handling,
TCP communication， XML configuration file operations, etc..

### 应用程序框架：frameworks

#### 用于简单小程序的框架：simple_app

适用于那些小体积、功能简单的程序，通常不需要配置文件，且用完即弃，一般作为测试程序。

> Generally used for building those small-size, simeple programs, 
which usually don't need a configuration file and act as one-shot testing programs.

#### 用于TCP程序的框架：tcp_app

适用于较复杂的、以TCP作为通信手段的应用程序，这类程序往往比较复杂，需要配置文件，
需要自定义通信协议（可利用`Protocol Buffer`或`JSON`），
该框架可帮助开发人员快速构建起一个实用的TCP应用服务器或客户端，
但目前性能不是太高，且只能用单线程，后续若有需要会优化性能并增加多线程支持。

> This framework can be used for building those TCP-based programs which are more complex 
but more practical at the same time. Programs of this kind usually need a configuration
file and communication protocols (using `Protocol` or `JSON`).
This framework could help developers build a practical TCP server or client quickly and easily.
The disadvantage of it is that the performance is not so good by far,
and it is fit for single-threading only. Anyway, if necessary, performance improvement
and multi-threading support will be made and added in future.

## 安装及卸载 | Installation and Uninstallation

### 适用平台 | Supported Platforms

* `Linux`: Better be `Ubuntu` (>= 16.04)

* `Windows`: May or may not be supported in future.

### 适用的语言环境 | Language environments for running

* 简体中文

* English

### 依赖关系 | Dependencies

* `STL` (C++ Standard Template Library)

* `g++`: >= `5.4.0`

* `make`

* `Google Protocol Buffer`

* `jsoncpp`

### 安装步骤 | Installation Steps

1. 将该项目克隆到`$HOME/src`目录（推荐）或其它目录：

> Clone this project to `$HOME/src` (recommended) or some other directory:

```
$ export CA_PARENT_DIR=$HOME/src

# or:

$ export CA_PARENT_DIR=/path/to/ca_parent

$ mkdir -p $CA_PARENT_DIR && cd $CA_PARENT_DIR

$ git clone https://github.com/FooFooDamon/cpp_assistant.git
```

2. 编译并安装基础库`libcpp_assistant`：

> Compile and install the fundamental library `libcpp_assistant`:

```
$ cd $CA_PARENT_DIR/cpp_assistant/code/libcpp_assistant/

$ CA_MAKE_FLAGS=""

# 若要自定义库的命名空间，修改并执行以下命令：
# If you need to customize namespace of the library, modify and run the command below:
# CA_MAKE_FLAGS="$CA_MAKE_FLAGS CA_LIB_NAMESPACE=foo"

# 若系统已安装了TinyXML，则执行以下命令：
# If TinyXML already exists in your system, run the command below:
# CA_MAKE_FLAGS="$CA_MAKE_FLAGS NO_TINYXML=1"

# 若预装的TinyXML头文件不在系统头文件目录，则需要显式指定：
# If the pre-installed TinyXML headers are not in a system header directory,
# you should specify the directory explicitly:
# CA_MAKE_FLAGS="$CA_MAKE_FLAGS TINYXML_INC_DIR=/path/to/tinyxml/headers/directory"

$ make $CA_MAKE_FLAGS

$ make install

$ make clean
```

3. 将`CPP_ASSISTANT_ROOT`环境变量写入`Bash启动配置文件`。
如果套件放在`$HOME/src`目录，这一步可省略，否则必须执行，不然框架用不了。
操作如下：

> Set the `CPP_ASSISTANT_ROOT` environment variable into the startup configuration file
of Bash. But if the SDK is in `$HOME/src` directory, it's okay to skip this step,
otherwise this step must be executed and if not, frameworks of this SDK would not work.
Settings are:

```
$ BASH_RC=$HOME/.bashrc # or .bash_profile, depending on the Linux release type.

$ echo "export CPP_ASSISTANT_ROOT=$CA_PARENT_DIR/cpp_assistant" >> $BASH_RC
```

4. 安装完毕也不要删除SDK目录，因为每次使用框架都需要源码的即时编译。

> DO NOT delete the SDK directory, because source code of frameworks are needed for applications
for instant compiling.

### 卸载步骤 | Uninstallation Steps

1. 卸载`libcpp_assistant`：

> Uninstall `libcpp_assistant`:

```
$ cd $CA_PARENT_DIR/cpp_assistant/code/libcpp_assistant/

$ make uninstall
```

2. 删除整个SDK目录：

> Delete the whole SDK directory:

```
$ rm -rf $CPP_ASSISTANT_ROOT
```

3. 清除`CPP_ASSISTANT_ROOT`环境变量：

> Clear the `CPP_ASSISTANT_ROOT` environment variable:

```
$ sed -i "/CPP_ASSISTANT_ROOT/d" $BASH_RC
```

## 用法 | Usage

### libcpp_assistant

在编译你的应用程序时，加上编译选项`-I${HOME}/include`，
以及链接选项`-L${HOME}/lib -lcpp_assistant`，即可使用该库的接口。

> Add the compile option `-I${HOME}/include` and link options `-L${HOME}/lib -lcpp_assistant`
into your project Makefile when compiling your project, then you could use APIs of this library.

### frameworks

1. 利用C++助手框架创建项目：

> Use a framework of this SDK to create a project:

```
$ find $CPP_ASSISTANT_ROOT/ -name "*.sh" | xargs chmod 777

$ $CPP_ASSISTANT_ROOT/scripts/generate_project_from_cpp_assistant.sh
```

2. 根据需要修改项目Makefile的编译参数。参数列举如下，
你可选择启用或屏蔽它们中的一个或多个：

> Modify compile parameters in project Makefiles depending on your needs.
Parameters are listed below, you can enable or disable one or more of them:

```
USE_PURE_FILE_NAME

-DENABLES_CHINESE

-DHAS_CONFIG_FILES

-DHAS_PROTOBUF

-DUSE_JSON_MSG

-DHAS_TCP

-DACCEPTS_CLIENTS

-DHAS_UPSTREAM_SERVERS

-DVALIDATES_CONNECTION

IS_DISPATCHER
```

3. 增加你自己的编译参数（若有）。

> Add your own compile parameters (if any).

4. 在项目根目录下执行`make`，即可编译。

> Execute `make` command in project root directory to compile it.

## 文档说明 | Documentation

1. 简单的、指引性的文档，见各个`doc`或`docs`目录下的文档。这类文档以中文为主。
特别地，`$CPP_ASSISTANT_ROOT/conf`目录下的HTML文档是对于程序配置文件的说明。

> Simple and guiding documents are in directories named `doc` or `docs`.
These documents are mostly written in Chinese. 
Particularly, HTML documents in `$CPP_ASSISTANT_ROOT/conf` are comments
for application program configuration files.

2. 详细的文档即代码及其注释，用英文写的，为的是确保任何环境下都能阅读，
防止中文乱码现象。

> The detailed documentation is the code itself and its comments written
in English, you can read them using any editor and don't have to worry about
the messy code.

## 联系方式及反馈 | Contact and Feedback

Author: Wen Xiongchang (Man Hung-Coeng)

Email: <udc577@126.com>

GitHub: https://github.com/FooFooDamon/

任何缺陷、建议，欢迎给我发邮件，或在GitHub上创建**问题单**。

> Any bugs and recommendations, please send me emails, or create **issues** on GitHub.

## 许可证 | License

详见各代码文件。

> See details in each code files.


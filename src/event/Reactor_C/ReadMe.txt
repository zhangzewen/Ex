这是一个纯粹从livevent-1.4.X版本上一个模子刻出来的，就是想自己看看libevent的工作流程以及Reactor模式
当然，这个复制品剔除了timeout和signal，单纯的I/O，现在还是跑的一塌糊涂
这个只是个人练习使用。
你要是不怕抓狂的话，可以试试
当然，我会不停的去完善它

This just a copy of libevent-1.4.x, I delete timeout and signal from the source code for exercise
By now， it dose not work well！


<1>到2013年8月22日，她可以正常的跑起来了 
siege -c 10 -t 10 http://192.168.10.65:8080 跑起来很欢快，我太激动鸟。
之前的错误就是出现在list_for_each_entry
导致event_process_active在循环判断时失误，采用list_for_each_entry_safe就搞定了，但是这用了我几乎一天的事件
又是gdb，又是strace。很高兴在早上时候想到了原因所在。具体的原因就是两个循环的实现的区别了。
<2>2013年9年25日，现在添加了evbuf了，可以在水平触发下接收完整的HTTP Request，但是还有好多细节没有处理，如：处理http请求的后续
状态，请求处理完成后资源的释放工作，这些都没有做完！
<3>2013年11月8号，添加了定时器，现在几乎是可以正常工作了，但是还是有些小问题，这一个半月的时间断断续续的弄，终于快完整了,具体可以看../Reactor_Timeout_C的实现

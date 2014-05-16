
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>

//函数：基于原子操作的自旋锁方法
//lock：原子变量表达的锁
//value：标志位，锁是否被某一进程占用
//spin：在多处理器系统内，当ngx_spinlock方法没有拿到锁时，当前进程在内核的一次调度中该方法等待其他处理器释放锁的时间
void
ngx_spinlock(ngx_atomic_t *lock, ngx_atomic_int_t value, ngx_uint_t spin)
{

#if (NGX_HAVE_ATOMIC_OPS)

    ngx_uint_t  i, n;
//一直处于循环中，直到获取到锁
    for ( ;; ) {
//lock为0表示没有其他进程持有锁，这时将lock值设置为value参数表示当前进程持有了锁
        if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, value)) {
            return;
        }
//如果时多处理器系统
        if (ngx_ncpu > 1) {

            for (n = 1; n < spin; n <<= 1) {
//随着等待的次数越来越多，实际去检查锁的间隔时间越来越大
                for (i = 0; i < n; i++) {
                    ngx_cpu_pause();//告诉cpu现在处于自旋锁等待状态
                }
//检测锁是否被释放
                if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, value)) {
                    return;
                }
            }
        }
//当前进程让出处理器，但仍然处于可执行状态
        ngx_sched_yield();
    }

#else

#if (NGX_THREADS)

#error ngx_spinlock() or ngx_atomic_cmp_set() are not defined !

#endif

#endif

}

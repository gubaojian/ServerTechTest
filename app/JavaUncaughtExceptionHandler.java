package com.rushi.lib_common.safe;

import androidx.annotation.NonNull;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import com.blankj.utilcode.util.ThrowableUtils;
import com.rushi.lib_common.BuildConfig;

import java.io.StringWriter;

/**
 * Java有些异常kotlin的try{}catch(e:Throwable)捕获不了，用Java捕获更靠谱
 * */
public class JavaUncaughtExceptionHandler implements  Thread.UncaughtExceptionHandler {
    private static boolean isReporting;
    private Handler mainHandler =  new Handler(Looper.getMainLooper());
    private Thread.UncaughtExceptionHandler chain;

    public JavaUncaughtExceptionHandler(Thread.UncaughtExceptionHandler chain) {
        this.chain = chain;
    }

    @Override
    public void uncaughtException(@NonNull Thread thread, @NonNull Throwable throwable) {
        if (isReporting){
            Log.e("CommonApplication", "DefaultExceptionHandler is Reporting");
            return;
        }
        if (BuildConfig.DEBUG) {
            Log.e("CommonApplication", "DefaultExceptionHandler Detail: \n" + ThrowableUtils.getFullStackTrace(throwable));
        }
        try {
            isReporting = true;
            Log.e("CommonApplication", "DefaultExceptionHandler Pass Next Chain" + chain);
            if (chain != null) {
                //catch finally ma not execute
                mainHandler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        isReporting = false;
                    }
                }, 500);
                if (thread != Looper.getMainLooper().getThread()) {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                //may crash when reporting, which cause try catch not execute.
                                chain.uncaughtException(thread, throwable);
                                isReporting = false;
                             } catch (Throwable e) { //catch finally ma not execute
                                Log.e("CommonApplication", "ReportError: \n" + ThrowableUtils.getFullStackTrace(throwable));
                             } finally {
                                isReporting = false;
                            }
                        }
                    });
                } else {
                    try {
                        chain.uncaughtException(thread, throwable);
                    } catch (Throwable e) {
                        Log.e("CommonApplication", "ReportError Main: \n" + ThrowableUtils.getFullStackTrace(throwable));
                     } finally {
                        isReporting = false;
                    }
                }
            }
        } catch (java.lang.Throwable e) {
            Log.e("CommonApplication", "ExceptionHandler Inner: \n" + ThrowableUtils.getFullStackTrace(throwable));
        }
    }
}

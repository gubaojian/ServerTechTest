package org.efurture.test.inner;

import com.alibaba.fastjson2.JSON;
import com.alibaba.fastjson2.JSONObject;
import com.esotericsoftware.reflectasm.MethodAccess;
import org.efurture.test.Api;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class ApiMethodCallDispatcher {
    private final String name;
    private final Api api;
    private final Map<String, MethodCall> methodCallMap = new ConcurrentHashMap<>();

    public ApiMethodCallDispatcher(Api api, String name) {
        this.api = api;
        this.name = name;
        MethodAccess method = MethodAccess.get(api.getClass());
        String[] methodNames = method.getMethodNames();
        Class<?>[][] parameterTypes = method.getParameterTypes();
        Class<?>[] returnTypes = method.getReturnTypes();
        for(String methodName : methodNames) {
            MethodCall methodCall = new MethodCall();
            methodCall.method = method;
            methodCall.index = method.getIndex(methodName);
            methodCall.parameterType = parameterTypes[method.getIndex(methodName)];
            methodCall.returnType = returnTypes[method.getIndex(methodName)];
            if (methodCall.parameterType.length > 1) {
                throw new IllegalArgumentException("api " + name + " method " + methodName + " at most have one object params");
            }
            if (methodCallMap.containsKey(methodName)) {
                throw new IllegalArgumentException("api " + name + " method " + methodName + " should not contains two same method");
            }
            System.out.println("fount method " + methodName);
            methodCallMap.put(methodName, methodCall);
        }
    }

    public Object callMethod(String method, String json) throws NoSuchMethodException {
        MethodCall call = methodCallMap.get(method);
        if (call == null) {
            throw new NoSuchMethodException("api " + name + " method " + method + " not found");
        }
        if (call.parameterType.length == 0) {
            return call.method.invoke(api, call.index);
        }
        Object params = JSON.parseObject(json, call.parameterType[0]);
        return call.method.invoke(api, call.index, params);
    }

    public Object callMethod(String method, JSONObject json) throws NoSuchMethodException {
        MethodCall call = methodCallMap.get(method);
        if (call == null) {
            throw new NoSuchMethodException("api " + name + " method " + method + " not found");
        }
        if (call.parameterType.length == 0) {
            return call.method.invoke(api, call.index);
        }
        if (json == null) {
            throw new IllegalArgumentException("api " + name + " method " + method + " params should not be null");
        }
        Object params = json.to(call.parameterType[0]);
        return call.method.invoke(api, call.index, params);
    }

    public boolean needResponse(String method) throws NoSuchMethodException {
        MethodCall call = methodCallMap.get(method);
        if (call == null) {
            throw new NoSuchMethodException("api " + name + " method " + method + " not found");
        }
        return call.returnType == void.class || call.returnType == Void.class;
    }

    static class MethodCall {
        MethodAccess method;
        int index;
        Class<?>[] parameterType;
        Class<?> returnType;
    }
}

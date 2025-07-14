package org.efurture.test;

import com.alibaba.fastjson2.JSON;
import com.alibaba.fastjson2.JSONObject;
import org.efurture.test.inner.ApiMethodCallDispatcher;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class ApiCall {
    public Map<String, ApiMethodCallDispatcher> registerApiModule = new ConcurrentHashMap<>();

    public void registerApi(String name, Api api) {
        registerApiModule.put(name, new ApiMethodCallDispatcher(api, name));
    }

    //json protocol api call
    Object callApi(String apiCall) {
        try {
            //参考json rpc https://www.jsonrpc.org/specification
            JSONObject map = JSON.parseObject(apiCall);
            String api = map.getString("a");
            String method = map.getString("m");
            JSONObject params = map.getJSONObject("p");
            String requestId = map.getString("id");
            ApiMethodCallDispatcher apiMethodCallDispatcher = registerApiModule.get(api);
            if (apiMethodCallDispatcher == null) {
                throw new NoSuchMethodException("api " + api + " module not found");
            }
            try {
                Object result = apiMethodCallDispatcher.callMethod(method, params);
                boolean noReplay = apiMethodCallDispatcher.needResponse(method);
                if (noReplay) { // 不需要相应，只处理即可, 适合多协同任务
                    return null;
                }
                JSONObject response = new JSONObject();
                response.put("id", requestId);
                if (result != null) {
                    response.put("r", result); //result
                }
                return  response;
            } catch (Exception e) {
                JSONObject response = new JSONObject();
                response.put("id", requestId);
                response.put("e", e.getMessage());
                return  response;
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

}

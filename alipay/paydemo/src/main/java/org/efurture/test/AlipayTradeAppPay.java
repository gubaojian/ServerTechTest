package org.efurture.test;

import com.alipay.api.*;
import com.alipay.api.internal.util.AlipaySignature;
import com.alipay.api.internal.util.LoadTestUtil;
import com.alipay.api.internal.util.WebUtils;
import com.alipay.api.request.AlipayTradeAppPayRequest;
import com.alipay.api.domain.ExtUserInfo;
import com.alipay.api.domain.AlipayTradeAppPayModel;
import com.alipay.api.response.AlipayTradeAppPayResponse;
import com.alipay.api.domain.ExtendParams;
import com.alipay.api.domain.GoodsDetail;

import java.util.*;

public class AlipayTradeAppPay {

    static final  String privateKey  = "";
    static final  String appPublicKey  =  "";
    static final  String alipayPublicKey = "";

    
    public static void main(String[] args) throws AlipayApiException {
        // 初始化SDK
        DefaultAlipayClient alipayClient = new DefaultAlipayClient(getAlipayConfig());

        // 构造请求参数以调用接口
        AlipayTradeAppPayRequest request = new AlipayTradeAppPayRequest();
        AlipayTradeAppPayModel model = new AlipayTradeAppPayModel();

        // 设置商户订单号
        model.setOutTradeNo("5235355");

        // 设置订单总金额
        model.setTotalAmount("9.00");

        // 设置订单标题
        model.setSubject("大乐透");

        // 设置产品码
        //model.setProductCode("QUICK_MSECURITY_PAY");

        // 设置订单包含的商品列表信息
        List<GoodsDetail> goodsDetail = new ArrayList<GoodsDetail>();
        GoodsDetail goodsDetail0 = new GoodsDetail();
        goodsDetail0.setGoodsName("ipad");
        goodsDetail0.setAlipayGoodsId("20010001");
        goodsDetail0.setQuantity(1L);
        goodsDetail0.setPrice("2000");
        goodsDetail0.setGoodsId("apple-01");
        goodsDetail0.setGoodsCategory("34543238");
        goodsDetail0.setCategoriesTree("124868003|126232002|126252004");
        goodsDetail0.setShowUrl("http://www.alipay.com/xxx.jpg");
        goodsDetail.add(goodsDetail0);
        //model.setGoodsDetail(goodsDetail);

        // 设置订单绝对超时时间
        //model.setTimeExpire("2025-08-07 10:05:00");

        // 设置业务扩展参数
        ExtendParams extendParams = new ExtendParams();
        extendParams.setSysServiceProviderId("2088511833207846");
        extendParams.setHbFqSellerPercent("100");
        extendParams.setHbFqNum("3");
        extendParams.setIndustryRefluxInfo("{\"scene_code\":\"metro_tradeorder\",\"channel\":\"xxxx\",\"scene_data\":{\"asset_name\":\"ALIPAY\"}}");
        extendParams.setRoyaltyFreeze("true");
        extendParams.setCardType("S0JP0000");
        ///model.setExtendParams(extendParams);

        // 设置公用回传参数
        //model.setPassbackParams("merchantBizType%3d3C%26merchantBizNo%3d2016010101111");

        // 设置商户的原始订单号
        //model.setMerchantOrderNo("20161008001");


        request.setBizModel(model);
        // 第三方代调用模式下请设置app_auth_token
        // request.putOtherTextParam("app_auth_token", "<-- 请填写应用授权令牌 -->");

        AlipayTradeAppPayResponse response = alipayClient.sdkExecute(request);
        String orderStr = response.getBody();
        System.out.println(orderStr);



        Map<String, String>  params = WebUtils.splitUrlQuery(orderStr);
        Set<String> keys = params.keySet();
        for(String key : keys) {
            params.put(key, WebUtils.decode(params.get(key)));
        }
        Map<String, String> sortedParams = new TreeMap();
        sortedParams.putAll(params);
        String sign = sortedParams.remove("sign");

        String signContent = AlipaySignature.getSignCheckContentV2(sortedParams);
        System.out.println(signContent);
        System.out.println(sign);
        // alipay_sdk=alipay-sdk-java-4.40.354.ALL&app_id=2021005175610319&biz_content={"out_trade_no":"0c8aacc9-c8cd-4276-ad67-62a864238f12","subject":"大乐透","total_amount":"9.00"}&charset=UTF-8&format=json&method=alipay.trade.app.pay&sign_type=RSA2&timestamp=2025-08-06 16:15:21&version=1.0

        DefaultSignChecker signChecker = new DefaultSignChecker(appPublicKey);
        boolean result = signChecker.check(signContent, sign, "RSA2", "UTF-8");
        System.out.println("sign check match " + result);


        if (response.isSuccess()) {
            System.out.println("调用成功");
        } else {
            System.out.println("调用失败");
            // sdk版本是"4.38.0.ALL"及以上,可以参考下面的示例获取诊断链接
            // String diagnosisUrl = DiagnosisUtils.getDiagnosisUrl(response);
            // System.out.println(diagnosisUrl);
        }
    }

    private static AlipayConfig getAlipayConfig() {
        AlipayConfig alipayConfig = new AlipayConfig();
        alipayConfig.setServerUrl("https://openapi.alipay.com/gateway.do");
        alipayConfig.setAppId("2021005175610319");
        alipayConfig.setPrivateKey(privateKey);
        alipayConfig.setFormat("json");
        alipayConfig.setAlipayPublicKey(alipayPublicKey);
        alipayConfig.setCharset("UTF-8");
        alipayConfig.setSignType("RSA2");
        return alipayConfig;
    }
}
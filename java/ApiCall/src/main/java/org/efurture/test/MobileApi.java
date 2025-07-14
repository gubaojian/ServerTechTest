package org.efurture.test;

import java.util.Map;

public interface MobileApi extends Api {


    public String hello();

    public String helloV2(String love);

    public String helloV3(Map<String,Object> request);

    public void hello4();
}

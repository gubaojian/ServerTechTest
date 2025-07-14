package org.efurture.test;

//TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
public class Main {
    public static void main(String[] args) throws NoSuchMethodException {
        //TIP Press <shortcut actionId="ShowIntentionActions"/> with your caret at the highlighted text
        // to see how IntelliJ IDEA suggests fixing it.
        ApiCall apiCall = new ApiCall();
        apiCall.registerApi("mapi", new MobileApiImpl());

        apiCall.callApi("{\"a\":\"mapi\",\"m\":\"hello\",\"p\":{\"dd\":\"ddd\"},\"id\":100}");
        apiCall.callApi("{\"a\":\"mapi\",\"m\":\"helloV2\",\"p\":{\"dd\":\"ddd\"},\"id\":100}");

        long start = System.currentTimeMillis();
        for(int i=0; i<1000*1000; i++) {
            apiCall.callApi("{\"a\":\"mapi\",\"m\":\"helloV3\",\"p\":{\"dd\":\"ddd\"},\"id\":100}");
        }
        long end = System.currentTimeMillis();
        System.out.println("api call used " + (end - start) + " ms");

        Object response = apiCall.callApi("{\"a\":\"mapi\",\"m\":\"helloV2\",\"p\":{\"dd\":\"ddd\"},\"id\":100}");
        System.out.println(response  + " v2 response");

        Object responseV4 = apiCall.callApi("{\"a\":\"mapi\",\"m\":\"hello4\",\"p\":{\"dd\":\"ddd\"},\"id\":100}");
        System.out.println(responseV4  + " v4 need no reply");


    }
}